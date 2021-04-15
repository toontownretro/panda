/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physShape.I
 * @author brian
 * @date 2021-04-14
 */

/**
 *
 */
INLINE void PhysShape::
set_local_transform(const LPoint3 &pos, const LVecBase3 &hpr) {
  nassertv(_shape != nullptr);

  LQuaternion quat;
  quat.set_hpr(hpr);

  physx::PxTransform trans = physx::PxTransform(
    physx::PxVec3(pos[0], pos[1], pos[2]),
    physx::PxQuat(quat[1], quat[2], quat[3], quat[0]));

  _shape->setLocalPose(trans);
}

/**
 * Modifies just the position of the shape.
 */
INLINE void PhysShape::
set_local_pos(const LPoint3 &pos) {
  nassertv(_shape != nullptr);

  physx::PxTransform trans = _shape->getLocalPose();
  trans.p = physx::PxVec3(pos[0], pos[1], pos[2]);
  _shape->setLocalPose(trans);
}

/**
 * Modifies just the orientation of the shape.
 */
INLINE void PhysShape::
set_local_hpr(const LVecBase3 &hpr) {
  nassertv(_shape != nullptr);

  LQuaternion quat;
  quat.set_hpr(hpr);

  physx::PxTransform trans = _shape->getLocalPose();
  trans.q = physx::PxQuat(quat[1], quat[2], quat[3], quat[0]);
  _shape->setLocalPose(trans);
}

/**
 *
 */
INLINE LPoint3 PhysShape::
get_local_pos() const {
  nassertr(_shape != nullptr, LPoint3());

  physx::PxTransform trans = _shape->getLocalPose();
  return LPoint3(trans.p.x, trans.p.y, trans.p.z);
}

/**
 *
 */
INLINE LVecBase3 PhysShape::
get_local_hpr() const {
  nassertr(_shape != nullptr, LVecBase3());

  physx::PxTransform trans = _shape->getLocalPose();
  LQuaternion quat(trans.q.w, trans.q.x, trans.q.y, trans.q.z);
  return quat.get_hpr();
}

/**
 *
 */
INLINE physx::PxShape *PhysShape::
get_shape() const {
  return _shape;
}
