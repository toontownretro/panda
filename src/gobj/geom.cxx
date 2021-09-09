/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.cxx
 * @author brian
 * @date 2021-09-05
 */

#include "geom.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "boundingBox.h"
#include "boundingSphere.h"
#include "config_mathutil.h"

UpdateSeq Geom::_next_modified;
TypeHandle Geom::_type_handle;

/**
 *
 */
Geom::
Geom(GeomPrimitiveType type, const GeomVertexData *vertex_data, const GeomIndexData *index_data) :
  _vertex_data((GeomVertexData *)vertex_data),
  _index_data((GeomIndexData *)index_data),
  _primitive_type(type),
  _internal_bounds_stale(true),
  _num_vertices_per_patch(0),
  _nested_vertices(0),
  _bounds_type(BoundingVolume::BT_default)
{
  compute_index_range();
}

/**
 * Constructor for an empty Geom.
 */
Geom::
Geom() :
  _primitive_type(GPT_triangles),
  _vertex_data(nullptr),
  _index_data(nullptr),
  _first_index(0),
  _num_indices(0),
  _num_vertices_per_patch(0),
  _nested_vertices(0),
  _internal_bounds_stale(true),
  _bounds_type(BoundingVolume::BT_default)
{
}

/**
 *
 */
Geom::
~Geom() {
}

/**
 * Recomputes the index range of the Geom.  If the Geom does not have an index
 * buffer, the range is the number of rows in the vertex buffer.  Otherwise,
 * the range is the number of rows in the index buffer.  This can be later
 * overridden by the user to specify a subset of vertices or indices that the
 * Geom should render.
 */
void Geom::
compute_index_range() {
  _first_index = 0;
  _num_indices = 0;

  if (_index_data == nullptr) {
    // No index buffer, use number of rows in vertex buffer.
    if (_vertex_data != nullptr) {
      _num_indices = _vertex_data.get_read_pointer()->get_num_rows();
    }
  } else {
    // Use the number of rows in the index buffer.
    _num_indices = _index_data.get_read_pointer()->get_num_rows();
  }

  mark_internal_bounds_stale();
}

/**
 *
 */
Geom *Geom::
make_copy() const {
  return new Geom(*this);
}

/**
 *
 */
UpdateSeq Geom::
get_next_modified() {
  ++_next_modified;
  return _next_modified;
}

/**
 *
 */
int Geom::
get_primitive_start(int n) const {
  return n * get_num_vertices_per_primitive();
}

/**
 *
 */
int Geom::
get_primitive_end(int n) const {
  int verts_per_prim = get_num_vertices_per_primitive();
  return n * verts_per_prim + verts_per_prim;
}

/**
 *
 */
int Geom::
get_primitive_num_vertices(int n) const {
  return get_num_vertices_per_primitive();
}

/**
 *
 */
int Geom::
get_num_used_vertices() const {
  int num_primitives = get_num_primitives();
  if (num_primitives > 0) {
    return get_num_vertices();

  } else {
    return 0;
  }
}

/**
 * Returns a copy of this Geom with the index ordering reversed.
 *
 * This only means something to triangle Geoms.  Other primitive types
 * just return a copy of the same exact Geom.
 */
PT(Geom) Geom::
reverse() const {
  PT(Geom) copy = make_copy();
  copy->reverse_in_place();
  return copy;
}

/**
 * Reverses the winding order of the Geom's primitives, as an in-place operation.
 *
 * This implicitly converts the Geom to an indexed Geom if it is not one already.
 */
void Geom::
reverse_in_place() {
  if (_primitive_type == GPT_triangles ||
      _primitive_type == GPT_triangles_adj) {

    make_indexed();
    set_index_data(get_index_data()->reverse());
  }
}

/**
 * Returns a copy of this Geom with the reversed set of indices appended
 * to the index buffer, to double-side the Geom.
 *
 * This only means something to triangle Geoms.  Other primitive types
 * just return a copy of the same exact Geom.
 */
PT(Geom) Geom::
doubleside() const {
  PT(Geom) copy = make_copy();
  copy->doubleside_in_place();
  return copy;
}

/**
 *
 */
