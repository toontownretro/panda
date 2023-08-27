/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexArrayData.h
 * @author brian
 * @date 2023-08-16
 */

#ifndef GEOMINDEXARRAYDATA_H
#define GEOMINDEXARRAYDATA_H

#include "pandabase.h"
#include "geomVertexArrayData.h"

class IndexBufferContext;

/**
 * Small specialization on GeomVertexArrayData for index buffers.
 *
 * Stores an IndexBufferContext instead of a VertexBufferContext.
 */
class EXPCL_PANDA_GOBJ GeomIndexArrayData : public GeomVertexArrayData {
private:
  GeomIndexArrayData();

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy() override;

PUBLISHED:
  explicit GeomIndexArrayData(NumericType numeric_type, UsageHint usage_hint);
  explicit GeomIndexArrayData(const GeomVertexArrayFormat *array_format,
                              UsageHint usage_hint);
  GeomIndexArrayData(const GeomIndexArrayData &copy);
  virtual ~GeomIndexArrayData();
  ALLOC_DELETED_CHAIN(GeomIndexArrayData);

  void operator = (const GeomIndexArrayData &copy) = delete;

  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;

  IndexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                                   GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

private:
  IndexBufferContext *_context;

  typedef pflat_hash_map<PreparedGraphicsObjects *, IndexBufferContext *, pointer_hash> Contexts;
  Contexts _contexts;

public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomVertexArrayData::init_type();
    register_type(_type_handle, "GeomIndexArrayData",
                  GeomVertexArrayData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

#include "geomIndexArrayData.I"

#endif // GEOMINDEXARRAYDATA_H
