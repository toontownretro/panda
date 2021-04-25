/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physConvexMesh.cxx
 * @author brian
 * @date 2021-04-22
 */

#include "physConvexMesh.h"

/**
 *
 */
PhysConvexMesh::
PhysConvexMesh(PhysConvexMeshData *mesh_data) {
  if (!mesh_data->has_mesh()) {
    bool mesh_generated = mesh_data->generate_mesh();
    nassertv(mesh_generated);
  }

  _geom = physx::PxConvexMeshGeometry(mesh_data->get_mesh());
}

/**
 *
 */
physx::PxGeometry *PhysConvexMesh::
get_geometry() {
  return &_geom;
}
