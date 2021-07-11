/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriangleMesh.cxx
 * @author brian
 * @date 2021-07-09
 */

#include "physTriangleMesh.h"

/**
 *
 */
PhysTriangleMesh::
PhysTriangleMesh(PhysTriangleMeshData *mesh_data) {
  if (!mesh_data->has_mesh()) {
    bool mesh_generated = mesh_data->generate_mesh();
    nassertv(mesh_generated);
  }

  _geom = physx::PxTriangleMeshGeometry(mesh_data->get_mesh());
}

/**
 *
 */
physx::PxGeometry *PhysTriangleMesh::
get_geometry() {
  return &_geom;
}
