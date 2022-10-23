/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.cxx
 * @author lachbr
 * @date 2021-02-22
 */

#include "character.h"
#include "bamReader.h"
#include "config_anim.h"
#include "bamWriter.h"
#include "clockObject.h"
#include "loader.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "characterJointEffect.h"
#include "randomizer.h"
#include "animChannelTable.h"
#include "animEvalContext.h"
#include "mathutil_simd.h"

TypeHandle Character::_type_handle;

static PStatCollector apply_pose_collector("*:Animation:Joints:ApplyPose");
static PStatCollector ap_compose_collector("*:Animation:Joints:ApplyPose:Compose");
static PStatCollector ap_net_collector("*:Animation:Joints:ApplyPose:NetTransform");
static PStatCollector ap_skinning_collector("*:Animation:Joints:ApplyPose:SkinningMatrix");
static PStatCollector ap_mark_jvt_collector("*:Animation:Joints:ApplyPose:MarkModified");
static PStatCollector ap_update_net_transform_nodes("*:Animation:Joints:ApplyPose:UpdateNetTransformNodes");

/**
 *
 */
Character::
Character(const Character &copy) :
  Namable(copy)
{
  _update_delay = 0.0;
  _active_owner = nullptr;
  _built_bind_pose = false;

  CDWriter cdata(_cycler, true);
  CDReader cdata_from(copy._cycler);
  cdata->_frame_blend_flag = cdata_from->_frame_blend_flag;
  cdata->_root_xform = cdata_from->_root_xform;

  ensure_layer_count(1);
}

/**
 *
 */
Character::
Character(const std::string &name) :
  Namable(name),
  _update_delay(0.0),
  _active_owner(nullptr),
  _built_bind_pose(false)
{
  ensure_layer_count(1);
}

/**
 * Transforms all the joints of the character by the indicated transform
 * matrix.
 */
void Character::
xform(const LMatrix4 &mat) {
  {
    CDWriter cdata(_cycler);
    cdata->_root_xform = cdata->_root_xform * mat;
  }

  LMatrix4 inv = invert(mat);

  for (CharacterJointPoseData &joint : _joint_poses) {
    joint._initial_net_transform_inverse = inv * joint._initial_net_transform_inverse;
  }
}

/**
 * Creates a new CharacterJoint with the indicated name and parent
 * joint, and returns the index of the new joint.
 */
int Character::
make_joint(const std::string &name, int parent, const LMatrix4 &default_value) {
  int index = (int)_joints.size();

  CharacterJoint joint(name);
  joint._index = index;
  joint._default_value = default_value;
  // Break out the components as well.
  LVecBase3 hpr;
  decompose_matrix(default_value, joint._default_scale,
                   joint._default_shear, hpr, joint._default_pos);
  joint._default_quat.set_hpr(hpr);

  if (parent != -1) {
    nassertr(parent < (int)_joints.size(), -1);
    _joints[parent]._children.push_back(joint._index);
  }

  CharacterJointPoseData pose;
  pose._parent = parent;
  pose._value = default_value;
  pose._net_transform = LMatrix4::ident_mat();
  pose._has_forced_value = false;
  pose._merge_joint = -1;
  pose._vertex_transform = nullptr;

  _joints.push_back(joint);
  _joint_poses.push_back(pose);

  recompute_joint_net_transform(index);

  _joint_poses[index]._initial_net_transform_inverse = invert(_joint_poses[index]._net_transform);

  return index;
}

/**
 * Creates a new CharacterSlider with the indicated name, and returns the index
 * of the new slider.
 */
int Character::
make_slider(const std::string &name, PN_stdfloat default_value) {
  CharacterSlider slider(name);
  slider._default_value = default_value;
  size_t slider_index = _sliders.size();
  _sliders.push_back(slider);
  return slider_index;
}

/**
 * Binds the indicated AnimChannelTable to this Character.  Matches up joints
 * in the character to joints in the animation with the same name.  The results
 * are stored on the character for access by the AnimChannelTable when
 * computing the animation.
 */
bool Character::
bind_anim(AnimChannelTable *anim) {
  ChannelBindings::const_iterator it = _channel_bindings.find(anim);
  if (it != _channel_bindings.end()) {
    // Animation is already bound.
    return true;
  }

  // We need to map our joints and sliders to joints and sliders on the
  // animation.
  ChannelBindings::iterator iit = _channel_bindings.insert(
    ChannelBindings::value_type(anim, ChannelBinding())).first;
  vector_int &joint_map = (*iit).second._joint_map;
  vector_int &slider_map = (*iit).second._slider_map;

  int num_anim_joints = (int)anim->get_joint_names().size();
  joint_map.resize(num_anim_joints);
  slider_map.resize(get_num_sliders());

  for (int anim_joint = 0; anim_joint < num_anim_joints; anim_joint++) {
    const std::string &anim_joint_name = anim->get_joint_names()[anim_joint];
    int cjoint = find_joint(anim_joint_name);
    if (cjoint == -1) {
      // The character doesn't have this joint from the animation.  We can deal
      // with it, but give a warning about it, because this might be a mistake.
      anim_cat.warning()
        << "Joint " << anim_joint_name << " in animation " << anim->get_name()
        << " does not exist on Character " << get_name() << "\n";
    }
    joint_map[anim_joint] = cjoint;
  }
  for (int slider = 0; slider < get_num_sliders(); slider++) {
    const std::string &slider_name = get_slider_name(slider);
    int anim_slider = anim->find_slider_channel(slider_name);
    if (anim_slider == -1) {
      // This character slider doesn't appear in the animation.  We can deal
      // with it, but give a warning about it, because this might be a mistake.
      anim_cat.warning()
        << "Character slider " << slider_name << " does not appear in animation " << anim->get_name() << "\n";
    }
    slider_map[slider] = anim_slider;
  }

  return true;
}

