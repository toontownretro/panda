/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decalProjector.cxx
 * @author brian
 * @date 2022-11-05
 */

#include "decalProjector.h"
#include "geomVertexReader.h"
#include "mathutil_misc.h"
#include "luse.h"
#include "plane.h"
#include "look_at.h"
#include "winding.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"

/**
 *
 */
bool DecalProjector::
project(const NodePath &root) {
  setup_coordinate_space();

  LMatrix4 parent_net;
  NodePath parent = root.get_parent();
  if (!parent.is_empty()) {
    parent_net = parent.get_net_transform()->get_mat();
  } else {
    parent_net = LMatrix4::ident_mat();
  }

  return r_project(root.node(), parent_net);
}

/**
 *
 */
bool DecalProjector::
r_project(PandaNode *node, const LMatrix4 &net_transform) {
  bool any = false;

  LMatrix4 this_net_transform = node->get_transform()->get_mat() * net_transform;

  if (node->is_geom_node()) {
    if (project(DCAST(GeomNode, node), this_net_transform)) {
      any = true;
    }
  }

  for (int i = 0; i < node->get_num_children(); ++i) {
    if (r_project(node->get_child(i), this_net_transform)) {
      any = true;
    }
  }
  return any;
}

/**
 *
 */
bool DecalProjector::
project(GeomNode *geom_node, const LMatrix4 &net_transform) {
  bool any = false;
  for (int i = 0; i < geom_node->get_num_geoms(); ++i) {
    if (project(geom_node->get_geom(i), net_transform)) {
      any = true;
    }
  }
  return any;
}

/**
 *
 */
bool DecalProjector::
project(const Geom *geom, const LMatrix4 &net_transform) {
  bool any = false;

  //PT(Geom) dgeom = geom->decompose();

  const GeomVertexData *vdata = geom->get_vertex_data();
  for (int i = 0; i < geom->get_num_primitives(); ++i) {
    const GeomPrimitive *prim = geom->get_primitive(i);
    // We can only put decals on polygons (triangles and tristrips).
    //std::cout << prim->get_type() << "\n";
    if (prim->get_primitive_type() != GeomEnums::PT_polygons) {
      continue;
    }

    TypeHandle prim_type = prim->get_type();

    if (prim_type == GeomTriangles::get_class_type()) {

      for (int j = 0; j < prim->get_num_primitives(); j++) {
        int start = prim->get_primitive_start(j);
        if (project(vdata, prim->get_vertex(start), prim->get_vertex(start + 1), prim->get_vertex(start + 2), net_transform)) {
          any = true;
        }
      }

    } else if (prim_type == GeomTristrips::get_class_type()) {

      // Extract triangles from tristrip indices.

      CPTA_int ends = prim->get_ends();
      int num_vertices = prim->get_num_vertices();
      int num_unused = prim->get_num_unused_vertices_per_primitive();

      int vi = -num_unused;
      int li = 0;
      while (li < (int)ends.size()) {
        vi += num_unused;
        int end = ends[li];
        nassertr(vi + 2 <= end, false);
        int v0 = prim->get_vertex(vi);
        ++vi;
        int v1 = prim->get_vertex(vi);
        ++vi;
        bool reversed = false;
        while (vi < end) {
          int v2 = prim->get_vertex(vi);
          ++vi;
          if (reversed) {
            if (v0 != v1 && v0 != v2 && v1 != v2) {
              if (project(vdata, v0, v2, v1, net_transform)) {
                any = true;
              }
            }

            reversed = false;
          } else {
            if (v0 != v1 && v0 != v2 && v1 != v2) {
              if (project(vdata, v0, v1, v2, net_transform)) {
                any = true;
              }
            }
            reversed = true;
          }
          v0 = v1;
          v1 = v2;
        }
        ++li;
      }

      nassertr(vi == num_vertices, false);
    }
  }

  return any;
}

/**
 * Attempts to project the decal onto the triangle defined by v1, v2, and v3,
 * which are vertex indices into the given GeomVertexData.
 *
 * Returns true if a decal fragment was made, or false otherise.
 *
 * A decal fragment will not be made if the triangle is facing away
 * from the decal normal, or if the triangle does not intersect the
 * decal bounding box.
 */
