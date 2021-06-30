/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeballNode.I
 * @author lachbr
 * @date 2021-03-24
 */

/**
 * Sets the character and joint that the eyeball is parented to.  The eye
 * offset is interpreted as being relative to the parent joint.
 */
INLINE void EyeballNode::
set_character(Character *character, int parent_joint) {
  _character = character;
  _parent_joint = parent_joint;
}

/**
 * Returns the character that the eyeball is parented to.
 */
INLINE Character *EyeballNode::
get_character() const {
  if (_character.was_deleted()) {
    return nullptr;
  }
  return _character.p();
}

/**
 * Returns the joint that the eyeball is parented to.
 */
INLINE int EyeballNode::
get_parent_joint() const {
  return _parent_joint;
}

/**
 * Sets the offset of the eye relative to the parent joint.
 */
INLINE void EyeballNode::
set_eye_offset(const LPoint3 &offset) {
  _eye_offset = TransformState::make_pos(offset);
}

/**
 * Returns the offset of the eye relative to the parent joint.
 */
INLINE const TransformState *EyeballNode::
get_eye_offset() const {
  return _eye_offset;
}

/**
 *
 */
INLINE void EyeballNode::
set_view_target(NodePath node, const LPoint3 &offset) {
  _view_offset = TransformState::make_pos(offset);
  _view_target = node;
}

/**
 *
 */
INLINE void EyeballNode::
set_eye_shift(const LVector3 &shift) {
  _eye_shift = shift;
}

/**
 *
 */
INLINE void EyeballNode::
set_z_offset(PN_stdfloat offset) {
  _z_offset = offset;
}

/**
 *
 */
INLINE void EyeballNode::
set_radius(PN_stdfloat radius) {
  _radius = radius;
}

/**
 *
 */
INLINE void EyeballNode::
set_iris_scale(PN_stdfloat scale) {
  _iris_scale = scale;
}

/**
 *
 */
INLINE void EyeballNode::
set_eye_size(PN_stdfloat size) {
  _eye_size = size;
}

/**
 *
 */
INLINE void EyeballNode::
set_debug_enabled(bool enable) {
  _debug_enabled = enable;
}