void Geom::
doubleside_in_place() {
  if (_primitive_type == GPT_triangles ||
      _primitive_type == GPT_triangles_adj) {

    make_indexed();
    set_index_data(get_index_data()->doubleside());
  }
}

/**
 * If the Geom does not have an index buffer, creates one using the Geom's
 * non-indexed vertex range, and assigns it to the Geom.
 */
void Geom::
make_indexed() {
  if (is_indexed()) {
    return;
  }

  // We must have some vertex range.
  nassertv(_num_indices != -1);

  // Determine the index type from the maximum vertex index.
  int max_index = _first_index + _num_indices - 1;

  NumericType index_type;
  if (max_index <= 0xff) {
    index_type = NT_uint8;
  } else if (max_index <= 0xffff) {
    index_type = NT_uint16;
  } else {
    index_type = NT_uint32;
  }

  _index_data = new GeomIndexData(GeomEnums::UH_static, index_type);
  PT(GeomIndexData) indices = modify_index_data();
  indices->add_consecutive_vertices(_first_index, _num_indices);
}

/**
 * Replaces a Geom's vertex table with a new table, and simultaneously adds
 * the indicated offset to all vertex references within the Geom's primitives.
 * This is intended to be used to combine multiple GeomVertexDatas from
 * different Geoms into a single big buffer, with each Geom referencing a
 * subset of the vertices in the buffer.
 */
void Geom::
offset_vertices(const GeomVertexData *data, int offset) {
  _vertex_data = (GeomVertexData *)data;

  if (is_indexed()) {
    // Offset all indices in the index buffer.
    PT(GeomIndexData) index_data = modify_index_data();
    index_data->offset_vertices(offset);

#ifndef NDEBUG
    // Sanity check.
    index_data->check_minmax();
    nassertv((index_data->get_min_vertex() < data->get_num_rows()) &&
             (index_data->get_max_vertex() < data->get_num_rows()));
#endif

  } else {
    // Just simply push the non-indexed vertex range.
    _first_index += offset;
    _num_indices += offset;

    // Sanity check.
    nassertv((_first_index + _num_indices - 1) < data->get_num_rows());
  }
}

/**
 * Turns on all bits corresponding to vertex indices that are referenced by
 * the Geom (or its index buffer).
 */
void Geom::
get_referenced_vertices(BitArray &bits) const {
  if (is_indexed()) {
    // Go through the whole index buffer.
    get_index_data()->get_referenced_vertices(bits);

  } else {
    // Non-indexed case.
    bits.set_range(_first_index, _num_indices);
  }
}

/**
 * Clears the Geom.  This removes the vertex buffer and index buffer
 * references.
 */
void Geom::
clear() {
  _vertex_data = nullptr;
  _index_data = nullptr;
  _first_index = 0;
  _num_indices = 0;
}

/**
 * Returns a GeomVertexData that represents the results of computing the
 * vertex animation on the CPU for this Geom's vertex data.
 *
 * If there is no CPU-defined vertex animation on this object, this just
 * returns the original object.
 *
 * If there is vertex animation, but the VertexTransform values have not
 * changed since last time, this may return the same pointer it returned
 * previously.  Even if the VertexTransform values have changed, it may still
 * return the same pointer, but with its contents modified (this is preferred,
 * since it allows the graphics backend to update vertex buffers optimally).
 *
 * If force is false, this method may return immediately with stale data, if
 * the vertex data is not completely resident.  If force is true, this method
 * will never return stale data, but may block until the data is available.
 */
CPT(GeomVertexData) Geom::
get_animated_vertex_data(bool force, Thread *current_thread) const {
  const GeomVertexData *vdata = get_vertex_data();

  if (vdata == nullptr) {
    return nullptr;
  }

  return vdata->animate_vertices(force, current_thread);
}

/**
 *
 */
void Geom::
write_datagram(BamWriter *manager, Datagram &me) {
  CopyOnWriteObject::write_datagram(manager, me);

  manager->write_pointer(me, (GeomVertexData *)_vertex_data.get_read_pointer());
  manager->write_pointer(me, (GeomIndexData *)_index_data.get_read_pointer());
  me.add_uint8(_primitive_type);
  me.add_uint32(_first_index);
  me.add_uint32(_num_indices);
  me.add_int32(_num_vertices_per_patch);
}

