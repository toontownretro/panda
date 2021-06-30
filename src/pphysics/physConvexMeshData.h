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
#include "pta_uchar.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysConvexMeshData : public ReferenceCount {
PUBLISHED:
  INLINE PhysConvexMeshData();
  INLINE PhysConvexMeshData(CPTA_uchar mesh_data);

  INLINE void add_point(const LPoint3 &point);
  INLINE size_t get_num_points() const;
  INLINE LPoint3 get_point(size_t n) const;
  INLINE void clear_points();

  INLINE bool has_mesh() const;
  INLINE bool has_mesh_data() const;

  INLINE void invalidate_mesh();

  bool generate_mesh();
  bool cook_mesh();

  INLINE CPTA_uchar get_mesh_data() const;

  void get_mass_information(PN_stdfloat *mass, LMatrix3 *local_inertia, LPoint3 *center_of_mass) const;

public:
  INLINE physx::PxConvexMesh *get_mesh() const;

private:
  typedef pvector<physx::PxVec3> Points;
  Points _points;

  physx::PxConvexMesh *_mesh;
  CPTA_uchar _mesh_data;
  bool _has_mesh_data;
};

#include "physConvexMeshData.I"

#endif // PHYSCONVEXMESH_H