/**
 * Updates all joints and sliders in the character to reflect the animation for
 * the current frame.
 *
 * Returns true if something in the character changed as a result of this,
 * false otherwise.
 */
bool Character::
update() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, false, current_thread);

  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now > cdata->_last_update + _update_delay || cdata->_anim_changed) {
    return do_update(now, cdata, current_thread);

  } else {
    return false;
  }
}

/**
 * Internal method that advances the animation time for all layers.
 */
void Character::
do_advance(double now, CData *cdata, Thread *current_thread) {
  // We must have at least 1 layer at all times, even if no animations are
  // playing.
  nassertv(!_anim_layers.empty());

  double dt = ClockObject::get_global_clock()->get_dt();

  // Advance our layers.
  for (int i = 0; i < (int)_anim_layers.size(); i++) {
    AnimLayer *layer = &_anim_layers[i];

    if (layer->is_active()) {
      if (layer->is_killme()) {
        if (anim_cat.is_debug()) {
          anim_cat.debug()
            << "Layer " << i << " is active and killme\n";
        }
        if (layer->_kill_delay > 0.0f) {
          if (anim_cat.is_debug()) {
            anim_cat.debug()
              << "Layer " << i << " kill delay " << layer->_kill_delay << "\n";
          }
          layer->_kill_delay -= dt;
          layer->_kill_delay = std::max(0.0f, std::min(1.0f, layer->_kill_delay));

        } else if (layer->_kill_weight != 0.0f) {
          // Give it at least one frame advance cycle to propagate 0.0 to client.
          layer->_kill_weight -= layer->_kill_rate * dt;
          layer->_kill_weight = std::max(0.0f, std::min(1.0f, layer->_kill_weight));
          if (anim_cat.is_debug()) {
            anim_cat.debug()
              << "Layer " << i << " kill weight " << layer->_kill_weight << "\n";
          }

        } else {
          // Shift the other layers down in order
          //fast_remove_layer(i);
          // Needs at least one thing cycle dead to trigger sequence change.
          if (anim_cat.is_debug()) {
            anim_cat.debug()
              << "Layer " << i << " killme now dying\n";
          }
          layer->dying();
          continue;
        }
      }

      layer->update();

      if (layer->_sequence_finished && layer->is_autokill()) {
        layer->_kill_weight = 0.0f;
        layer->killme();
      }

      layer->_weight = layer->_kill_weight * layer->_ramp_weight;

    } else if (layer->is_dying()) {
      layer->dead();

    } else if (layer->_weight > 0.0f) {
      // Now that the server blends, it is turning off layers all the time.
      layer->init(this);
      layer->dying();
    }
  }
}

/**
 * Internal method that actually computes the animation for the character.
 */
bool Character::
do_update(double now, CData *cdata, Thread *current_thread) {
  if ((int)_joints.size() > max_character_joints) {
    anim_cat.error()
      << "Too many joints on character " << get_name() << "\n";
    return false;
  }

  // We must have at least 1 layer at all times, even if no animations are
  // playing.
  nassertr(!_anim_layers.empty(), false);

  // If we are auto advancing animation time, do that now.
  if (cdata->_auto_advance_flag) {
    do_advance(now, cdata, current_thread);
  }

  // Initialize the context for the evaluation.
  AnimEvalContext ctx;
  ClearBitString(ctx._joint_mask, max_character_joints);
  ctx._character = this;
  ctx._joints = _joints.data();
  ctx._num_joints = (int)_joints.size();
  // Set up number of SIMD joint groups.  Pad to ensure it is an exact multiple
  // of the SIMD vector width.
  ctx._num_joint_groups = simd_align_value(ctx._num_joints, SIMDFloatVector::num_columns) / SIMDFloatVector::num_columns;
  ctx._frame_blend = cdata->_frame_blend_flag;
  ctx._time = now;

  for (size_t i = 0; i < _joint_poses.size(); i++) {
    if (_joint_poses[i]._merge_joint == -1 && !_joint_poses[i]._has_forced_value) {
      // We need to calculate this joint in the evaluation.
      SetBit(ctx._joint_mask, i);
    }
  }

  // Read in the local transform of any controller nodes into the joint's
  // forced value.
  for (size_t i = 0; i < _joints.size(); i++) {
    PandaNode *controller = _joints[i]._controller;
    if (controller != nullptr) {
      _joint_poses[i]._forced_value = controller->get_transform()->get_mat();
    }
  }

  AnimEvalData data;
  // Apply the bind poses of each joint as the starting point.
  if (!_built_bind_pose) {
    // Cache the bind pose on the character and then just copy the poses
    // from here on out.
    for (size_t i = 0; i < _joints.size(); i++) {
      int group = i / SIMDFloatVector::num_columns;
      int sub = i % SIMDFloatVector::num_columns;
      _bind_pose._pose[group].pos.set_lvec(sub, _joints[i]._default_pos);
      _bind_pose._pose[group].scale.set_lvec(sub, _joints[i]._default_scale);
      _bind_pose._pose[group].shear.set_lvec(sub, _joints[i]._default_shear);
      _bind_pose._pose[group].quat.set_lquat(sub, _joints[i]._default_quat);
    }
    _built_bind_pose = true;
  }
  data.copy_pose(_bind_pose, ctx._num_joint_groups);

  //
  // Evaluate our layers.
  //

  // Sort the layers.
  int *layer = (int *)alloca(sizeof(int) * _anim_layers.size());
  for (int i = 0; i < (int)_anim_layers.size(); i++) {
    layer[i] = -1;
  }

  for (int i = 0; i < (int)_anim_layers.size(); i++) {
    AnimLayer *thelayer = &_anim_layers[i];
    if ((thelayer->_weight > 0.0f) &&
        (thelayer->is_active()) &&
        (thelayer->_order >= 0) &&
        (thelayer->_order < (int)_anim_layers.size())) {
      layer[thelayer->_order] = i;
    }
  }

  for (int i = 0; i < (int)_anim_layers.size(); i++) {
    if (layer[i] < 0 || layer[i] >= (int)_anim_layers.size()) {
      continue;
    }

    AnimLayer *thelayer = &_anim_layers[layer[i]];
    if ((thelayer->_sequence >= 0) &&
        (thelayer->_sequence < (int)_channels.size())) {

      thelayer->calc_pose(ctx, data, cdata->_channel_transition_flag && (layer[i] == 0));
    }
  }

  // Now apply the evaluated pose to the joints.
  bool any_changed = apply_pose(cdata, cdata->_root_xform, data, current_thread);

  cdata->_anim_changed = false;
  cdata->_last_update = now;

  return any_changed;
}

