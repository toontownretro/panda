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
#include "animBundleNode.h"
//#include "bindAnimRequest.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "characterJointEffect.h"
#include "randomizer.h"

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
  _anim_preload = copy._anim_preload;
  _update_delay = 0.0;
  _active_owner = nullptr;

  CDWriter cdata(_cycler, true);
  CDReader cdata_from(copy._cycler);
  cdata->_frame_blend_flag = cdata_from->_frame_blend_flag;
  cdata->_root_xform = cdata_from->_root_xform;
}

/**
 *
 */
Character::
Character(const std::string &name) :
  Namable(name),
  _update_delay(0.0),
  _active_owner(nullptr)
{
}

/**
 * Copies the contents of the other Character's preload table into this one.
 */
void Character::
merge_anim_preloads(const Character *other) {
  if (other->_anim_preload == nullptr ||
      _anim_preload == other->_anim_preload) {
    // No-op.
    return;
  }

  if (_anim_preload == nullptr) {
    // Trivial case.
    _anim_preload = other->_anim_preload;
    return;
  }

  // Copy-on-write.
  PT(AnimPreloadTable) anim_preload = _anim_preload.get_write_pointer();
  anim_preload->add_anims_from(other->_anim_preload.get_read_pointer());
}


/**
 * Creates a new CharacterJoint with the indicated name and parent
 * joint, and returns the index of the new joint.
 */
int Character::
make_joint(const std::string &name, int parent, const LMatrix4 &default_value) {
  int index = (int)_joints.size();

  CharacterJoint joint(name);
  joint._parent = parent;
  joint._index = index;
  joint._default_value = default_value;
  // Break out the components as well.
  LVecBase3 hpr, shear;
  decompose_matrix(default_value, joint._default_scale, shear, hpr, joint._default_pos);
  joint._default_quat.set_hpr(hpr);

  if (parent != -1) {
    _joints[parent]._children.push_back(joint._index);
  }

  _joints.push_back(joint);
  _joint_values.push_back(default_value);
  _joint_net_transforms.push_back(LMatrix4::ident_mat());
  _joint_skinning_matrices.push_back(LMatrix4::ident_mat());
  _joint_vertex_transforms.push_back(nullptr);
  _changed_joints.set_bit(index);

  recompute_joint_net_transform(index);
  _joint_initial_net_transform_inverse.push_back(invert(_joint_net_transforms[index]));

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
  slider._value = default_value;
  size_t slider_index = _sliders.size();
  _sliders.push_back(slider);
  return slider_index;
}

/**
 * Binds the animation to the bundle, if possible, and returns a new
 * AnimControl that can be used to start and stop the animation.  If the anim
 * hierarchy does not match the part hierarchy, returns NULL.
 *
 * If hierarchy_match_flags is 0, only an exact match is accepted; otherwise,
 * it may contain a union of PartGroup::HierarchyMatchFlags values indicating
 * conditions that will be tolerated (but warnings will still be issued).
 *
 * If subset is specified, it restricts the binding only to the named subtree
 * of joints.
 *
 * The AnimControl is not stored within the PartBundle; it is the user's
 * responsibility to maintain the pointer.  The animation will automatically
 * unbind itself when the AnimControl destructs (i.e.  its reference count
 * goes to zero).
 */
int Character::
bind_anim(AnimBundle *anim) {
  // Check to see if the animation has already been bound.
  Animations::const_iterator it = std::find(_animations.begin(), _animations.end(), anim);
  if (it != _animations.end()) {
    // Already bound.  Return the index of the existing animation.
    return (int)(it - _animations.begin());
  }

  int index = (int)_animations.size();
  _animations.push_back(anim);

  if (anim->has_mapped_character()) {
    // The animation has already been mapped to a Character.  We can assume
    // that Character has the same joint hierarchy as us.
    return index;
  }

  // We need to map our joints and sliders to joints and sliders on the
  // animation.
  anim->init_joint_mapping(get_num_joints(), get_num_sliders());
  for (int joint = 0; joint < get_num_joints(); joint++) {
    const std::string &joint_name = get_joint_name(joint);
    int anim_joint = anim->find_joint_channel(joint_name);
    if (anim_joint == -1) {
      // This character joint doesn't appear in the animation.  We can deal
      // with it, but give a warning about it, because this might be a mistake.
      anim_cat.warning()
        << "Character joint " << joint_name << " does not appear in animation " << anim->get_name() << "\n";
    }
    anim->map_character_joint_to_anim_joint(joint, anim_joint);
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
    anim->map_character_slider_to_anim_slider(slider, anim_slider);
  }

  return index;
}