/**
 *
 */
void Geom::
fillin(DatagramIterator &scan, BamReader *manager) {
  CopyOnWriteObject::fillin(scan, manager);

  manager->read_pointer(scan); // vertex data
  manager->read_pointer(scan); // index data
  _primitive_type = (GeomPrimitiveType)scan.get_uint8();
  _first_index = scan.get_uint32();
  _num_indices = scan.get_uint32();
  _num_vertices_per_patch = scan.get_int32();
}

/**
 *
 */
int Geom::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CopyOnWriteObject::complete_pointers(p_list, manager);

  _vertex_data = DCAST(GeomVertexData, p_list[pi++]);
  _index_data = DCAST(GeomIndexData, p_list[pi++]);

  return pi;
}

/**
 *
 */
void Geom::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *Geom::
make_from_bam(const FactoryParams &params) {
  Geom *geom = new Geom;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  geom->fillin(scan, manager);
  return geom;
}

/**
 * Returns the bounding volume for the Geom.
 */
CPT(BoundingVolume) Geom::
get_bounds() const {
  if (_user_bounds != nullptr) {
    // Use the explicitly set bounds on the Geom.
    return _user_bounds;
  }

  if (_internal_bounds_stale) {
    ((Geom *)this)->compute_internal_bounds();
  }

  return _internal_bounds;
}

/**
 * Recomputes the dynamic bounding volume for this Geom.  This includes all of
 * the vertices.
 */
void Geom::
compute_internal_bounds() {
  int num_vertices = 0;

  // Get the vertex data, after animation.
  CPT(GeomVertexData) vertex_data = get_animated_vertex_data(true, Thread::get_current_thread());

  // Now actually compute the bounding volume.  We do this by using
  // calc_tight_bounds to determine our box first.
  LPoint3 pmin, pmax;
  PN_stdfloat sq_center_dist = 0.0f;
  bool found_any = false;
  do_calc_tight_bounds(pmin, pmax, sq_center_dist, found_any,
                       vertex_data, false, LMatrix4::ident_mat(),
                       InternalName::get_vertex());

  BoundingVolume::BoundsType btype = _bounds_type;
  if (btype == BoundingVolume::BT_default) {
    btype = bounds_type;
  }

  if (found_any) {
    nassertv(!pmin.is_nan());
    nassertv(!pmax.is_nan());

    // Then we put the bounding volume around both of those points.
    PN_stdfloat avg_box_area;
    switch (btype) {
    case BoundingVolume::BT_best:
    case BoundingVolume::BT_fastest:
    case BoundingVolume::BT_default:
      {
        // When considering a box, calculate (roughly) the average area of the
        // sides.  We will use this to determine whether a sphere or box is a
        // better fit.
        PN_stdfloat min_extent = std::min(pmax[0] - pmin[0],
                                 std::min(pmax[1] - pmin[1],
                                          pmax[2] - pmin[2]));
        PN_stdfloat max_extent = std::max(pmax[0] - pmin[0],
                                 std::max(pmax[1] - pmin[1],
                                          pmax[2] - pmin[2]));
        avg_box_area = ((min_extent * min_extent) + (max_extent * max_extent)) / 2;
      }
      // Fall through
    case BoundingVolume::BT_sphere:
      {
        // Determine the best radius for a bounding sphere.
        LPoint3 aabb_center = (pmin + pmax) * 0.5f;
        PN_stdfloat best_sq_radius = (pmax - aabb_center).length_squared();

        if (btype != BoundingVolume::BT_fastest && best_sq_radius > 0.0f &&
            aabb_center.length_squared() / best_sq_radius >= (0.2f * 0.2f)) {
          // Hmm, this is an off-center model.  Maybe we can do a better job
          // by calculating the bounding sphere from the AABB center.

          PN_stdfloat better_sq_radius;
          bool found_any = false;
          do_calc_sphere_radius(aabb_center, better_sq_radius, found_any,
                                vertex_data);

          if (found_any && better_sq_radius > 0.0f &&
              better_sq_radius <= best_sq_radius) {
            // Great.  This is as good a sphere as we're going to get.
            if (btype == BoundingVolume::BT_best &&
                avg_box_area < better_sq_radius * MathNumbers::pi) {
              // But the box is better, anyway.  Use that instead.
              _internal_bounds = new BoundingBox(pmin, pmax);
              break;
            }
            _internal_bounds =
              new BoundingSphere(aabb_center, csqrt(better_sq_radius));
            break;
          }
        }

        if (btype != BoundingVolume::BT_sphere &&
            avg_box_area < sq_center_dist * MathNumbers::pi) {
          // A box is probably a tighter fit.
          _internal_bounds = new BoundingBox(pmin, pmax);
          break;

        } else if (sq_center_dist >= 0.0f && sq_center_dist <= best_sq_radius) {
          // No, but a sphere centered on the origin is apparently still
          // better than a sphere around the bounding box.
          _internal_bounds =
            new BoundingSphere(LPoint3::origin(), csqrt(sq_center_dist));
          break;

        } else if (btype == BoundingVolume::BT_sphere) {
          // This is the worst sphere we can make, which is why we will only
          // do it when the user specifically requests a sphere.
          _internal_bounds =
            new BoundingSphere(aabb_center,
              (best_sq_radius > 0.0f) ? csqrt(best_sq_radius) : 0.0f);
          break;
        }
      }
      // Fall through.

    case BoundingVolume::BT_box:
      _internal_bounds = new BoundingBox(pmin, pmax);
    }

    num_vertices += get_num_vertices();

  } else {
    // No points; empty bounding volume.
    if (btype == BoundingVolume::BT_sphere) {
      _internal_bounds = new BoundingSphere;
    } else {
      _internal_bounds = new BoundingBox;
    }
  }

  _nested_vertices = num_vertices;
  _internal_bounds_stale = false;
}