/**
 * Forces the character to update all joints and sliders to reflect the
 * animation for the current frame, regardless of whether we think it needs to.
 */
bool Character::
force_update() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, false, current_thread);
  double now = ClockObject::get_global_clock()->get_frame_time();

  return do_update(now, cdata, current_thread);
}

/**
 * Recomputes the net transforms for all joints in the character.
 */
void Character::
recompute_joint_net_transforms() {
  for (size_t i = 0; i < _joints.size(); i++) {
    recompute_joint_net_transform(i);
  }
}

/**
 * Recomputes the net transforms for the indicated joint.
 */
void Character::
recompute_joint_net_transform(int i) {
  nassertv(i >= 0 && i < (int)_joint_poses.size());

  CharacterJointPoseData &joint = _joint_poses[i];

  if (joint._parent != -1) {
    joint._net_transform = joint._value * _joint_poses[joint._parent]._net_transform;

  } else {
    joint._net_transform = joint._value * get_root_xform();
  }
}

/**
 * Adds a new attachment with the indicated name to the character.
 */
int Character::
add_attachment(const std::string &name) {
  int index = _attachments.size();
  _attachments.push_back(CharacterAttachment(name));
  return index;
}

/**
 * Adds a new parent influence to the indicated attachment.
 */
void Character::
add_attachment_parent(int n, int parent, const LPoint3 &local_pos,
                      const LVecBase3 &local_hpr, float weight) {
  nassertv(n >= 0 && n < (int)_attachments.size());

  CharacterAttachment &attach = _attachments[n];
  CharacterAttachment::ParentInfluence inf;
  inf._parent = parent;
  inf._offset = LMatrix4(TransformState::make_pos_hpr(local_pos, local_hpr)->get_mat());
  inf._weight = weight;
  inf._transform = LMatrix4::ident_mat();
  if (parent == -1) {
    inf._transform = inf._offset * inf._weight;
  } //else {
    //_changed_joints.set_bit(parent);
  //}
  attach._parents.push_back(std::move(inf));

  compute_attachment_transform(n);
}

/**
 * Removes the indicated parent from the indicated attachment's set of parent
 * influences.
 */
void Character::
remove_attachment_parent(int n, int parent) {
  nassertv(n >= 0 && n < (int)_attachments.size());
  CharacterAttachment &attach = _attachments[n];
  for (auto it = attach._parents.begin(); it != attach._parents.end(); ++it) {
    if ((*it)._parent == parent) {
      attach._parents.erase(it);
      break;
    }
  }
}

/**
 * Sets the node that should receive the attachment's net transform from the
 * root.
 */
void Character::
set_attachment_node(int n, PandaNode *node) {
  nassertv(n >= 0 && n < (int)_attachments.size());

  CharacterAttachment &attach = _attachments[n];

  if (attach._node != nullptr) {
    CPT(RenderEffect) effect = attach._node->get_effect(CharacterJointEffect::get_class_type());
    if (effect != nullptr &&
        DCAST(CharacterJointEffect, effect)->matches_character(_active_owner)) {
      attach._node->clear_effect(CharacterJointEffect::get_class_type());
    }
  }

  attach._node = node;

  if (attach._node != nullptr) {
    attach._node->set_effect(CharacterJointEffect::make(_active_owner));
    attach._node->set_transform(attach._curr_transform);
  }
}

/**
 * Clears the current node that should receive the net transform from the
 * root of the indicated attachment.
 */
void Character::
clear_attachment_node(int n) {
  set_attachment_node(n, nullptr);
}