bool DecalProjector::
project(const GeomVertexData *vdata, int v1, int v2, int v3, const LMatrix4 &net_transform) {
  Winding tri_winding;

  int vertices[3] = { v1, v2, v3 };

  GeomVertexReader vreader(vdata, InternalName::get_vertex());
  vreader.set_row(v1);
  tri_winding.add_point(net_transform.xform_point(vreader.get_data3f()));
  vreader.set_row(v2);
  tri_winding.add_point(net_transform.xform_point(vreader.get_data3f()));
  vreader.set_row(v3);
  tri_winding.add_point(net_transform.xform_point(vreader.get_data3f()));

  LVector3 triangle_normal = -tri_winding.get_plane().get_normal();
  if (triangle_normal.dot(_projector_world_forward) <= 0.0f) {
    // If triangle is exactly perpendicular or facing away from decal
    // normal, don't create a fragment.
    return false;
  }

  LPoint3 tri_mins(1e9);
  LPoint3 tri_maxs(1e9);
  tri_mins = tri_mins.fmin(tri_winding.get_point(0));
  tri_mins = tri_mins.fmin(tri_winding.get_point(1));
  tri_mins = tri_mins.fmin(tri_winding.get_point(2));
  tri_maxs = tri_maxs.fmax(tri_winding.get_point(0));
  tri_maxs = tri_maxs.fmax(tri_winding.get_point(1));
  tri_maxs = tri_maxs.fmax(tri_winding.get_point(2));

  BoundingBox tri_bbox(tri_mins, tri_maxs);
  tri_bbox.local_object();
  if (!_projector_world_bbox->contains(&tri_bbox)) {
    return false;
  }

  // This triangle will definitely be part of the decal.
  // Clip the triangle to the 6 planes of the decal bounding box.

  // Now clip the triangle to each plane.
  bool valid = true;

  Winding fragment_winding(tri_winding);

  for (int i = 0; i < 6; ++i) {
    fragment_winding = fragment_winding.chop(_box_planes[i]);
    if (fragment_winding.get_num_points() < 3) {
      valid = false;
      break;
    }
  }

  if (!valid) {
    return false;
  }

  DecalFragment frag;
  frag._orig_vertices[0]._pos = tri_winding.get_point(0);
  frag._orig_vertices[1]._pos = tri_winding.get_point(1);
  frag._orig_vertices[2]._pos = tri_winding.get_point(2);

  for (int i = 0; i < 3; ++i) {
    int vtx_idx = vertices[i];

    vreader.set_column(InternalName::get_normal());
    if (vreader.has_column()) {
      vreader.set_row(vtx_idx);
      frag._orig_vertices[i]._normal = net_transform.xform_vec(vreader.get_data3f());
    } else {
      frag._orig_vertices[i]._normal = LVector3::forward();
    }

    vreader.set_column(InternalName::get_tangent());
    if (vreader.has_column()) {
      vreader.set_row(vtx_idx);
      frag._orig_vertices[i]._tangent = net_transform.xform_vec(vreader.get_data3f());
    } else {
      frag._orig_vertices[i]._tangent = LVector3::right();
    }

    vreader.set_column(InternalName::get_binormal());
    if (vreader.has_column()) {
      vreader.set_row(vtx_idx);
      frag._orig_vertices[i]._binormal = net_transform.xform_vec(vreader.get_data3f());
    } else {
      frag._orig_vertices[i]._tangent = LVector3::up();
    }
  }

  frag._winding = std::move(fragment_winding);

  _fragments.push_back(std::move(frag));

  return true;
}

/**
 *
 */