/**
 * The private implementation of calc_tight_bounds().
 */
void Geom::
do_calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                     PN_stdfloat &sq_center_dist, bool &found_any,
                     const GeomVertexData *vertex_data,
                     bool got_mat, const LMatrix4 &mat,
                     const InternalName *column_name) const {

  GeomVertexReader reader(vertex_data, column_name);
  if (!reader.has_column()) {
    // No vertex data.
    return;
  }

  int i = 0;

  if (!is_indexed()) {
    // Non-indexed case.

    if (_num_indices == 0) {
      return;
    }

    if (got_mat) {
      // FInd the first non-NaN vertex.
      while (!found_any && i < _num_indices) {
        reader.set_row(_first_index + i);
        LPoint3 first_vertex = mat.xform_point_general(reader.get_data3());
        if (!first_vertex.is_nan()) {
          min_point = first_vertex;
          max_point = first_vertex;
          sq_center_dist = first_vertex.length_squared();
          found_any = true;
        }
        ++i;
      }

      for (; i < _num_indices; ++i) {
        reader.set_row_unsafe(_first_index + i);
        nassertv(!reader.is_at_end());

        LPoint3 vertex = mat.xform_point_general(reader.get_data3());

        min_point.set(std::min(min_point[0], vertex[0]),
                      std::min(min_point[1], vertex[1]),
                      std::min(min_point[2], vertex[2]));
        max_point.set(std::max(max_point[0], vertex[0]),
                      std::max(max_point[1], vertex[1]),
                      std::max(max_point[2], vertex[2]));
        sq_center_dist = std::max(sq_center_dist, vertex.length_squared());
      }
    } else {
      // Find the first non-NaN vertex.
      while (!found_any && i < _num_indices) {
        reader.set_row(_first_index + i);
        LPoint3 first_vertex = reader.get_data3();
        if (!first_vertex.is_nan()) {
          min_point = first_vertex;
          max_point = first_vertex;
          sq_center_dist = first_vertex.length_squared();
          found_any = true;
        }
        ++i;
      }

      for (; i < _num_indices; ++i) {
        reader.set_row_unsafe(_first_index + i);
        nassertv(!reader.is_at_end());

        const LVecBase3 &vertex = reader.get_data3();

        min_point.set(std::min(min_point[0], vertex[0]),
                      std::min(min_point[1], vertex[1]),
                      std::min(min_point[2], vertex[2]));
        max_point.set(std::max(max_point[0], vertex[0]),
                      std::max(max_point[1], vertex[1]),
                      std::max(max_point[2], vertex[2]));
        sq_center_dist = std::max(sq_center_dist, vertex.length_squared());
      }
    }
  } else {
    // Indexed case.
    GeomVertexReader index(get_index_data(), 0);
    if (index.is_at_end()) {
      return;
    }

    if (got_mat) {
      // Find the first non-NaN vertex.
      while (!found_any && !index.is_at_end()) {
        int ii = index.get_data1i();
        reader.set_row(ii);
        LPoint3 first_vertex = mat.xform_point_general(reader.get_data3());
        if (!first_vertex.is_nan()) {
          min_point = first_vertex;
          max_point = first_vertex;
          sq_center_dist = first_vertex.length_squared();
          found_any = true;
        }
      }

      while (!index.is_at_end()) {
        int ii = index.get_data1i();
        reader.set_row_unsafe(ii);
        nassertv(!reader.is_at_end());

        LPoint3 vertex = mat.xform_point_general(reader.get_data3());

        min_point.set(std::min(min_point[0], vertex[0]),
                      std::min(min_point[1], vertex[1]),
                      std::min(min_point[2], vertex[2]));
        max_point.set(std::max(max_point[0], vertex[0]),
                      std::max(max_point[1], vertex[1]),
                      std::max(max_point[2], vertex[2]));
        sq_center_dist = std::max(sq_center_dist, vertex.length_squared());
      }
    } else {
      // Find the first non-NaN vertex.
      while (!found_any && !index.is_at_end()) {
        int ii = index.get_data1i();
        reader.set_row(ii);
        LVecBase3 first_vertex = reader.get_data3();
        if (!first_vertex.is_nan()) {
          min_point = first_vertex;
          max_point = first_vertex;
          sq_center_dist = first_vertex.length_squared();
          found_any = true;
        }
      }

      while (!index.is_at_end()) {
        int ii = index.get_data1i();
        reader.set_row_unsafe(ii);
        nassertv(!reader.is_at_end());

        const LVecBase3 &vertex = reader.get_data3();

        min_point.set(std::min(min_point[0], vertex[0]),
                      std::min(min_point[1], vertex[1]),
                      std::min(min_point[2], vertex[2]));
        max_point.set(std::max(max_point[0], vertex[0]),
                      std::max(max_point[1], vertex[1]),
                      std::max(max_point[2], vertex[2]));
        sq_center_dist = std::max(sq_center_dist, vertex.length_squared());
      }
    }
  }
}