/**
 * Binds an animation to the bundle.  The animation is loaded from the disk
 * via the indicated Loader object.  In other respects, this behaves similarly
 * to bind_anim(), with the addition of asynchronous support.
 *
 * If allow_aysnc is true, the load will be asynchronous if possible.  This
 * requires that the animation basename can be found in the PartBundle's
 * preload table (see get_anim_preload()).
 *
 * In an asynchronous load, the animation file will be loaded and bound in a
 * sub-thread.  This means that the animation will not necessarily be
 * available at the time this method returns.  You may still use the returned
 * AnimControl immediately, though, but no visible effect will occur until the
 * animation eventually becomes available.
 *
 * You can test AnimControl::is_pending() to see if the animation has been
 * loaded yet, or wait for it to finish with AnimControl::wait_pending() or
 * even PartBundle::wait_pending().  You can also set an event to be triggered
 * when the animation finishes loading with
 * AnimControl::set_pending_done_event().
 */
int Character::
load_bind_anim(Loader *loader, const Filename &filename) {
  nassertr(loader != nullptr, -1);

  LoaderOptions anim_options(LoaderOptions::LF_search |
                             LoaderOptions::LF_report_errors |
                             LoaderOptions::LF_convert_anim);
  std::string basename = filename.get_basename_wo_extension();

  //int anim_index = -1;
  //CPT(AnimPreloadTable) anim_preload = _anim_preload.get_read_pointer();
  //if (anim_preload != nullptr) {
  //  anim_index = anim_preload->find_anim(basename);
  //}

  if (true) {//anim_index < 0 || !allow_async || !Thread::is_threading_supported()) {
    // The animation is not present in the table, or allow_async is false.
    // Therefore, perform an ordinary synchronous load-and-bind.

    PT(PandaNode) model = loader->load_sync(filename, anim_options);
    if (model == nullptr) {
      // Couldn't load the file.
      return -1;
    }
    AnimBundle *anim = AnimBundleNode::find_anim_bundle(model);
    if (anim == nullptr) {
      // No anim bundle.
      return -1;
    }
    return bind_anim(anim);
  }

  // The animation is present in the table, so we can perform an asynchronous
  // load-and-bind.
  //PN_stdfloat frame_rate = anim_preload->get_base_frame_rate(anim_index);
  //int num_frames = anim_preload->get_num_frames(anim_index);
  //PT(AnimControl) control =
  //  new AnimControl(basename, this, frame_rate, num_frames);

  //if (!subset.is_include_empty()) {
    // Figure out the actual subset of joints to be bound.
  //  BitArray bound_joints;
  //  find_bound_joints(0, false, bound_joints, subset);
  //  control->set_bound_joints(bound_joints);
  //}

  //PT(BindAnimRequest) request =
  //  new BindAnimRequest(std::string("bind:") + filename.get_basename(),
  //                      filename, anim_options, loader, control,
  //                      hierarchy_match_flags, subset);
  //request->set_priority(async_bind_priority);
  //loader->load_async(request);

  return -1;
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

  if (cdata->_anim_graph == nullptr) {
    return false;
  }

  bool any_changed = false;

  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now > cdata->_last_update + _update_delay || cdata->_anim_changed) {

    BitArray joint_mask;
    for (size_t i = 0; i < _joints.size(); i++) {
      if (_joints[i]._merge_joint != -1 || _joints[i]._has_forced_value) {
        // Don't need to animate this joint.
        joint_mask.clear_bit(i);

      } else {
        joint_mask.set_bit(i);
      }
    }

    AnimGraphEvalContext ctx(this, _joints.data(), (int)_joints.size(), cdata->_frame_blend_flag, joint_mask);
    // Apply the bind poses of each joint as the starting point.
    for (int i = 0; i < ctx._num_joints; i++) {
      if (!joint_mask.get_bit(i)) {
        continue;
      }
      ctx._joints[i]._position = _joints[i]._default_pos;
      ctx._joints[i]._rotation = _joints[i]._default_quat;
      ctx._joints[i]._scale = _joints[i]._default_scale;
    }
    cdata->_anim_graph->evaluate(ctx);

    any_changed = apply_pose(cdata, cdata->_root_xform, ctx, current_thread);

    cdata->_anim_changed = false;
    cdata->_last_update = now;
  }

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

  if (cdata->_anim_graph == nullptr) {
    return false;
  }

  double now = ClockObject::get_global_clock()->get_frame_time();

  BitArray joint_mask;
  for (size_t i = 0; i < _joints.size(); i++) {
    if (_joints[i]._merge_joint != -1 || _joints[i]._has_forced_value) {
      // Don't need to animate this joint.
      joint_mask.clear_bit(i);

    } else {
      joint_mask.set_bit(i);
    }
  }

  AnimGraphEvalContext ctx(this, _joints.data(), (int)_joints.size(), cdata->_frame_blend_flag, joint_mask);
  // Apply the bind poses of each joint as the starting point.
  for (int i = 0; i < ctx._num_joints; i++) {
    if (!joint_mask.get_bit(i)) {
      continue;
    }
    ctx._joints[i]._position = _joints[i]._default_pos;
    ctx._joints[i]._rotation = _joints[i]._default_quat;
    ctx._joints[i]._scale = _joints[i]._default_scale;
  }
  cdata->_anim_graph->evaluate(ctx);

  bool any_changed = apply_pose(cdata, cdata->_root_xform, ctx, current_thread);

  cdata->_anim_changed = false;
  cdata->_last_update = now;

  return any_changed;
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
  CharacterJoint &joint = _joints[i];

  if (joint._parent != -1) {
    _joint_net_transforms[i] = _joint_values[i] * _joint_net_transforms[joint._parent];

  } else {
    _joint_net_transforms[i] = _joint_values[i] * get_root_xform();
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
  } else {
    _changed_joints.set_bit(parent);
  }
  attach._parents[parent] = inf;

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
  auto it = attach._parents.find(parent);
  if (it != attach._parents.end()) {
    attach._parents.erase(it);
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
  float weight_total = 0.0f;
  for (auto it = attach._parents.begin(); it != attach._parents.end(); ++it) {
    int parent = (*it).first;
    CharacterAttachment::ParentInfluence &inf = (*it).second;
    if (parent != -1) {
      if (!_changed_joints.get_bit(parent)) {
        continue;
      }
      inf._transform = _joint_net_transforms[parent] * inf._offset * inf._weight;
    }
    transform += inf._transform;
    weight_total += inf._weight;
  }
  if (weight_total != 0.0f) {
    transform /= weight_total;
  } else {
    transform = LMatrix4::ident_mat();
  }
  attach._curr_transform = TransformState::make_mat(transform);
  attach._curr_transform = TransformState::make_pos_hpr_scale_shear(
    attach._curr_transform->get_pos(),
    attach._curr_transform->get_hpr(), 1, 0);
  if (attach._node != nullptr) {
    attach._node->set_transform(attach._curr_transform);
  }
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

  for (size_t i = 0; i < _joints.size(); i++) {
    copy->_joints.push_back(_joints[i]);
    copy->_joint_values.push_back(_joint_values[i]);
    copy->_joint_net_transforms.push_back(_joint_net_transforms[i]);
    copy->_joint_skinning_matrices.push_back(_joint_skinning_matrices[i]);
    copy->_joint_initial_net_transform_inverse.push_back(_joint_initial_net_transform_inverse[i]);
    copy->_joint_vertex_transforms.push_back(nullptr);

    // We don't copy the sets of transform nodes.
  }

  for (size_t i = 0; i < _sliders.size(); i++) {
    copy->_sliders.push_back(_sliders[i]);
  }

  copy->_animations = _animations;
  copy->_sequences = _sequences;
  copy->_pose_parameters = _pose_parameters;
  copy->_attachments = _attachments;

  return copy;
}