/**
 * Returns the node that should receive the indicated attachment's net
 * transform from the root.
 */
PandaNode *Character::
get_attachment_node(int n) const {
  nassertr(n >= 0 && n < (int)_attachments.size(), nullptr);
  return _attachments[n]._node;
}

/**
 * Returns the attachment's current net transform from the root.
 */
const TransformState *Character::
get_attachment_transform(int n) const {
  nassertr(n >= 0 && n < (int)_attachments.size(), TransformState::make_identity());
  return _attachments[n]._curr_transform;
}

/**
 * Returns the current transform of the attachment in world coordinates.  This
 * uses the associated PandaNode to compute the transform, so if no node is
 * associated, it will return the transform relative to the root of the
 * character.
 */
CPT(TransformState) Character::
get_attachment_net_transform(int n) const {
  nassertr(n >= 0 && n < (int)_attachments.size(), TransformState::make_identity());
  const CharacterAttachment &attach = _attachments[n];
  if (attach._node == nullptr) {
    return attach._curr_transform;
  }
  return NodePath(attach._node).get_net_transform();
}

/**
 * Returns the number of attachments in the character.
 */
int Character::
get_num_attachments() const {
  return _attachments.size();
}

/**
 * Returns the index of the attachment with the indicatd name, or -1 if no
 * such attachment exists.
 */
int Character::
find_attachment(const std::string &name) const {
  for (size_t i = 0; i < _attachments.size(); i++) {
    if (_attachments[i].get_name() == name) {
      return i;
    }
  }

  return -1;
}

/**
 * Computes the indicated attachment's net transform from the root.
 */
void Character::
compute_attachment_transform(int index) {
  nassertv(index >= 0 && index < (int)_attachments.size());

  CharacterAttachment &attach = _attachments[index];
  LMatrix4 transform = LMatrix4::zeros_mat();
  for (auto it = attach._parents.begin(); it != attach._parents.end(); ++it) {
    CharacterAttachment::ParentInfluence &inf = *it;
    if (inf._parent != -1) {
      //if (!_changed_joints.get_bit(parent)) {
      //  continue;
      //}
      inf._transform = (inf._offset * _joint_poses[inf._parent]._net_transform) * inf._weight;
    }
    transform += inf._transform;
  }
  if (!transform.is_nan()) {
    attach._curr_transform = TransformState::make_mat(transform);
  } else {
    attach._curr_transform = TransformState::make_identity();
  }
  if (attach._node != nullptr) {
    attach._node->set_transform(attach._curr_transform);
  }
}

/**
 * Removes the attachment from the character at the indicated index.
 */
void Character::
remove_attachment(int attachment) {
  nassertv(attachment >= 0 && attachment < (int)_attachments.size());
  _attachments.erase(_attachments.begin() + attachment);
}

/**
 * Removes all attachments from the character.
 */
void Character::
remove_all_attachments() {
  _attachments.clear();
}

/**
 * Adds a new IK chain to the character and returns the index of the chain.
 */
int Character::
add_ik_chain(const std::string &name, int top_joint, int middle_joint,
             int end_joint, const LVector3 &middle_dir,
             const LPoint3 &center, PN_stdfloat height,
             PN_stdfloat floor, PN_stdfloat pad) {
  IKChain chain(name, top_joint, middle_joint, end_joint);
  chain.set_middle_joint_direction(middle_dir);
  chain.set_center(center);
  chain.set_height(height);
  chain.set_floor(floor);
  chain.set_pad(pad);
  return add_ik_chain(std::move(chain));
}

/**
 * Copies the indicated IK chain to the character and returns the index of the
 * chain.
 */
int Character::
add_ik_chain(const IKChain &chain) {
  int index = (int)_ik_chains.size();
  _ik_chains.push_back(chain);
  return index;
}

/**
 * Moves the indicated IK chain to the character and returns the index of the
 * chain.
 */
int Character::
add_ik_chain(IKChain &&chain) {
  int index = (int)_ik_chains.size();
  _ik_chains.push_back(std::move(chain));
  return index;
}

/**
 *
 */
int Character::
add_ik_target() {
  int index = (int)_ik_targets.size();
  _ik_targets.push_back(IKTarget());
  return index;
}

/**
 *
 */
PT(Character) Character::
make_copy() const {
  return new Character(*this);
}

/**
 *
 */
PT(Character) Character::
copy_subgraph() const {
  PT(Character) copy = make_copy();

  copy->_joints = _joints;
  copy->_joint_poses = _joint_poses;
  copy->_bind_pose.copy_pose(_bind_pose, simd_align_value(_joint_poses.size(), SIMDFloatVector::num_columns) / SIMDFloatVector::num_columns);
  copy->_built_bind_pose = _built_bind_pose;

  // Don't inherit the vertex transforms.
  for (size_t i = 0; i < _joint_poses.size(); i++) {
    copy->_joint_poses[i]._vertex_transform = nullptr;
  }

  copy->_sliders = _sliders;

  copy->_channels = _channels;
  copy->_channel_bindings = _channel_bindings;
  copy->_pose_parameters = _pose_parameters;
  copy->_attachments = _attachments;
  copy->_ik_chains = _ik_chains;

  return copy;
}

/**
 * Applies the final pose computed by the animation graph to each joint.
 */