LVecBase3 DecalProjector::
calc_barycentric_coordinates(const LPoint3 &a, const LPoint3 &b, const LPoint3 &c, const LPoint3 &point) const {
  LVecBase3 weights(0.0f);
  static constexpr float equal_epsilon = 0.01f;

  if ((a - point).length_squared() < equal_epsilon) {
    // Close enough to be fully in point A.
    weights[0] = 1.0f;
    weights[1] = weights[2] = 0.0f;
    return weights;
  }

  if ((b - point).length_squared() < equal_epsilon) {
    // Close enough to be fully in point B.
    weights[1] = 1.0f;
    weights[0] = weights[2] = 0.0f;
    return weights;
  }

  if ((c - point).length_squared() < equal_epsilon) {
    // Close enough to be fully in point C.
    weights[2] = 1.0f;
    weights[0] = weights[1] = 0.0f;
    return weights;
  }

  // Need to blend between the points.
  LVector3 v0 = b - a;
  LVector3 v1 = c - a;
  LVector3 v2 = point - a;

  PN_stdfloat d00 = v0.dot(v0);
  PN_stdfloat d01 = v0.dot(v1);
  PN_stdfloat d11 = v1.dot(v1);
  PN_stdfloat d20 = v2.dot(v0);
  PN_stdfloat d21 = v2.dot(v1);
  PN_stdfloat denom = (d00 * d11 - d01 * d01);

  if (denom < equal_epsilon) {
    weights[0] = 1.0f;
    weights[1] = weights[2] = 0.0f;
    return weights;
  }

  PN_stdfloat oo_denom = 1.0f / denom;

  PN_stdfloat v = (d11 * d20 - d01 * d21) * oo_denom;
  PN_stdfloat w = (d00 * d21 - d01 * d20) * oo_denom;
  PN_stdfloat u = 1.0f - v - w;

  weights[0] = u;
  weights[1] = v;
  weights[2] = w;
  return weights;
}

/**
 * Generates and returns geometry for the decal, using the fragments
 * previously created during projection.
 */
