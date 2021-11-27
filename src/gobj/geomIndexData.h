/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexData.h
 * @author brian
 * @date 2021-11-24
 */

#ifndef GEOMINDEXDATA_H
#define GEOMINDEXDATA_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "geomEnums.h"
#include "updateSeq.h"
#include "vertexDataBuffer.h"

class PreparedGraphicsObjects;

/**
 * This is a list of integers that index into a GeomVertexArrayData object.
 * It corresponds directly to an index buffer in the graphics pipe.  It forms
 * the individual primitives that will be rendered using an associated
 * vertex buffer.
 *
 * The indices are stored as a block of bytes so they can be easily transferred
 * to the graphics card.
 */
class EXPCL_PANDA_GOBJ GeomIndexData : public TypedWritableReferenceCount, public GeomEnums {
PUBLISHED:

  virtual ~GeomIndexData() override;

  void add_vertex(unsigned int vertex);
  void insert_vertex(size_t loc, unsigned int vertex);

  void reserve(size_t count);

  INLINE void set_index_type(NumericType index_type);
  INLINE NumericType get_index_type() const;

  INLINE size_t get_index_stride() const;

  INLINE size_t get_num_indices() const;
  INLINE unsigned int get_index(size_t n) const;

  INLINE const unsigned char *get_data() const;

  INLINE size_t get_data_size_bytes() const;

  INLINE void set_usage_hint(UsageHint hint);
  INLINE UsageHint get_usage_hint() const;

  bool is_prepared(PreparedGraphicsObjects *pgo) const;
  void prepare_now(PreparedGraphicsObjects *pgo, GraphicsStateGuardianBase *gsg);

private:
  NumericType _index_type;
  UsageHint _usage_hint;
  UpdateSeq _modified;

  VertexDataBuffer _buffer;
};

#include "geomIndexData.I"

#endif // GEOMINDEXDATA_H