bool Character::
apply_pose(CData *cdata, const LMatrix4 &root_xform, const AnimEvalData &data, Thread *current_thread) {
  PStatTimer timer(apply_pose_collector);

  Character *merge_char = cdata->_joint_merge_character;
  if (merge_char != nullptr) {
    if (merge_char->_active_owner == nullptr) {
      merge_char = nullptr;
    }
  }
  LMatrix4 parent_to_me;
  if (merge_char != nullptr) {
    // Make sure the parent character's animation is up-to-date.
    // Update through the managing CharacterNode so the lock gets
    // acquired.
    merge_char->_active_owner->update();

    // Compute the matrix that will transform joints from the parent
    // coordinate space to my coordinate space.
    NodePath my_path = NodePath::any_path(_active_owner);
    NodePath parent_path = NodePath::any_path(merge_char->_active_owner);
    parent_to_me = parent_path.get_transform(my_path)->get_mat();
  }

  ap_compose_collector.start();

  size_t joint_count = _joints.size();

  for (size_t i = 0; i < joint_count; i++) {
    CharacterJointPoseData &joint = _joint_poses[i];

    if (joint._merge_joint == -1) {

      if (!joint._has_forced_value) {
        // Use the transform calculated during the channel evaluation.
        int group = i / SIMDFloatVector::num_columns;
        int sub = i % SIMDFloatVector::num_columns;
        joint._value = LMatrix4::scale_shear_mat(data._pose[group].scale.get_lvec(sub), data._pose[group].shear.get_lvec(sub)) * data._pose[group].quat.get_lquat(sub);
        joint._value.set_row(3, data._pose[group].pos.get_lvec(sub));

      } else {
        // Take the local transform from the forced value.
        joint._value = joint._forced_value;
      }

      // Now compute the net transform.
      if (joint._parent != -1) {
        joint._net_transform = joint._value * _joint_poses[joint._parent]._net_transform;
      } else {
        joint._net_transform = joint._value * root_xform;
      }

    } else if (merge_char != nullptr) {
      // Use the transform of the parent merge joint.

      // Re-compute this joint's local transform such that it ends up
      // with the same world-space transform as the parent merge joint.

      const LMatrix4 &parent_net = merge_char->_joint_poses[joint._merge_joint]._net_transform;
      joint._net_transform = parent_net * parent_to_me;
      if (joint._parent != -1) {
        LMatrix4 parent_inverse = invert(_joint_poses[joint._parent]._net_transform);
        joint._value = joint._net_transform * parent_inverse;

      } else {
        joint._value = joint._net_transform;
      }
    }

    // Compute the skinning matrix to transform the vertices.
    if (joint._vertex_transform != nullptr) {
      joint._vertex_transform->set_matrix(joint._initial_net_transform_inverse * joint._net_transform, current_thread);
    }
  }
  ap_compose_collector.stop();

  ap_update_net_transform_nodes.start();
  // Compute attachment transforms from the updated character pose.
  for (size_t i = 0; i < _attachments.size(); i++) {
    compute_attachment_transform(i);
  }
  ap_update_net_transform_nodes.stop();

  // Also update slider values... this is temporary.
  //for (size_t i = 0; i < _sliders.size(); i++) {
  //  _sliders[i].update(current_thread);
  //}

  return true;
}

/**
 * Adds the PartBundleNode pointer to the set of nodes associated with the
 * PartBundle.  Normally called only by the PartBundleNode itself, for
 * instance when the bundle is flattened with another node.
 */
void Character::
add_node(CharacterNode *node) {
  nassertv(find(_nodes.begin(), _nodes.end(), node) == _nodes.end());
  _nodes.push_back(node);

  update_active_owner(_active_owner, node);
}

/**
 * Removes the PartBundleNode pointer from the set of nodes associated with
 * the PartBundle.  Normally called only by the PartBundleNode itself, for
 * instance when the bundle is flattened with another node.
 */
void Character::
remove_node(CharacterNode *node) {
  Nodes::iterator ni = find(_nodes.begin(), _nodes.end(), node);
  if (ni == _nodes.end()) {
    return;
  }
  _nodes.erase(ni);

  if (get_num_nodes() > 0) {
    update_active_owner(_active_owner, get_node(get_num_nodes() - 1));

  } else {
    update_active_owner(_active_owner, nullptr);
  }
}

/**
 * Builds the mapping of parent joints with joint merge enabled to the
 * corresponding joints on this character.
 */
void Character::
build_joint_merge_map(Character *merge_char) {
  {
    CDReader cdata(_cycler);
    if (cdata->_joint_merge_character == merge_char) {
      return;
    }
  }

  if (merge_char == nullptr) {
    for (size_t i = 0; i < _joint_poses.size(); i++) {
      _joint_poses[i]._merge_joint = -1;
    }

  } else {
    for (size_t i = 0; i < _joint_poses.size(); i++) {
      CharacterJointPoseData &joint = _joint_poses[i];

      // See if the parent character has a joint with this name.
      int parent_joint_idx = merge_char->find_joint(_joints[i].get_name());
      if (parent_joint_idx == -1) {
        joint._merge_joint = -1;
        continue;
      }

      // It does, see if joint merge is enabled on the parent joint.
      CharacterJoint &parent_joint = merge_char->_joints[parent_joint_idx];

      if (parent_joint._merge) {
        // Joint merge is enabled, so our joint will take the transform from
        // the parent character's joint.
        joint._merge_joint = parent_joint_idx;
      } else {
        joint._merge_joint = -1;
      }
    }
  }
}

