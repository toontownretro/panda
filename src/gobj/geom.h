/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.h
 * @author brian
 * @date 2021-09-05
 */

#ifndef GEOM_H
#define GEOM_H

#include "pandabase.h"
#include "geomIndexData.h"
#include "geomVertexData.h"
#include "pointerTo.h"
#include "geomEnums.h"
#include "updateSeq.h"
#include "bitArray.h"
#include "boundingVolume.h"
#include "copyOnWriteObject.h"
#include "pta_int.h"

/**
 * A Geom is the smallest atomic piece of renderable geometry that can be sent
 * to the graphics card in one call.  It is simply a vertex buffer and index
 * buffer pairing.  Each Geom has an associated primitive type that is used
 * to interpret the index buffer when rendering the Geom.  Examples of
 * primitive types are triangles, lines, or points.
 */
class EXPCL_PANDA_GOBJ Geom : public CopyOnWriteObject, public GeomEnums {
PUBLISHED:
  Geom(GeomPrimitiveType type, const GeomVertexData *vertex_data, UsageHint usage_hint);
  Geom();

  virtual ~Geom() override;

  void compute_index_range();

  virtual Geom *make_copy() const;

  Geom(const Geom &copy);
  Geom(Geom &&other);
  void operator = (const Geom &copy);
  void operator = (Geom &&other);

  INLINE void set_buffers(GeomVertexData *vertex_data, GeomIndexData *index_data);

  INLINE void set_vertex_data(const GeomVertexData *data);
  INLINE const GeomVertexData *get_vertex_data() const;
  INLINE PT(GeomVertexData) modify_vertex_data();

  INLINE void set_index_data(const GeomIndexData *data);
  INLINE const GeomIndexData *get_index_data() const;
  INLINE bool is_indexed() const;
  INLINE PT(GeomIndexData) modify_index_data();

  INLINE void set_primitive_type(GeomPrimitiveType type);
  INLINE GeomPrimitiveType get_primitive_type() const;

  INLINE PrimitiveType get_primitive_family() const;

  INLINE int get_num_vertices_per_primitive() const;

  INLINE void set_index_range(unsigned int first_index, unsigned int num_indices);
  INLINE unsigned int get_first_index() const;
  INLINE unsigned int get_num_indices() const;

  INLINE bool is_empty() const;

  INLINE bool is_composite() const;

  INLINE int get_first_vertex() const;
  INLINE int get_num_vertices() const;

  INLINE int get_min_vertex() const;
  INLINE int get_max_vertex() const;

  INLINE int get_num_primitives() const;
  int get_primitive_start(int n) const;
  int get_primitive_end(int n) const;
  int get_primitive_num_vertices(int n) const;
  int get_num_used_vertices() const;

  INLINE int get_num_faces() const;
  INLINE int get_primitive_num_faces(int n) const;

  void make_indexed();

  PT(Geom) reverse() const;
  void reverse_in_place();

  PT(Geom) doubleside() const;
  void doubleside_in_place();

  void offset_vertices(const GeomVertexData *data, int offset);

  void get_referenced_vertices(BitArray &bits) const;

  void clear();

  bool copy_primitives_from(CPT(Geom) other, int max_indices, bool preserve_order);

  CPT(GeomVertexData) get_animated_vertex_data(bool force, Thread *current_thread) const;

  int get_nested_vertices() const;
  CPT(BoundingVolume) get_bounds() const;

  INLINE void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                                bool &found_any,
                                const GeomVertexData *vertex_data,
                                bool got_mat, const LMatrix4 &mat) const;
  INLINE void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                                bool &found_any) const;
  INLINE void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                                bool &found_any,
                                const GeomVertexData *vertex_data,
                                bool got_mat, const LMatrix4 &mat,
                                const InternalName *column_name) const;

  INLINE void set_bounds_type(BoundingVolume::BoundsType type);
  INLINE BoundingVolume::BoundsType get_bounds_type() const;
  INLINE void set_bounds(const BoundingVolume *volume);
  INLINE void clear_bounds();
  INLINE void mark_bounds_stale() const;
  INLINE void mark_internal_bounds_stale();

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  void compute_internal_bounds();
  void do_calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                            PN_stdfloat &sq_center_dist, bool &found_any,
                            const GeomVertexData *vertex_data,
                            bool got_mat, const LMatrix4 &mat,
                            const InternalName *column_name) const;
  void do_calc_sphere_radius(const LPoint3 &center,
                             PN_stdfloat &sq_radius, bool &found_any,
                             const GeomVertexData *vertex_data) const;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  void fillin(DatagramIterator &scan, BamReader *manager);

  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static UpdateSeq get_next_modified();

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy() override;

private:
  // The geometric primitive type of the Geom.  Can be triangles,
  // lines, points, or patches.  No strips or triangle fans.
  GeomPrimitiveType _primitive_type;

  // Pointer to the vertex buffer the Geom should render with.
  COWPT(GeomVertexData) _vertex_data;

  // Pointer to the index buffer the Geom should render with.
  // If this is NULL, the Geom is non-indexed, and the _first_index
  // and _num_indices variables define the range of consecutive
  // vertices from the vertex buffer to render.
  COWPT(GeomVertexArrayData) _vertices;

  // Numeric type of the index buffer indices.
  // Can be uin8, uint16, or uint32.
  NumericType _index_type;

  /////////////////////////////////////////////////////////////////////////////
  // For an indexed Geom, this is a range of consective indices
  // into the *index* buffer that should be drawn for the Geom.
  //
  // For a non-indexed Geom, this is the range of consecutive indices
  // into the *vertex* buffer.
  //
  unsigned int _first_vertex;
  unsigned int _num_vertices;
  /////////////////////////////////////////////////////////////////////////////

  // Specific to the patch primitive type.
  int _num_vertices_per_patch;

  // Min/max vertex index.
  bool _got_minmax;
  unsigned int _min_vertex;
  unsigned int _max_vertex;
  COWPT(GeomVertexArrayData) _mins;
  COWPT(GeomVertexArrayData) _maxs;

  PTA_int _ends;

  // To note if the index buffer needs to be reuploaded.
  UpdateSeq _modified;

  // Usage hint to give to the index buffer.
  UsageHint _usage_hint;

  // Bounds information.
  CPT(BoundingVolume) _user_bounds;
  bool _internal_bounds_stale;
  CPT(BoundingVolume) _internal_bounds;
  BoundingVolume::BoundsType _bounds_type;
  int _nested_vertices;

public:
  static UpdateSeq _next_modified;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "Geom",
                  CopyOnWriteObject::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const Geom &obj);

#include "geom.I"

#endif // GEOM_H
