/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.I
 * @author lachbr
 * @date 2021-02-22
 */

/**
 * Adds a new pose parameter to the character.  This can be used by animation
 * blend nodes to control the blend result.  Returns the index of the new pose
 * parameter.
 */
INLINE int Character::
add_pose_parameter(const std::string &name, PN_stdfloat min_val,
                   PN_stdfloat max_val, PN_stdfloat looping) {
  PoseParameter param(name, min_val, max_val, looping);
  _pose_parameters.push_back(std::move(param));
  return (int)_pose_parameters.size() - 1;
}

/**
 * Returns the number of pose parameters stored on the character.
 */
INLINE int Character::
get_num_pose_parameters() const {
  return (int)_pose_parameters.size();
}

/**
 * Returns the pose parameter at the indicated index.
 */
INLINE PoseParameter &Character::
get_pose_parameter(int n) {
  return _pose_parameters[n];
}

/**
 * Returns the index of the pose parameter with the indicated name, or -1 if no
 * such pose parameter exists.
 */
INLINE int Character::
find_pose_parameter(const std::string &name) const {
  for (int i = 0; i < (int)_pose_parameters.size(); i++) {
    if (_pose_parameters[i].get_name() == name) {
      return i;
    }
  }

  return -1;
}

/**
 * Specifies whether the character interpolates (blends) between two
 * sequential frames of an active animation, showing a smooth intra-frame
 * motion, or whether it holds each frame until the next frame is ready,
 * showing precisely the specified animation.
 *
 * When this value is false, the character holds each frame until the next is
 * ready.  When this is true, the character will interpolate between two
 * consecutive frames of animation for each frame the animation is onscreen,
 * according to the amount of time elapsed between the frames.
 *
 * The default value of this flag is determined by the interpolate-frames
 * Config.prc variable.
 *
 * Use set_blend_type() to change the algorithm that the character uses to
 * interpolate matrix positions.
 */
INLINE void Character::
set_frame_blend_flag(bool frame_blend_flag) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_frame_blend_flag = frame_blend_flag;
}

/**
 * Returns whether the character interpolates (blends) between two sequential
 * animation frames, or whether it holds the current frame until the next one
 * is ready.  See set_frame_blend_flag().
 */
INLINE bool Character::
get_frame_blend_flag() const {
  CDReader cdata(_cycler);
  return cdata->_frame_blend_flag;
}

/**
 * Specifies the transform matrix which is implicitly applied at the root of
 * the animated hierarchy.
 */
INLINE void Character::
set_root_xform(const LMatrix4 &root_xform) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_root_xform = root_xform;
  cdata->_anim_changed = true;
}

/**
 * Returns the transform matrix which is implicitly applied at the root of the
 * animated hierarchy.
 */
INLINE const LMatrix4 &Character::
get_root_xform() const {
  CDReader cdata(_cycler);
  return cdata->_root_xform;
}

/**
 * Returns the number of PartBundleNodes that contain a pointer to this
 * PartBundle.
 */
INLINE int Character::
get_num_nodes() const {
  return _nodes.size();
}

/**
 * Returns the nth PartBundleNode associated with this PartBundle.
 */
INLINE CharacterNode *Character::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), nullptr);
  return _nodes[n];
}

/**
 * Applies a forced override value to the specified joint.  All animation to
 * this joint will be overridden by this value.  This is used for procedurally
 * animating joints.  The value is assumed to be in the joint's local
 * coordinate space, relative to the joint's parent.
 */
INLINE void Character::
set_joint_forced_value(int n, const LMatrix4 &value) {
  nassertv(n >= 0 && n < (int)_joints.size());

  _joints[n]._forced_value = value;
  _joints[n]._has_forced_value = true;
  const CharacterJoint &joint = _joints[n];
  LMatrix4 old_net = _joint_net_transforms[n];
  if (joint._parent != -1) {
    _joint_net_transforms[n] = value * _joint_net_transforms[joint._parent];

  } else {
    _joint_net_transforms[n] = value * get_root_xform();
  }

  if (!_changed_joints.get_bit(n) && _joint_net_transforms[n] != old_net) {
    _changed_joints.set_bit(n);
  }
}

/**
 * Clears the forced override value on the specified joint.  From this point
 * on, the joint's animation will be driven by the animation tables.
 */
