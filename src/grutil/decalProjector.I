/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decalProjector.I
 * @author brian
 * @date 2022-11-05
 */

/**
 *
 */
INLINE DecalProjector::
DecalProjector() :
  _projector_mins(-1.0f),
  _projector_maxs(1.0f),
  _projector_transform(TransformState::make_identity()),
  _decal_state(RenderState::make_empty()),
  _decal_uv_transform(TransformState::make_identity()),
  _fragments_lock("fragments-lock"),
  _num_vertices(0),
  _num_indices(0),
  _test_bounds(true)
{
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_parent(const NodePath &parent) {
  _projector_parent = parent;
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_transform(const TransformState *transform) {
  _projector_transform = transform;
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_frame(const LPoint2 &ll, const LPoint2 &ul, const LPoint2 &ur, const LPoint2 &lr,
                    PN_stdfloat min_depth, PN_stdfloat max_depth) {
  _projector_frame[0] = LPoint3(ll[0], 0.0f, ll[1]);
  _projector_frame[1] = LPoint3(ul[0], 0.0f, ul[1]);
  _projector_frame[2] = LPoint3(ur[0], 0.0f, ur[1]);
  _projector_frame[3] = LPoint3(lr[0], 0.0f, lr[1]);
  _projector_mins = LPoint3(ll[0], min_depth, ll[1]);
  _projector_maxs = LPoint3(ur[0], max_depth, ur[1]);
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_bounds(const LPoint3 &mins, const LPoint3 &maxs) {
  _projector_frame[0] = LPoint3(mins[0], 0.0f, mins[2]);
  _projector_frame[1] = LPoint3(mins[0], 0.0f, maxs[2]);
  _projector_frame[2] = LPoint3(maxs[0], 0.0f, maxs[2]);
  _projector_frame[3] = LPoint3(maxs[0], 0.0f, mins[2]);
  _projector_mins = mins;
  _projector_maxs = maxs;
}

/**
 *
 */
INLINE void DecalProjector::
set_test_bounds(bool flag) {
  _test_bounds = flag;
}

/**
 *
 */
INLINE void DecalProjector::
set_decal_parent(const NodePath &parent) {
  _decal_parent = parent;
}

/**
 *
 */
INLINE void DecalProjector::
set_decal_uv_transform(const TransformState *transform) {
  _decal_uv_transform = transform;
}

/**
 *
 */
INLINE void DecalProjector::
set_decal_render_state(const RenderState *state) {
  _decal_state = state;
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
ALWAYS_INLINE bool DecalProjector::
project(const Readers &readers, int v1, int v2, int v3, const LMatrix4 *net_mat) {
  DecalWinding tri_winding;

  bool has_mat = (net_mat != nullptr);

  int vertices[3] = { v1, v2, v3 };

  GeomVertexReader &vreader = ((Readers *)&readers)->_vertex;

  vreader.set_row_unsafe(v1);
  LPoint3 p1 = vreader.get_data3f();
  if (has_mat) {
    p1 = net_mat->xform_point(p1);
  }
  tri_winding.add_point(p1);

  vreader.set_row_unsafe(v2);
  LPoint3 p2 = vreader.get_data3f();
  if (has_mat) {
    p2 = net_mat->xform_point(p2);
  }
  tri_winding.add_point(p2);

  vreader.set_row_unsafe(v3);
  LPoint3 p3 = vreader.get_data3f();
  if (has_mat) {
    p3 = net_mat->xform_point(p3);
  }
  tri_winding.add_point(p3);

  LPoint3 tri_mins(1e9);
  LPoint3 tri_maxs(-1e9);
  tri_mins = tri_mins.fmin(tri_winding.get_point(0));
  tri_mins = tri_mins.fmin(tri_winding.get_point(1));
  tri_mins = tri_mins.fmin(tri_winding.get_point(2));
  tri_maxs = tri_maxs.fmax(tri_winding.get_point(0));
  tri_maxs = tri_maxs.fmax(tri_winding.get_point(1));
  tri_maxs = tri_maxs.fmax(tri_winding.get_point(2));

  if (!box_overlap(_projector_world_bbox->get_minq(), _projector_world_bbox->get_maxq(),
                   tri_mins, tri_maxs)) {
    return false;
  }

  LVector3 triangle_normal = -tri_winding.get_plane().get_normal();
  if (triangle_normal.dot(_projector_world_forward) < 0.1f) {
    // If triangle is exactly perpendicular or facing away from decal
    // normal, don't create a fragment.
    return false;
  }

  //BoundingBox tri_bbox(tri_mins, tri_maxs);
  //tri_bbox.local_object();
  //if (!_projector_world_bbox->contains(&tri_bbox)) {
  //  return false;
  //}

  // This triangle will definitely be part of the decal.
  // Clip the triangle to the 6 planes of the decal bounding box.

  // Now clip the triangle to each plane.
  bool valid = true;

  for (int i = 0; i < 6; ++i) {
    tri_winding = tri_winding.chop(_box_planes[i]);
    if (tri_winding.get_num_points() < 3) {
      valid = false;
      break;
    }
  }

  if (!valid) {
    return false;
  }

  DecalFragment frag;
  frag._orig_vertices[0]._pos = p1;
  frag._orig_vertices[1]._pos = p2;
  frag._orig_vertices[2]._pos = p3;

  GeomVertexReader &nreader = ((Readers *)&readers)->_normal;
  //vreader.set_column(InternalName::get_normal());
  if (nreader.has_column()) {
    for (int i = 0; i < 3; ++i) {
      nreader.set_row_unsafe(vertices[i]);
      if (has_mat) {
        frag._orig_vertices[i]._normal = net_mat->xform_vec(nreader.get_data3f());
      } else {
        frag._orig_vertices[i]._normal = nreader.get_data3f();
      }
    }
  } else {
    for (int i = 0; i < 3; ++i) {
      frag._orig_vertices[i]._normal = LVector3::forward();
    }
  }

  GeomVertexReader &lmreader = ((Readers *)&readers)->_texcoord_lightmap;
  if (lmreader.has_column()) {
    for (int i = 0; i < 3; ++i) {
      lmreader.set_row_unsafe(vertices[i]);
      frag._orig_vertices[i]._texcoord_lightmap = lmreader.get_data2f();
    }
  } else {
    for (int i = 0; i < 3; ++i) {
      frag._orig_vertices[i]._texcoord_lightmap.set(0, 0);
    }
  }

#if 0
  GeomVertexReader treader(readers._tangent);
  //vreader.set_column(InternalName::get_tangent());
  if (treader.has_column()) {
    for (int i = 0; i < 3; ++i) {
      treader.set_row_unsafe(vertices[i]);
      if (!ident_transform) {
        frag._orig_vertices[i]._tangent = net_mat.xform_vec(treader.get_data3f());
      } else {
        frag._orig_vertices[i]._tangent = treader.get_data3f();
      }
    }
  } else {
    for (int i = 0; i < 3; ++i) {
      frag._orig_vertices[i]._tangent = LVector3::right();
    }
  }

  GeomVertexReader breader(readers._binormal);
  //vreader.set_column(InternalName::get_binormal());
  if (breader.has_column()) {
    for (int i = 0; i < 3; ++i) {
      breader.set_row_unsafe(vertices[i]);
      if (!ident_transform) {
        frag._orig_vertices[i]._binormal = net_mat.xform_vec(breader.get_data3f());
      } else {
        frag._orig_vertices[i]._binormal = breader.get_data3f();
      }
    }
  } else {
    for (int i = 0; i < 3; ++i) {
      frag._orig_vertices[i]._binormal = LVector3::up();
    }
  }
#endif

  frag._winding = std::move(tri_winding);

  _num_vertices += frag._winding.get_num_points();
  _num_indices += (frag._winding.get_num_points() - 2) * 3;

  //_fragments_lock.acquire();
  _fragments.push_back(std::move(frag));
  //_fragments_lock.release();

  return true;
}

/**
 *
 */
INLINE bool DecalProjector::
box_overlap(const LPoint3 &min_a, const LPoint3 &max_a, const LPoint3 &min_b, const LPoint3 &max_b) const {
  return (max_a[0] >= min_b[0] && max_b[0] >= min_a[0] &&
          max_a[1] >= min_b[1] && max_b[1] >= min_a[1] &&
          max_a[2] >= min_b[2] && max_b[2] >= min_a[2]);
}

/**
 *
 */
INLINE int DecalProjector::
get_prim_vertex(int i, const unsigned char *read_pointer, const GeomPrimitivePipelineReader &reader) const {
  if (read_pointer != nullptr) {
    switch (reader.get_index_type()) {
    case GeomEnums::NT_uint8:
      return ((uint8_t *)read_pointer)[i];
    case GeomEnums::NT_uint16:
      return ((uint16_t *)read_pointer)[i];
    case GeomEnums::NT_uint32:
      return ((uint32_t *)read_pointer)[i];
    default:
      return 0;
    }

  } else {
    return reader.get_first_vertex() + i;
  }
}