/**
 * Updates the active CharacterNode owner of this Character.  Redirects the
 * CharacterJointEffects to the new owner.
 */
void Character::
update_active_owner(CharacterNode *old_owner, CharacterNode *new_owner) {
  if (old_owner == new_owner) {
    return;
  }

  for (size_t i = 0; i < _attachments.size(); i++) {
    CharacterAttachment &attach = _attachments[i];
    if (new_owner != nullptr) {
      // Change or set a _character pointer on each joint's exposed node.
      if (attach._node != nullptr) {
        attach._node->set_effect(CharacterJointEffect::make(new_owner));
      }

    } else {
      CPT(RenderEffect) effect = attach._node->get_effect(CharacterJointEffect::get_class_type());
      if (effect != nullptr &&
          DCAST(CharacterJointEffect, effect)->matches_character(old_owner)) {
        attach._node->clear_effect(CharacterJointEffect::get_class_type());
      }
    }
  }

  _active_owner = new_owner;
}

/**
 *
 */
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void Character::
finalize(BamReader *) {
  //Thread *current_thread = Thread::get_current_thread();
  //CDWriter cdata(_cycler, true);
  //do_update(this, cdata, nullptr, true, true, current_thread);
}

/**
 *
 */
void Character::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());

  me.add_int16((int)_joints.size());
  for (int i = 0; i < (int)_joints.size(); i++) {
    _joints[i].write_datagram(me);

    const CharacterJointPoseData &pose = _joint_poses[i];
    me.add_int16(pose._parent);
    pose._value.write_datagram(me);
    pose._net_transform.write_datagram(me);
    pose._initial_net_transform_inverse.write_datagram(me);
  }

  me.add_int16((int)_sliders.size());
  for (int i = 0; i < (int)_sliders.size(); i++) {
    _sliders[i].write_datagram(me);
  }

  me.add_uint8(_pose_parameters.size());
  for (size_t i = 0; i < _pose_parameters.size(); i++) {
    _pose_parameters[i].write_datagram(manager, me);
  }

  me.add_uint8(_attachments.size());
  for (size_t i = 0; i < _attachments.size(); i++) {
    _attachments[i].write_datagram(manager, me);
  }

  me.add_uint8(_ik_chains.size());
  for (size_t i = 0; i < _ik_chains.size(); i++) {
    _ik_chains[i].write_datagram(manager, me);
  }

  me.add_uint16(_channels.size());
  for (size_t i = 0; i < _channels.size(); i++) {
    manager->write_pointer(me, _channels[i]);
  }

  me.add_uint16(_channel_bindings.size());
  for (auto it = _channel_bindings.begin(); it != _channel_bindings.end(); ++it) {
    manager->write_pointer(me, (*it).first);
    for (size_t i = 0; i < _joints.size(); i++) {
      me.add_int16((*it).second._joint_map[i]);
    }
    for (size_t i = 0; i < _sliders.size(); i++) {
      me.add_int16((*it).second._slider_map[i]);
    }
  }

  manager->write_cdata(me, _cycler);
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int Character::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  for (size_t i = 0; i < _attachments.size(); i++) {
    pi = _attachments[i].complete_pointers(pi, p_list, manager);
  }

  for (size_t i = 0; i < _channels.size(); i++) {
    _channels[i] = DCAST(AnimChannel, p_list[pi++]);
  }

  for (size_t i = 0; i < _read_bindings.size(); i++) {
    AnimChannel *chan = DCAST(AnimChannel, p_list[pi++]);
    _channel_bindings[chan] = _read_bindings[i];
  }
  _read_bindings.clear();

  return pi;
}

/**
 *
 */
