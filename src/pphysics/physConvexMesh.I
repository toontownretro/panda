/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physConvexMesh.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE void PhysConvexMesh::
set_scale(const LVecBase3 &scale) {
  _geom.scale.scale = panda_norm_vec_to_physx(scale);
}

/**
 *
 */
INLINE void PhysConvexMesh::
set_scale(PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  _geom.scale.scale = panda_norm_vec_to_physx(LVecBase3(sx, sy, sz));
}

/**
 *
 */
INLINE LVecBase3 PhysConvexMesh::
get_scale() const {
  return physx_norm_vec_to_panda(_geom.scale.scale);
}

/**
 *
 */
INLINE bool PhysConvexMesh::
is_valid() const {
  return _geom.isValid();
}