INLINE void Character::
clear_joint_forced_value(int n) {
  nassertv(n >= 0 && n < (int)_joints.size());

  _joints[n]._has_forced_value = false;
  _changed_joints.set_bit(n);
}

/**
 * Returns the number of joints in the Character.
 */
INLINE int Character::
get_num_joints() const {
  return (int)_joints.size();
}

/**
 * Returns the index of the joint with the indicated name, or -1 if no such
 * joint exists.
 */
INLINE int Character::
find_joint(const std::string &name) const {
  for (int i = 0; i < (int)_joints.size(); i++) {
    if (_joints[i].get_name() == name) {
      return i;
    }
  }

  return -1;
}

/**
 * Returns the name of the indicated joint.
 */
INLINE const std::string &Character::
get_joint_name(int n) const {
  return _joints[n].get_name();
}

/**
 * Returns the number of sliders in the Character.
 */
INLINE int Character::
get_num_sliders() const {
  return (int)_sliders.size();
}

/**
 * Sets the value of the indicated slider.
 */
INLINE void Character::
set_slider_value(int n, PN_stdfloat value) {
  nassertv(n >= 0 && n < (int)_sliders.size());

  _sliders[n].set_value(value);
}

/**
 * Returns the index of the slider with the indicated name, or -1 if no such
 * slider exists.
 */
INLINE int Character::
find_slider(const std::string &name) const {
  for (int i = 0; i < (int)_sliders.size(); i++) {
    if (_sliders[i].get_name() == name) {
      return i;
    }
  }

  return -1;
}

/**
 * Returns the current value of the indicated slider.
 */
INLINE PN_stdfloat Character::
get_slider_value(int n) const {
  return _sliders[n]._value;
}

/**
 * Returns the name of the indicated slider.
 */
INLINE const std::string &Character::
get_slider_name(int n) const {
  return _sliders[n].get_name();
}

/**
 * Sets the CharacterVertexSlider associated with the indicated slider.
 */
INLINE void Character::
set_vertex_slider(int n, CharacterVertexSlider *slider) {
  _sliders[n]._vertex_slider = slider;
}

/**
 * Sets the joint merge character; the character that will be used to merge
 * our joints with theirs.
 */
INLINE void Character::
set_joint_merge_character(Character *character) {
  build_joint_merge_map(character);

  CDWriter cdata(_cycler);
  cdata->_joint_merge_character = character;
}

/**
 * Returns the joint merge character.  See set_joint_merge_character().
 */
INLINE Character *Character::
get_joint_merge_character() const {
  CDReader cdata(_cycler);
  return cdata->_joint_merge_character;
}

/**
 * Sets the default pose of the specified joint.
 */
INLINE void Character::
set_joint_default_value(int n, const LMatrix4 &value) {
  _joint_values[n] = value;
  _joints[n]._default_value = value;
  // Break out the components as well.
  LVecBase3 hpr, shear;
  decompose_matrix(value, _joints[n]._default_scale, shear, hpr, _joints[n]._default_pos);
  _joints[n]._default_quat.set_hpr(hpr);
}

/**
 * Enable joint merge on the indicated joint.
 */
INLINE void Character::
set_joint_merge(int n, bool enable) {
  _joints[n]._merge = enable;
}

/**
 * Returns true if the joint has joint merge enabled, false otherwise.
 */
INLINE bool Character::
get_joint_merge(int n) const {
  return _joints[n]._merge;
}

/**
 * Returns the index of the parent of the specified joint.
 */
INLINE int Character::
get_joint_parent(int n) const {
  return _joints[n]._parent;
}

/**
 * Returns the number of joints that are children of the specified joint.
 */
INLINE int Character::
get_joint_num_children(int n) const {
  return _joints[n]._children.size();
}

/**
 * Returns the nth children of the specified joint.
 */
INLINE int Character::
get_joint_child(int joint, int child) const {
  return _joints[joint]._children[child];
}

/**
 * Returns the current skinning matrix of the specified joint.
 */
INLINE const LMatrix4 &Character::
get_joint_skinning_matrix(int n) const {
  return _joint_skinning_matrices[n];
}

/**
 * Returns the net transform of the indicated joint.
 */
INLINE const LMatrix4 &Character::
get_joint_net_transform(int n) const {
  return _joint_net_transforms[n];
}