TypedWritable *Character::
make_from_bam(const FactoryParams &params) {
  Character *object = new Character("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);
  return object;
}

/**
 *
 */
void Character::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  _joints.resize(scan.get_int16());
  _joint_poses.resize(_joints.size());
  for (size_t i = 0; i < _joints.size(); i++) {
    _joints[i].read_datagram(scan);

    CharacterJointPoseData &pose = _joint_poses[i];
    pose._parent = scan.get_int16();
    pose._value.read_datagram(scan);
    pose._net_transform.read_datagram(scan);
    pose._initial_net_transform_inverse.read_datagram(scan);
    pose._has_forced_value = false;
    pose._merge_joint = -1;
    pose._vertex_transform = nullptr;
  }

  _sliders.resize(scan.get_int16());
  for (size_t i = 0; i < _sliders.size(); i++) {
    _sliders[i].read_datagram(scan);
  }

  _pose_parameters.resize(scan.get_uint8());
  for (size_t i = 0; i < _pose_parameters.size(); i++) {
    _pose_parameters[i].fillin(scan, manager);
  }

  _attachments.resize(scan.get_uint8());
  for (size_t i = 0; i < _attachments.size(); i++) {
    _attachments[i].fillin(scan, manager);
  }

  _ik_chains.resize(scan.get_uint8());
  for (size_t i = 0; i < _ik_chains.size(); i++) {
    _ik_chains[i].fillin(scan, manager);
  }

  _channels.resize(scan.get_uint16());
  manager->read_pointers(scan, _channels.size());

  _read_bindings.resize(scan.get_uint16());
  for (size_t i = 0; i < _read_bindings.size(); i++) {
    manager->read_pointer(scan);
    _read_bindings[i]._joint_map.resize(_joints.size());
    _read_bindings[i]._slider_map.resize(_sliders.size());
    for (size_t j = 0; j < _joints.size(); j++) {
      _read_bindings[i]._joint_map[j] = scan.get_int16();
    }
    for (size_t j = 0; j < _sliders.size(); j++) {
      _read_bindings[i]._slider_map[j] = scan.get_int16();
    }
  }

  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
Character::CData::
CData() :
  _frame_blend_flag(interpolate_frames),
  _auto_advance_flag(true),
  _channel_transition_flag(true),
  _anim_changed(false),
  _last_update(0.0),
  _root_xform(LMatrix4::ident_mat()),
  _joint_merge_character(nullptr)
{
}

/**
 *
 */
Character::CData::
CData(const Character::CData &copy) :
  _frame_blend_flag(copy._frame_blend_flag),
  _auto_advance_flag(copy._auto_advance_flag),
  _channel_transition_flag(copy._channel_transition_flag),
  _root_xform(copy._root_xform),
  _anim_changed(copy._anim_changed),
  _last_update(copy._last_update),
  _joint_merge_character(copy._joint_merge_character)
{
  // Note that this copy constructor is not used by the PartBundle copy
  // constructor!  Any elements that must be copied between PartBundles should
  // also be explicitly copied there.
}

/**
 *
 */
CycleData *Character::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Character::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_bool(_frame_blend_flag);
  _root_xform.write_datagram(dg);

  // The remaining members are strictly dynamic.
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PartBundle.
 */
void Character::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _frame_blend_flag = scan.get_bool();
  _root_xform.read_datagram(scan);
}

/**
 * Returns a suitable channel to use for the indicated activity number.
 * If multiple channels are part of the same activity, the channel is chosen
 * at random based on assigned weight.  An explicit seed may be given for the
 * random number generator, in case the selected channel needs to be
 * consistent, for instance during client-side prediction.
 */
int Character::
get_channel_for_activity(int activity, int curr_channel, unsigned long seed) const {
  if (_channels.empty()) {
    return -1;
  }

  Randomizer random(seed);

  int weight_total = 0;
  int chan_idx = -1;
  bool got_chan = false;
  for (int i = 0; i < (int)_channels.size() && !got_chan; i++) {
    AnimChannel *channel = _channels[i];
    for (int j = 0; j < channel->get_num_activities(); j++) {
      int curr_activity = channel->get_activity(j);
      int weight = channel->get_activity_weight(j);
      if (curr_activity == activity) {
        if (curr_channel == i && weight < 0) {
          // If this is the current sequence and the weight is < 0, stick with
          // this sequence.
          chan_idx = i;
          got_chan = true;
          break;
        } else {
          weight_total += std::abs(weight);

          int random_value = random.random_int(weight_total);
          if (!weight_total || random_value < std::abs(weight)) {
            chan_idx = i;
          }
        }
      }
    }
  }

  return chan_idx;
}

/**
 *
 */
void Character::
set_joint_vertex_transform(JointVertexTransform *transform, int n) {
  nassertv(n >= 0 && n < (int)_joint_poses.size());
  _joint_poses[n]._vertex_transform = transform;
}

/**
 * Collects AnimChannel events from all playing layers.
 */
void Character::
get_events(AnimEventQueue &queue, int type) {
  for (size_t i = 0; i < _anim_layers.size(); i++) {
    _anim_layers[i].get_events(queue, type);
  }
}

/**
 * Plays the indicated animation channel on the indicated layer completely
 * through once and stops.
 */
void Character::
play(int channel, int layer, PN_stdfloat play_rate, bool autokill,
     PN_stdfloat blend_in, PN_stdfloat blend_out) {
  nassertv(channel >= 0 && channel < (int)_channels.size());
  AnimChannel *chan = _channels[channel];
  play(channel, 0.0, chan->get_num_frames() - 1, layer, play_rate,
       autokill, blend_in, blend_out);
}

/**
 * Plays the indicated animation channel on the indicated layer once,
 * constrained to the indicated frame range, and stops.
 */
void Character::
play(int channel, double from, double to, int layer, PN_stdfloat play_rate,
     bool autokill, PN_stdfloat blend_in, PN_stdfloat blend_out) {
  if (from >= to) {
    pose(channel, from, layer, blend_in, blend_out);
    return;
  }

  PN_stdfloat start_time = ClockObject::get_global_clock()->get_frame_time();
  reset_layer_channel(layer, channel, -1, true,
                      start_time,
                      from, to, AnimLayer::PM_play, play_rate,
                      autokill, blend_in, blend_out);
}

/**
 * Loops the indicated animation channel on the indicated layer repeatedly.
 */
void Character::
loop(int channel, bool restart, int layer, PN_stdfloat play_rate,
     PN_stdfloat blend_in) {
  nassertv(channel >= 0 && channel < (int)_channels.size());
  AnimChannel *chan = _channels[channel];
  loop(channel, restart, 0.0, chan->get_num_frames() - 1, layer,
       play_rate, blend_in);
}

/**
 *
 */
