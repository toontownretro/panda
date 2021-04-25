/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physConvexMeshData.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSCONVEXMESHDATA_H
#define PHYSCONVEXMESHDATA_H

#include "pandabase.h"
#include "referenceCount.h"
#include "luse.h"
#include "pvector.h"
#include "physx_includes.h"
#include "physx_utils.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysConvexMeshData : public ReferenceCount {
PUBLISHED:
  INLINE PhysConvexMeshData();

  INLINE void add_point(const LPoint3 &point);
  INLINE size_t get_num_points() const;
  INLINE LPoint3 get_point(size_t n) const;
  INLINE void clear_points();

  INLINE bool has_mesh() const;
  INLINE void invalidate_mesh();

  bool generate_mesh();

public:
  INLINE physx::PxConvexMesh *get_mesh() const;

private:
  typedef pvector<physx::PxVec3> Points;
  Points _points;

  physx::PxConvexMesh *_mesh;
};

#include "physConvexMeshData.I"

#endif // PHYSCONVEXMESH_H
