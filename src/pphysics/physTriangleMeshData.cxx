/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriangleMeshData.cxx
 * @author brian
 * @date 2021-07-09
 */

#include "physTriangleMeshData.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"
#include "geomNode.h"
#include "physSystem.h"
#include "streamWrapper.h"
#include "physXStreams.h"
#include "nodePath.h"

/**
 * Adds a triangle to the mesh data.
 */
void PhysTriangleMeshData::
add_triangle(const LPoint3 &v1, const LPoint3 &v2, const LPoint3 &v3, int material_index) {
  size_t start = _vertices.size();
  _vertices.push_back(panda_vec_to_physx(v1));
  _vertices.push_back(panda_vec_to_physx(v2));
  _vertices.push_back(panda_vec_to_physx(v3));
  _indices.push_back(start);
  _indices.push_back(start + 1);
  _indices.push_back(start + 2);
  _mat_indices.push_back(material_index);

  invalidate_mesh();
}

/**
 * Adds a quad to the triangle mesh.  Internally adds two triangles.
 */
void PhysTriangleMeshData::
add_quad(const LPoint3 &v1, const LPoint3 &v2, const LPoint3 &v3, const LPoint3 &v4, int material_index) {
  size_t start = _vertices.size();
  _vertices.push_back(panda_vec_to_physx(v1));
  _vertices.push_back(panda_vec_to_physx(v2));
  _vertices.push_back(panda_vec_to_physx(v3));
  _vertices.push_back(panda_vec_to_physx(v4));
  _indices.push_back(start);
  _indices.push_back(start + 1);
  _indices.push_back(start + 2);
  _indices.push_back(start);
  _indices.push_back(start + 2);
  _indices.push_back(start + 3);

  _mat_indices.push_back(material_index);
  _mat_indices.push_back(material_index);

  invalidate_mesh();
}

/**
 * Adds a polygon with an arbitrary number of vertices to the mesh.  There
 * must be at least 3 vertices.  The polygon is added in a triangle fan
 * formation.
 */
void PhysTriangleMeshData::
add_polygon(const pvector<LPoint3> &v, int material_index) {
  nassertv(v.size() >= 3u);

  size_t start = _vertices.size();
  for (size_t i = 0; i < v.size(); i++) {
    _vertices.push_back(panda_vec_to_physx(v[i]));
  }
  for (size_t i = 1; i < (v.size() - 1); i++) {
    _indices.push_back(start);
    _indices.push_back(start + i);
    _indices.push_back(start + i + 1);
    _mat_indices.push_back(material_index);
  }

  invalidate_mesh();
}

/**
 * Adds triangles into the mesh from the indicated Geom object.
 */
void PhysTriangleMeshData::
add_triangles_from_geom(const Geom *geom, const LMatrix4 &mat, int material_index) {
  PT(Geom) dgeom = geom->decompose();
  GeomVertexReader vreader(dgeom->get_vertex_data(), InternalName::get_vertex());
  for (size_t i = 0; i < dgeom->get_num_primitives(); i++) {
    const GeomPrimitive *prim = dgeom->get_primitive(i);
    for (int j = 0; j < prim->get_num_primitives(); j++) {
      int start = prim->get_primitive_start(j);
      int end = prim->get_primitive_end(j);

      size_t first_index = _vertices.size();
      for (int k = start; k < end; k++) {
        vreader.set_row(prim->get_vertex(k));
        _vertices.push_back(panda_vec_to_physx(mat.xform_point(vreader.get_data3f())));
      }
      _indices.push_back(first_index);
      _indices.push_back(first_index + 1);
      _indices.push_back(first_index + 2);
      _mat_indices.push_back(material_index);
    }
  }
  invalidate_mesh();
}

/**
 * Adds triangles into the mesh from the Geoms of the indicated GeomNode.
 */
void PhysTriangleMeshData::
add_triangles_from_geom_node(GeomNode *node, bool world_space, int material_index) {
  LMatrix4 mat = LMatrix4::ident_mat();
  if (world_space) {
    mat = NodePath(node).get_net_transform()->get_mat();
  }

  for (int i = 0; i < node->get_num_geoms(); i++) {
    add_triangles_from_geom(node->get_geom(i), mat, material_index);
  }
  invalidate_mesh();
}

/**
 *
 */
bool PhysTriangleMeshData::
generate_mesh() {
  invalidate_mesh();

  PhysSystem *sys = PhysSystem::ptr();
  physx::PxPhysics *physics = sys->get_physics();

  if (has_mesh_data()) {
    // Create from mesh data buffer.

    std::istringstream ss(
      std::string((char *)_mesh_data.p(), _mesh_data.size()));
    IStreamWrapper wrapper(&ss, false);
    PhysXInputStream pstream(wrapper);
    _mesh = physics->createTriangleMesh(pstream);
    return (_mesh != nullptr);

  } else {
    // Run-time creation from input triangles.
    physx::PxCooking *cooking = sys->get_cooking();

    physx::PxTriangleMeshDesc desc;
    desc.points.count = _vertices.size();
    desc.points.stride = sizeof(physx::PxVec3);
    desc.points.data = _vertices.data();
    desc.triangles.count = _indices.size() / 3;
    desc.triangles.stride = sizeof(physx::PxU32) * 3;
    desc.triangles.data = _indices.data();

    if (!_mat_indices.empty()) {
      nassertr(_mat_indices.size() == desc.triangles.count, false);
      desc.materialIndices.stride = sizeof(physx::PxMaterialTableIndex);
      desc.materialIndices.data = _mat_indices.data();

    } else {
      desc.materialIndices.data = nullptr;
    }

    physx::PxTriangleMeshCookingResult::Enum result;
    _mesh = cooking->createTriangleMesh(desc, physics->getPhysicsInsertionCallback(), &result);
    return (_mesh != nullptr) && (result == physx::PxTriangleMeshCookingResult::eSUCCESS);
  }
}

/**
 *
 */
bool PhysTriangleMeshData::
cook_mesh() {
  PhysSystem *sys = PhysSystem::ptr();
  physx::PxCooking *cooking = sys->get_cooking();

  physx::PxTriangleMeshDesc desc;
  desc.points.count = _vertices.size();
  desc.points.stride = sizeof(physx::PxVec3);
  desc.points.data = _vertices.data();
  desc.triangles.count = _indices.size() / 3;
  desc.triangles.stride = sizeof(physx::PxU32) * 3;
  desc.triangles.data = _indices.data();

  if (!_mat_indices.empty()) {
    nassertr(_mat_indices.size() == desc.triangles.count, false);
    desc.materialIndices.stride = sizeof(physx::PxMaterialTableIndex);
    desc.materialIndices.data = _mat_indices.data();

  } else {
    desc.materialIndices.data = nullptr;
  }

  physx::PxTriangleMeshCookingResult::Enum result;
  // Create and serialize mesh data to binary stream.
  std::ostringstream out;
  OStreamWrapper wrapper(&out, false);
  PhysXOutputStream pstream(wrapper);
  bool ret = cooking->cookTriangleMesh(desc, pstream, &result);
  std::string data = out.str();
  PTA_uchar mesh_data = PTA_uchar::empty_array(data.size());
  memcpy(mesh_data.p(), data.c_str(), data.size());
  _mesh_data = mesh_data;
  _has_mesh_data = true;
  return ret && (result == physx::PxTriangleMeshCookingResult::eSUCCESS);
}
