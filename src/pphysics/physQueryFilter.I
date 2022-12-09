/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physQueryFilter.I
 * @author brian
 * @date 2021-05-22
 */


/**
 *
 */
INLINE PhysRigidActorNode *PhysQueryFilterCallbackData::
get_actor() const {
  return _actor;
}

/**
 *
 */
INLINE PhysShape *PhysQueryFilterCallbackData::
get_shape() const {
  return _shape;
}

/**
 *
 */
INLINE unsigned int PhysQueryFilterCallbackData::
get_shape_contents_mask() const {
  return _shape_contents_mask;
}

/**
 *
 */
INLINE unsigned int PhysQueryFilterCallbackData::
get_shape_collision_group() const {
  return _shape_collision_group;
}

/**
 *
 */
INLINE unsigned int PhysQueryFilterCallbackData::
get_solid_mask() const {
  return _solid_mask;
}

/**
 *
 */
INLINE unsigned int PhysQueryFilterCallbackData::
get_collision_group() const {
  return _collision_group;
}

/**
 *
 */
INLINE void PhysQueryFilterCallbackData::
set_result(int flag) {
  _result = flag;
}

/**
 *
 */
INLINE int PhysQueryFilterCallbackData::
get_result() const {
  return _result;
}