PT(PandaNode) DecalProjector::
generate() {
  if (_fragments.empty()) {
    return nullptr;
  }

  PT(GeomVertexArrayFormat) arr_fmt_tmp = new GeomVertexArrayFormat;
  arr_fmt_tmp->add_column(InternalName::get_vertex(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_point);
  arr_fmt_tmp->add_column(InternalName::get_texcoord(), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
  arr_fmt_tmp->add_column(InternalName::get_normal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_normal);
  arr_fmt_tmp->add_column(InternalName::get_tangent(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
  arr_fmt_tmp->add_column(InternalName::get_binormal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
  CPT(GeomVertexFormat) vtx_fmt = GeomVertexFormat::register_format(arr_fmt_tmp);

  PT(GeomNode) node = new GeomNode("decal");
  PT(GeomVertexData) vdata = new GeomVertexData("decal", vtx_fmt, GeomEnums::UH_static);
  PT(Geom) geom = new Geom(vdata);
  PT(GeomTriangles) triangles = new GeomTriangles(GeomEnums::UH_static);

  int start_vertex = 0;
  GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
  GeomVertexWriter twriter(vdata, InternalName::get_texcoord());
  GeomVertexWriter nwriter(vdata, InternalName::get_normal());
  GeomVertexWriter biwriter(vdata, InternalName::get_binormal());
  GeomVertexWriter tawriter(vdata, InternalName::get_tangent());

  for (const DecalFragment &frag : _fragments) {
    // Generate a trifan-like thing from the winding.  Each decal fragment is
    // guaranteed to be a planar polygon, because it's generated by clipping a
    // triangle.

    for (int i = 0; i < frag._winding.get_num_points(); ++i) {
      vwriter.add_data3f(_decal_inv_net_mat.xform_point(frag._winding.get_point(i)));

      LVecBase3 bary = calc_barycentric_coordinates(
        frag._orig_vertices[0]._pos, frag._orig_vertices[1]._pos,
        frag._orig_vertices[2]._pos, frag._winding.get_point(i));

      LVector3 normal = frag._orig_vertices[0]._normal * bary[0] +
                        frag._orig_vertices[1]._normal * bary[1] +
                        frag._orig_vertices[2]._normal * bary[2];
      normal.normalize();
      nwriter.add_data3f(_decal_inv_net_mat.xform_vec(normal));

      LVector3 tangent = frag._orig_vertices[0]._tangent * bary[0] +
                         frag._orig_vertices[1]._tangent * bary[1] +
                         frag._orig_vertices[2]._tangent * bary[2];
      tangent.normalize();
      tawriter.add_data3f(_decal_inv_net_mat.xform_vec(tangent));

      LVector3 binormal = frag._orig_vertices[0]._binormal * bary[0] +
                          frag._orig_vertices[1]._binormal * bary[1] +
                          frag._orig_vertices[2]._binormal * bary[2];
      binormal.normalize();
      biwriter.add_data3f(_decal_inv_net_mat.xform_vec(binormal));

      // To calculate texture coordinates, we move the vertex position into
      // projector-space, and use the X and Z distances from the lower-left
      // corner of the projector bounds as texture coordinates, then
      // transform it by the user-specified UV matrix.

      LVecBase2 projector_size = _projector_maxs.get_xz() - _projector_mins.get_xz();

      LPoint2 projector_space_pos = _projector_inv_net_mat.xform_point(frag._winding.get_point(i)).get_xz();
      projector_space_pos += _projector_mins.get_xz();
      LVecBase2 uv = projector_space_pos;
      uv[0] /= -projector_size[0];
      uv[1] /= -projector_size[1];
      uv[1] = 1.0f - uv[1];

      //std::cout << "uv: " << uv << "\n";

      uv = _decal_uv_transform->get_mat().xform(LVecBase4(uv[0], uv[1], 0, 0)).get_xy();

      twriter.add_data2f(uv);
    }

    for (int i = 1; i < (frag._winding.get_num_points() - 1); ++i) {
      triangles->add_vertex(start_vertex);
      triangles->add_vertex(start_vertex + i);
      triangles->add_vertex(start_vertex + i + 1);
      triangles->close_primitive();
    }

    start_vertex += frag._winding.get_num_points();
  }

  geom->add_primitive(triangles);
  node->add_geom(geom, _decal_state);

  return node;
}

/**
 * Clears previously created decal fragments.
 */
void DecalProjector::
clear() {
  _fragments.clear();
}

/**
 *
 */
void DecalProjector::
setup_coordinate_space() {
  CPT(TransformState) projector_net_transform = _projector_parent.get_net_transform()
    ->compose(_projector_transform);
  const LMatrix4 &projector_net_mat = projector_net_transform->get_mat();
  LPoint3 world_mins = projector_net_mat.xform_point(_projector_mins);
  LPoint3 world_maxs = projector_net_mat.xform_point(_projector_maxs);
  LQuaternion quat = projector_net_transform->get_norm_quat();
  LVector3 world_forward = quat.get_forward();
  LVector3 world_up = quat.get_up();
  LVector3 world_right = quat.get_right();

  _projector_world_forward = world_forward;

  // Flip the plane, because we want to keep what is on the back side.
  // Winding::chop() returns what is in front of the plane.
  _box_planes[0] = -LPlane(world_forward, world_maxs);
  _box_planes[1] = -LPlane(-world_forward, world_mins);
  _box_planes[2] = -LPlane(world_right, world_maxs);
  _box_planes[3] = -LPlane(-world_right, world_mins);
  _box_planes[4] = -LPlane(world_up, world_maxs);
  _box_planes[5] = -LPlane(-world_up, world_mins);

  CPT(TransformState) decal_net_transform = _decal_parent.get_net_transform();
  const LMatrix4 *decal_inv_net_transform = decal_net_transform->get_inverse_mat();
  if (decal_inv_net_transform == nullptr) {
    _decal_inv_net_mat = LMatrix4::ident_mat();
  } else {
    _decal_inv_net_mat = *decal_inv_net_transform;
  }

  const LMatrix4 *projector_inv_net_mat = projector_net_transform->get_inverse_mat();
  if (projector_inv_net_mat == nullptr) {
    _projector_inv_net_mat = LMatrix4::ident_mat();
  } else {
    _projector_inv_net_mat = *projector_inv_net_mat;
  }

  _projector_world_center = (world_mins + world_maxs) * 0.5f;
  _projector_world_extents = (world_maxs - world_mins) * 0.5f;

  _projector_world_bbox = new BoundingBox(_projector_mins, _projector_maxs);
  _projector_world_bbox->xform(projector_net_mat);
}