/**
 * Applies the final pose computed by the animation graph to each joint.
 */
bool Character::
apply_pose(CData *cdata, const LMatrix4 &root_xform, const AnimGraphEvalContext &context, Thread *current_thread) {
  PStatTimer timer(apply_pose_collector);

  size_t joint_count = _joints.size();

  Character *merge_char = cdata->_joint_merge_character;

  ap_compose_collector.start();
  for (size_t i = 0; i < joint_count; i++) {
    CharacterJoint &joint = _joints[i];

    // Check for a forced joint override value.
    if (joint._has_forced_value) {
      _joint_values[i] = joint._forced_value;

    } else if (joint._merge_joint != -1) {
      // Use the transform of the parent merge joint.

      // Take the net transform and re-interpret it.
      const LMatrix4 &parent_net = merge_char->_joint_net_transforms[joint._merge_joint];
      //if (parent_net != _joint_net_transforms[i]) {
        _changed_joints.set_bit(i);
      //}
      _joint_net_transforms[i] = parent_net;
      if (joint._parent != -1) {
        LMatrix4 parent_inverse = _joint_net_transforms[joint._parent];
        parent_inverse.invert_in_place();
        _joint_values[i] = _joint_net_transforms[i] * parent_inverse;

      } else {
        _joint_values[i] = _joint_net_transforms[i];
      }

    } else {
      const JointTransform &xform = context._joints[i];
      _joint_values[i] = LMatrix4::scale_mat(xform._scale) * xform._rotation;
      _joint_values[i].set_row(3, xform._position);
    }
  }
  ap_compose_collector.stop();

  ap_net_collector.start();
  for (size_t i = 0; i < joint_count; i++) {
    CharacterJoint &joint = _joints[i];

    // If it's a merged joint, we already computed the net transform above.
    if (joint._merge_joint == -1) {
      LMatrix4 old_net = _joint_net_transforms[i];

      if (joint._parent != -1) {
        _joint_net_transforms[i] = _joint_values[i] * _joint_net_transforms[joint._parent];

      } else {
        _joint_net_transforms[i] = _joint_values[i] * root_xform;
      }

      //if (_joint_net_transforms[i] != old_net) {
        _changed_joints.set_bit(i);
      //}
    }

    _joint_skinning_matrices[i] = _joint_initial_net_transform_inverse[i] * _joint_net_transforms[i];
  }
  ap_net_collector.stop();

  ap_mark_jvt_collector.start();
  for (size_t i = 0; i < joint_count; i++) {
    JointVertexTransform *trans = _joint_vertex_transforms[i];
    if (trans != nullptr) {
      trans->mark_modified(current_thread);
    }
  }
  ap_mark_jvt_collector.stop();

  ap_update_net_transform_nodes.start();
  // Compute attachment transforms from the updated character pose.
  for (size_t i = 0; i < _attachments.size(); i++) {
    compute_attachment_transform(i);
  }
  ap_update_net_transform_nodes.stop();

  // Also update slider values... this is temporary.
  for (size_t i = 0; i < _sliders.size(); i++) {
    _sliders[i].update(current_thread);
  }

  _changed_joints.clear();

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
  nassertv(ni != _nodes.end());
  _nodes.erase(ni);

  if (get_num_nodes() > 0) {
    update_active_owner(_active_owner, get_node(get_num_nodes() - 1));
  }
}

