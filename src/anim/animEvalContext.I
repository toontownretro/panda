/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEvalContext.I
 * @author brian
 * @date 2021-08-04
 */

/**
 *
 */
INLINE AnimEvalData::
AnimEvalData() :
  _position{ LPoint3(0) },
  _scale{ LVecBase3(1) },
  _rotation{ LQuaternion::ident_quat() },
  _weight(1.0f),
  _cycle(0.0f)
{
}

/**
 *
 */
INLINE AnimEvalData::
AnimEvalData(const AnimEvalData &copy) {
  std::copy(copy._position, copy._position + max_character_joints, _position);
  std::copy(copy._scale, copy._scale + max_character_joints, _scale);
  std::copy(copy._rotation, copy._rotation + max_character_joints, _rotation);
  _weight = copy._weight;
  _cycle = copy._cycle;
}

/**
 *
 */
INLINE AnimEvalData::
AnimEvalData(AnimEvalData &&other) {
  std::move(other._position, other._position + max_character_joints, _position);
  std::move(other._scale, other._scale + max_character_joints, _scale);
  std::move(other._rotation, other._rotation + max_character_joints, _rotation);
  _weight = std::move(other._weight);
  _cycle = std::move(other._cycle);
}

/**
 * Copies the joint poses from the indicated AnimEvalData into this one.
 */
INLINE void AnimEvalData::
copy_joints(const AnimEvalData &copy) {
  std::copy(copy._position, copy._position + max_character_joints, _position);
  std::copy(copy._scale, copy._scale + max_character_joints, _scale);
  std::copy(copy._rotation, copy._rotation + max_character_joints, _rotation);
}

/**
 * Moves the joint poses from the indicated AnimEvalData into this one.  Do not
 * try to access the joint poses on the other AnimEvalData after calling this.
 */
INLINE void AnimEvalData::
steal_joints(AnimEvalData &other) {
  std::move(other._position, other._position + max_character_joints, _position);
  std::move(other._scale, other._scale + max_character_joints, _scale);
  std::move(other._rotation, other._rotation + max_character_joints, _rotation);
}
