// Filename: qpgeomVertexData.h
// Created by:  drose (06Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef qpGEOMVERTEXDATA_H
#define qpGEOMVERTEXDATA_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "qpgeomVertexFormat.h"
#include "qpgeomVertexDataType.h"
#include "internalName.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "pStatCollector.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pvector.h"
#include "pta_uchar.h"

class FactoryParams;
class qpGeomVertexDataType;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexData
// Description : This defines the actual numeric vertex data stored in
//               a Geom, in the structure defined by a particular
//               GeomVertexFormat object.
//
//               The data consists of one or more arrays of floats.
//               Typically, there will be only one array per Geom, and
//               the various data types defined in the
//               GeomVertexFormat will be interleaved throughout that
//               array.  However, it is possible to have multiple
//               different arrays, with different combinations of data
//               types through each one.
//
//               However the data is distributed, the effect is of a
//               table of vertices, with a value for each of the
//               GeomVertexFormat's data types, stored for each
//               vertex.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexData : public TypedWritableReferenceCount {
private:
  qpGeomVertexData();
PUBLISHED:
  qpGeomVertexData(const qpGeomVertexFormat *format);
  qpGeomVertexData(const qpGeomVertexData &copy);
  void operator = (const qpGeomVertexData &copy);
  virtual ~qpGeomVertexData();

  INLINE const qpGeomVertexFormat *get_format() const;

  int get_num_vertices() const;
  INLINE void set_num_vertices(int n);
  void clear_vertices();

  INLINE int get_num_arrays() const;
  INLINE CPTA_uchar get_array_data(int array) const;
  PTA_uchar modify_array_data(int array);
  void set_array_data(int array, PTA_uchar array_data);

  int get_num_bytes() const;

  CPT(qpGeomVertexData) convert_to(const qpGeomVertexFormat *new_format) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

  void clear_cache();

public:
  void set_data(int array, const qpGeomVertexDataType *data_type,
                int vertex, const float *data, int num_values);
  void get_data(int array, const qpGeomVertexDataType *data_type,
                int vertex, float *data, int num_values) const;

  bool get_array_info(const InternalName *name, CPTA_uchar &array_data,
                      int &num_components,
                      qpGeomVertexDataType::NumericType &numeric_type, 
                      int &start, int &stride) const;

  static void to_vec2(LVecBase2f &vec, const float *data, int num_values);
  static void to_vec3(LVecBase3f &vec, const float *data, int num_values);
  static void to_vec4(LVecBase4f &vec, const float *data, int num_values);

  static unsigned int pack_argb(const float data[4]);
  static void unpack_argb(float data[4], unsigned int packed_argb);

private:
  void remove_cache_entry(const qpGeomVertexFormat *modifier) const;

private:
  CPT(qpGeomVertexFormat) _format;

  typedef pvector<PTA_uchar> Arrays;

  // We have to use reference-counting pointers here instead of having
  // explicit cleanup in the GeomVertexFormat destructor, because the
  // cache needs to be stored in the CycleData, which makes accurate
  // cleanup more difficult.  We use the GeomVertexCacheManager class
  // to avoid cache bloat.
  typedef pmap<CPT(qpGeomVertexFormat), CPT(qpGeomVertexData) > ConvertedCache;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    Arrays _arrays;
    ConvertedCache _converted_cache;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

private:
  void do_set_num_vertices(int n, CDWriter &cdata);

  static PStatCollector _munge_data_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "qpGeomVertexData",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeomVertexCacheManager;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexData &obj);

#include "qpgeomVertexData.I"

#endif
