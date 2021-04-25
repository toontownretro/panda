/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physConvexMesh.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSCONVEXMESH_H
#define PHYSCONVEXMESH_H

#include "pandabase.h"
#include "physGeometry.h"
#include "physConvexMeshData.h"
#include "physx_utils.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysConvexMesh : public PhysGeometry {
PUBLISHED:
  PhysConvexMesh(PhysConvexMeshData *mesh_data);
  ~PhysConvexMesh() = default;

  INLINE void set_scale(const LVecBase3 &scale);
  INLINE void set_scale(PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz);
  INLINE LVecBase3 get_scale() const;

  INLINE bool is_valid() const;

public:
  virtual physx::PxGeometry *get_geometry() override;

private:
  physx::PxConvexMeshGeometry _geom;
};

#include "physConvexMesh.I"

#endif // PHYSCONVEXMESH_H
