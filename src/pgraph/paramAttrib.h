/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramAttrib.h
 * @author lachbr
 * @date 2020-10-15
 */

#ifndef PARAMATTRIB_H
#define PARAMATTRIB_H

#include "config_pgraph.h"
#include "renderAttrib.h"
#include "simpleHashMap.h"
#include "luse.h"
#include "string_utils.h"
#include "keyValues.h"

/**
 * Render attribute that contains arbitrary key-value parameters.
 */
class EXPCL_PANDA_PGRAPH ParamAttrib : public RenderAttrib {
private:
  INLINE ParamAttrib();
  INLINE ParamAttrib(const ParamAttrib &other);

PUBLISHED:
  INLINE static CPT(RenderAttrib) make();
  INLINE CPT(RenderAttrib) set_param(const std::string &key,
                                     const std::string &value) const;

  INLINE int get_num_params() const;
  INLINE int find_param(const std::string &key) const;
  INLINE bool has_param(const std::string &key) const;
  INLINE const std::string &get_param_key(int n) const;
  INLINE const std::string &get_param_value(int n) const;
  INLINE bool get_param_value_bool(int n) const;
  INLINE int get_param_value_int(int n) const;
  INLINE float get_param_value_float(int n) const;
  INLINE LVecBase2f get_param_value_2f(int n) const;
  INLINE LVecBase3f get_param_value_3f(int n) const;
  INLINE LVecBase4f get_param_value_4f(int n) const;

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:
  typedef SimpleHashMap<std::string, std::string, string_hash> Params;
  Params _params;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

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
    RenderAttrib::init_type();
    register_type(_type_handle, "ParamAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 10, new ParamAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "paramAttrib.I"

#endif // PARAMATTRIB_H