void Character::
loop(int channel, bool restart, double from, double to, int layer,
     PN_stdfloat play_rate, PN_stdfloat blend_in) {
  if (from >= to) {
    pose(channel, from, layer, blend_in, 0.0);
    return;
  }

  PN_stdfloat start_time = ClockObject::get_global_clock()->get_frame_time();
  //if (!restart) {

  //}
  reset_layer_channel(layer, channel, -1, true, start_time,
                      from, to, AnimLayer::PM_loop, play_rate,
                      false, blend_in, 0.0f);
}

/**
 * Plays the indicated animation channel on the indicated layer back and forth
 * repeatedly.
 */
void Character::
pingpong(int channel, bool restart, int layer, PN_stdfloat play_rate,
         PN_stdfloat blend_in) {
  nassertv(channel >= 0 && channel < (int)_channels.size());
  AnimChannel *chan = _channels[channel];
  pingpong(channel, restart, 0.0, chan->get_num_frames() - 1, layer,
           play_rate, blend_in);
}

/**
 *
 */
void Character::
pingpong(int channel, bool restart, double from, double to, int layer,
         PN_stdfloat play_rate, PN_stdfloat blend_in) {
  if (from >= to) {
    pose(channel, from, layer, blend_in, 0.0);
    return;
  }

  PN_stdfloat start_time = ClockObject::get_global_clock()->get_frame_time();
  //if (!restart) {

  //}
  reset_layer_channel(layer, channel, -1, true, start_time,
                      from, to, AnimLayer::PM_pingpong, play_rate,
                      false, blend_in, 0.0f);
}

/**
 * Holds a particular frame of the indicated animation channel on the indicated
 * layer.
 */
void Character::
pose(int channel, double frame, int layer, PN_stdfloat blend_in, PN_stdfloat blend_out) {
  PN_stdfloat start_time = ClockObject::get_global_clock()->get_frame_time();
  reset_layer_channel(layer, channel, -1, false, start_time, frame, frame,
                      AnimLayer::PM_pose, 1.0f, false, blend_in, blend_out);
}

/**
 * Stops whatever animation channel is playing on the indicated layer.  If -1
 * is passed, all layers are stopped.  If kill is true, the layer(s) will be
 * faded out instead of immediately stopped.
 */
void Character::
stop(int layer, bool kill) {
  if (layer < 0) {
    for (int i = 0; i < (int)_anim_layers.size(); i++) {
      if (kill) {
        _anim_layers[i].killme();
      } else {
        _anim_layers[i].dying();
      }
    }
  } else {
    nassertv(layer < (int)_anim_layers.size());
    if (kill) {
      _anim_layers[layer].killme();
    } else {
      _anim_layers[layer].dying();
    }
  }
}


/**
 * Resets the indicated animation layer to start playing the indicated
 * channel.
 */
void Character::
reset_layer_channel(int layer, int channel, int activity, bool restart, PN_stdfloat start_time,
                    PN_stdfloat from, PN_stdfloat to, AnimLayer::PlayMode mode,
                    PN_stdfloat play_rate, bool autokill, PN_stdfloat blend_in,
                    PN_stdfloat blend_out) {
  nassertv(channel >= 0 && channel < (int)_channels.size());
  ensure_layer_count(layer + 1);

  AnimChannel *chan = _channels[channel];

  int num_frames = std::max(1, chan->get_num_frames());
  PN_stdfloat from_cycle = AnimTimer::frame_to_cycle(from, num_frames);
  PN_stdfloat play_cycles = AnimTimer::frame_to_cycle(to - from + 1.0f, num_frames);

  AnimLayer *alayer = get_anim_layer(layer);
  if (restart || channel != alayer->_sequence) {
    // Bump the parity to note that the sequence changed.
    alayer->_sequence_parity = (alayer->_sequence_parity + 1) % 256;
  }
  alayer->_sequence = channel;
  alayer->_unclamped_cycle = from_cycle;
  alayer->_cycle = from_cycle;
  alayer->_prev_cycle = from_cycle;
  alayer->_start_cycle = from_cycle;
  alayer->_play_cycles = play_cycles;
  alayer->_activity = activity;
  alayer->_order = layer;
  alayer->_priority = 0;
  alayer->_play_rate = play_rate;
  alayer->_weight = 1.0f;
  alayer->_ramp_weight = 1.0f;
  alayer->_kill_weight = 1.0f;
  alayer->_blend_in = blend_in;
  alayer->_blend_out = blend_out;
  alayer->_sequence_finished = false;
  alayer->_last_event_check = 0;
  alayer->_play_mode = mode;
  alayer->_flags = AnimLayer::F_active;
  if (autokill) {
    alayer->_flags |= AnimLayer::F_autokill;
  }
  alayer->mark_active();
}

/**
 * Ensures that the character contains at least the indicated number of
 * animation layers.  If not, they will be allocated.
 */
void Character::
ensure_layer_count(int count) {
  int diff = count - (int)_anim_layers.size();
  for (int i = 0; i < diff; i++) {
    _anim_layers.push_back(AnimLayer());
    _anim_layers.back().init(this);
  }
}

/**
 *
 */
void Character::
advance() {
  CDWriter cdata(_cycler);
  Thread *current_thread = Thread::get_current_thread();
  double now = ClockObject::get_global_clock()->get_frame_time();
  do_advance(now, cdata, current_thread);
}
