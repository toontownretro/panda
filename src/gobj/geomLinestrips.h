/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLinestrips.h
 * @author drose
 * @date 2005-03-22
 */

#ifndef GEOMLINESTRIPS_H
#define GEOMLINESTRIPS_H

#include "pandabase.h"
#include "geomPrimitive.h"

/**
 * Defines a series of line strips.
 */
class EXPCL_PANDA_GOBJ GeomLinestrips : public GeomPrimitive {
PUBLISHED:
  explicit GeomLinestrips(UsageHint usage_hint);
  GeomLinestrips(const GeomLinestrips &copy);
  virtual ~GeomLinestrips();
  ALLOC_DELETED_CHAIN(GeomLinestrips);

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;
  virtual int get_geom_rendering() const;

  CPT(GeomPrimitive) make_adjacency() const;

  virtual int get_min_num_vertices_per_primitive() const;
  virtual int get_num_unused_vertices_per_primitive() const;

protected:
  virtual CPT(GeomPrimitive) decompose_impl() const;
  virtual CPT(GeomIndexArrayData) rotate_impl() const;
  virtual bool requires_unused_vertices() const;
  virtual void append_unused_vertices(GeomIndexArrayData *vertices,
                                      int vertex);

public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomPrimitive::init_type();
    register_type(_type_handle, "GeomLinestrips",
                  GeomPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Geom;
};

#endif
