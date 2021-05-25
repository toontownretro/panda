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
 * Returns the AnimPreloadTable associated with the Character.  This table,
 * if present, can be used for the benefit of load_bind_anim() to allow
 * asynchronous binding.
 */
INLINE CPT(AnimPreloadTable) Character::
get_anim_preload() const {
  return _anim_preload.get_read_pointer();
}

/**
 * Returns a modifiable pointer to the AnimPreloadTable associated with the
 * Character, if any.
 */
INLINE PT(AnimPreloadTable) Character::
modify_anim_preload() {
  return _anim_preload.get_write_pointer();
}

/**
 * Replaces the AnimPreloadTable associated with the Character.
 */
INLINE void Character::
set_anim_preload(AnimPreloadTable *anim_preload) {
  _anim_preload = anim_preload;
}

/**
 * Removes any AnimPreloadTable associated with the Character.
 */
INLINE void Character::
clear_anim_preload() {
  _anim_preload = nullptr;
}

/**
 * Adds a new pose parameter to the character.  This can be used by animation
 * blend nodes to control the blend result.  Returns the index of the new pose
 * parameter.
 */
INLINE int Character::
add_pose_parameter(const std::string &name, PN_stdfloat min_val, PN_stdfloat max_val, bool looping) {
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
 * Sets the animation graph that the character should evaluate to determine
 * a final pose for a frame of animation.
 */
INLINE void Character::
set_anim_graph(AnimGraphNode *graph) {
  CDWriter cdata(_cycler);
  if (cdata->_anim_graph != graph) {
    cdata->_anim_graph = graph;
    cdata->_anim_changed = true;
  }
}

/**
 * Returns the animation graph that the character should evaluate to determine
 * a final pose for a frame of animation.
 */
INLINE AnimGraphNode *Character::
get_anim_graph() const {
  CDReader cdata(_cycler);
  return cdata->_anim_graph;
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
  _joints[n]._forced_value = value;
  _joints[n]._has_forced_value = true;
  const CharacterJoint &joint = _joints[n];
  if (joint._parent != -1) {
    _joint_net_transforms[n] = value * _joint_net_transforms[joint._parent];

  } else {
    _joint_net_transforms[n] = value * get_root_xform();
  }
}

/**
 * Clears the forced override value on the specified joint.  From this point
 * on, the joint's animation will be driven by the animation tables.
 */
INLINE void Character::
clear_joint_forced_value(int n) {
  _joints[n]._has_forced_value = false;
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
 * Adds a new AnimSequence to the character and returns the index of the
 * sequence.
 */
INLINE int Character::
add_sequence(AnimSequence *sequence) {
  Sequences::const_iterator it = std::find(_sequences.begin(), _sequences.end(), sequence);
  if (it != _sequences.end()) {
    // Already added to the character.
    return (int)(it - _sequences.begin());
  }

  _sequences.push_back(sequence);
  return (int)_sequences.size() - 1;
}

/**
 * Returns the number of sequences in the character.
 */
INLINE int Character::
get_num_sequences() const {
  return (int)_sequences.size();
}

/**
 * Returns the sequence at the indicated index.
 */
INLINE AnimSequence *Character::
get_sequence(int n) const {
  nassertr(n >= 0 && n < (int)_sequences.size(), nullptr);
  return _sequences[n];
}

/**
 * Returns the number of animations in the character.  This is the number of
 * raw frame-by-frame animations that were created and exported from the
 * modelling package.  A sequence may be composed of multiple animations.
 */
INLINE int Character::
get_num_anims() const {
  return (int)_animations.size();
}

/**
 * Returns the AnimBundle at the indicated index.
 */
INLINE AnimBundle *Character::
get_anim(int n) const {
  nassertr(n >= 0 && n < (int)_animations.size(), nullptr);
  return _animations[n];
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
