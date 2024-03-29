/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamFloat.h
 * @author brian
 * @date 2021-03-07
 */

#ifndef MATERIALPARAMFLOAT_H
#define MATERIALPARAMFLOAT_H

#include "materialParamBase.h"

/**
 * A scalar material parameter.
 */
class EXPCL_PANDA_MATERIAL MaterialParamFloat final : public MaterialParamBase {
PUBLISHED:
  INLINE MaterialParamFloat(const std::string &name, PN_stdfloat default_value = 0.0f);

  INLINE void set_value(PN_stdfloat value);
  INLINE PN_stdfloat get_value() const;
  MAKE_PROPERTY(value, get_value, set_value);

private:
  PN_stdfloat _value;

public:
  virtual bool from_pdx(const PDXValue &val, const DSearchPath &search_path) override;
  virtual void to_pdx(PDXValue &val, const Filename &filename) override;

  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  static void register_with_read_factory();

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MaterialParamBase::init_type();
    register_type(_type_handle, "MaterialParamFloat",
                  MaterialParamBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "materialParamFloat.I"

#endif // MATERIALPARAMFLOAT_H