/**
 * Builds the mapping of parent joints with joint merge enabled to the
 * corresponding joints on this character.
 */
void Character::
build_joint_merge_map(Character *merge_char) {
  if (merge_char == nullptr) {
    for (size_t i = 0; i < _joints.size(); i++) {
      _joints[i]._merge_joint = -1;
    }

  } else {
    for (size_t i = 0; i < _joints.size(); i++) {
      CharacterJoint &joint = _joints[i];

      // See if the parent character has a joint with this name.
      int parent_joint_idx = merge_char->find_joint(joint.get_name());
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
    _joint_values[i].write_datagram(me);
    _joint_net_transforms[i].write_datagram(me);
    _joint_skinning_matrices[i].write_datagram(me);
    _joint_initial_net_transform_inverse[i].write_datagram(me);
  }

  me.add_int16((int)_sliders.size());
  for (int i = 0; i < (int)_sliders.size(); i++) {
    _sliders[i].write_datagram(me);
  }

  me.add_uint8(_pose_parameters.size());
  for (size_t i = 0; i < _pose_parameters.size(); i++) {
    _pose_parameters[i].write_datagram(manager, me);
  }

  me.add_uint16(_animations.size());
  for (size_t i = 0; i < _animations.size(); i++) {
    manager->write_pointer(me, _animations[i]);
  }

  me.add_uint16(_sequences.size());
  for (size_t i = 0; i < _sequences.size(); i++) {
    manager->write_pointer(me, _sequences[i]);
  }

  me.add_uint8(_attachments.size());
  for (size_t i = 0; i < _attachments.size(); i++) {
    _attachments[i].write_datagram(manager, me);
  }

  manager->write_pointer(me, _anim_preload.get_read_pointer());

  manager->write_cdata(me, _cycler);
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int Character::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  for (size_t i = 0; i < _animations.size(); i++) {
    _animations[i] = DCAST(AnimBundle, p_list[pi++]);
  }

  for (size_t i = 0; i < _sequences.size(); i++) {
    _sequences[i] = DCAST(AnimSequence, p_list[pi++]);
  }

  for (size_t i = 0; i < _attachments.size(); i++) {
    pi = _attachments[i].complete_pointers(pi, p_list, manager);
  }

  _anim_preload = DCAST(AnimPreloadTable, p_list[pi++]);

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
  _joint_values.resize(_joints.size());
  _joint_net_transforms.resize(_joints.size());
  _joint_skinning_matrices.resize(_joints.size());
  _joint_initial_net_transform_inverse.resize(_joints.size());
  _joint_vertex_transforms.resize(_joints.size());
  for (size_t i = 0; i < _joints.size(); i++) {
    _joints[i].read_datagram(scan);
    _joint_values[i].read_datagram(scan);
    _joint_net_transforms[i].read_datagram(scan);
    _joint_skinning_matrices[i].read_datagram(scan);
    _joint_initial_net_transform_inverse[i].read_datagram(scan);

    _joint_vertex_transforms[i] = nullptr;
  }

  _sliders.resize(scan.get_int16());
  for (size_t i = 0; i < _sliders.size(); i++) {
    _sliders[i].read_datagram(scan);
  }

  _pose_parameters.resize(scan.get_uint8());
  for (size_t i = 0; i < _pose_parameters.size(); i++) {
    _pose_parameters[i].fillin(scan, manager);
  }

  _animations.resize(scan.get_uint16());
  manager->read_pointers(scan, _animations.size());

  _sequences.resize(scan.get_uint16());
  manager->read_pointers(scan, _sequences.size());

  _attachments.resize(scan.get_uint8());
  for (size_t i = 0; i < _attachments.size(); i++) {
    _attachments[i].fillin(scan, manager);
  }

  manager->read_pointer(scan);

  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
Character::CData::
CData() :
  _frame_blend_flag(interpolate_frames),
  _anim_changed(false),
  _last_update(0.0),
  _root_xform(LMatrix4::ident_mat()),
  _anim_graph(nullptr),
  _joint_merge_character(nullptr)
{
}

/**
 *
 */
Character::CData::
CData(const Character::CData &copy) :
  _frame_blend_flag(copy._frame_blend_flag),
  _root_xform(copy._root_xform),
  _anim_changed(copy._anim_changed),
  _last_update(copy._last_update),
  _anim_graph(copy._anim_graph),
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
 * Returns a suitable sequence to use for the indicated activity number.
 * If multiple sequences are part of the same activity, the sequence is chosen
 * at random based on assigned weight.  An explicit seed may be given for the
 * random number generator, in case the selected sequence needs to be
 * consistent, for instance during client-side prediction.
 */
int Character::
get_sequence_for_activity(int activity, int curr_sequence, unsigned long seed) const {
  if (get_num_sequences() == 0) {
    return -1;
  }

  Randomizer random(seed);

  int weight_total = 0;
  int seq_idx = -1;
  for (int i = 0; i < get_num_sequences(); i++) {
    AnimSequence *sequence = get_sequence(i);
    int curr_activity = sequence->get_activity();
    int weight = sequence->get_activity_weight();
    if (curr_activity == activity) {
      if (curr_sequence == i && weight < 0) {
        // If this is the current sequence and the weight is < 0, stick with
        // this sequence.
        seq_idx = i;
        break;
      }

      weight_total += std::abs(weight);

      int random_value = random.random_int(weight_total);
      if (!weight_total || random_value < std::abs(weight)) {
        seq_idx = i;
      }
    }
  }

  return seq_idx;
}

/**
 *
 */
void Character::
set_joint_vertex_transform(JointVertexTransform *transform, int n) {
  _joint_vertex_transforms[n] = transform;
}