/**
 *
 */
void Geom::
do_calc_sphere_radius(const LPoint3 &center, PN_stdfloat &sq_radius,
                      bool &found_any, const GeomVertexData *vertex_data) const {
  GeomVertexReader reader(vertex_data, InternalName::get_vertex());
  if (!reader.has_column()) {
    // No vertex data.
    return;
  }

  if (!found_any) {
    sq_radius = 0.0;
  }

  if (!is_indexed()) {
    // Non-indexed case.
    if (_num_indices == 0) {
      return;
    }
    found_any = true;

    for (int i = 0; i < _num_indices; ++i) {
      reader.set_row_unsafe(_first_index + i);
      const LVecBase3 &vertex = reader.get_data3();

      sq_radius = std::max(sq_radius, (vertex - center).length_squared());
    }

  } else {
    // Indexed case.
    GeomVertexReader index(_index_data.get_read_pointer(), 0);
    if (index.is_at_end()) {
      return;
    }
    found_any = true;

    while (!index.is_at_end()) {
      int ii = index.get_data1i();
      reader.set_row_unsafe(ii);
      const LVecBase3 &vertex = reader.get_data3();

      sq_radius = std::max(sq_radius, (vertex - center).length_squared());
    }
  }
}

/**
 *
 */
PT(CopyOnWriteObject) Geom::
make_cow_copy() {
  return new Geom(*this);
}

/**
 * Returns the number of vertices rendered by all primitives within the Geom.
 */
int Geom::
get_nested_vertices() const {
  if (_internal_bounds_stale) {
    ((Geom *)this)->compute_internal_bounds();
  }
  return _nested_vertices;
}