/**
 * Returns the local transform of the indicated joint.
 */
INLINE const LMatrix4 &Character::
get_joint_transform(int n) const {
  return _joint_values[n];
}

/**
 * Returns the matrix needed to transform coordinates from character space to
 * the local space of this joint, from the resting pose.
 */
INLINE const LMatrix4 &Character::
get_joint_initial_net_transform_inverse(int n) const {
  return _joint_initial_net_transform_inverse[n];
}

/**
 * Returns resting pose of the specified joint.
 */
INLINE const LMatrix4 &Character::
get_joint_default_value(int n) const {
  return _joints[n]._default_value;
}

/**
 * Returns the current local-space pose of the specified joint.
 */
INLINE const LMatrix4 &Character::
get_joint_value(int n) const {
  return _joint_values[n];
}

/**
 * Adds a new AnimChannel to the character and returns the index of the
 * channel.
 */
INLINE int Character::
add_channel(AnimChannel *channel) {
  auto it = std::find(_channels.begin(), _channels.end(), channel);
  if (it != _channels.end()) {
    // Already added to the character.
    return (int)(it - _channels.begin());
  }

  _channels.push_back(channel);
  return (int)_channels.size() - 1;
}

/**
 * Returns the number of channels in the character.
 */
INLINE int Character::
get_num_channels() const {
  return (int)_channels.size();
}

/**
 * Returns the channel at the indicated index.
 */
INLINE AnimChannel *Character::
get_channel(int n) const {
  nassertr(n >= 0 && n < (int)_channels.size(), nullptr);
  return _channels[n];
}

/**
 * Returns the number of IK chains present in the character.
 */
INLINE int Character::
get_num_ik_chains() const {
  return (int)_ik_chains.size();
}

/**
 * Returns the nth IK chain of the character.  Use the return value for
 * temporary access to the chain.  The returned pointer may be invalidated
 * if new IK chains are added to the character or the character is deleted.
 */
INLINE IKChain *Character::
get_ik_chain(int n) {
  nassertr(n >= 0 && n < (int)_ik_chains.size(), nullptr);
  return &_ik_chains[n];
}

/**
 * Returns the number of animation layers on the character.
 */
INLINE int Character::
get_num_anim_layers() const {
  return (int)_anim_layers.size();
}

/**
 * Returns the nth animation layer on the character.
 */
INLINE AnimLayer *Character::
get_anim_layer(int n) {
  nassertr(n >= 0 && n < (int)_anim_layers.size(), nullptr);
  return &_anim_layers[n];
}

/**
 * Specifies the minimum amount of time, in seconds, that should elapse
 * between any two consecutive updates.  This is normally used by
 * Character::set_lod_animation(), and should not be called directly.
 */
INLINE void Character::
set_update_delay(double delay) {
  _update_delay = delay;
}

/**
 * Specifies whether the Character should automatically advance the timing of
 * all playing animations when it is visited during the Cull traversal.  If
 * this is set to false, the user is responsible for explicitly calling
 * advance() on the Character to update animation timing prior to rendering, or
 * manually supply timing values to each animation layer.  When doing
 * server-side replicated animation, this flag is false and the client will
 * manually set the timing values of each layer with interpolated values
 * received from the server.
 */
INLINE void Character::
set_auto_advance_flag(bool flag) {
  CDWriter cdata(_cycler);
  cdata->_auto_advance_flag = flag;
}

/**
 * Returns whether the Character should automatically advance the timing of
 * all playing animations when it is visited during the Cull traversal.  See
 * set_auto_advance_flag().
 */
INLINE bool Character::
get_auto_advance_flag() const {
  CDReader cdata(_cycler);
  return cdata->_auto_advance_flag;
}

/**
 * Specifies whether the Character should smoothly transition between changes
 * in animations within an animation layer.
 */
INLINE void Character::
set_channel_transition_flag(bool flag) {
  CDWriter cdata(_cycler);
  cdata->_channel_transition_flag = flag;
}

/**
 * Returns whether the Character should smoothly transition between changes in
 * animations within an animation layer.
 */
INLINE bool Character::
get_channel_transition_flag() const {
  CDReader cdata(_cycler);
  return cdata->_channel_transition_flag;
}
