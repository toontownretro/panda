/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriangleMesh.h
 * @author brian
 * @date 2021-07-09
 */

#ifndef PHYSTRIANGLEMESH_H
#define PHYSTRIANGLEMESH_H

#include "pandabase.h"
#include "physGeometry.h"
#include "physTriangleMeshData.h"
#include "physx_utils.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysTriangleMesh : public PhysGeometry {
PUBLISHED:
  PhysTriangleMesh(PhysTriangleMeshData *mesh_data);
  ~PhysTriangleMesh() = default;

  INLINE void set_scale(const LVecBase3 &scale);
  INLINE void set_scale(PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz);
  INLINE LVecBase3 get_scale() const;

  INLINE bool is_valid() const;

public:
  virtual physx::PxGeometry *get_geometry() override;

private:
  physx::PxTriangleMeshGeometry _geom;
};

#include "physTriangleMesh.I"

#endif // PHYSTRIANGLEMESH_H
