/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamMatrix.h
 * @author brian
 * @date 2022-03-22
 */

#ifndef MATERIALPARAMMATRIX_H
#define MATERIALPARAMMATRIX_H

#include "pandabase.h"
#include "materialParamBase.h"
#include "luse.h"

/**
 * A transform matrix material parameter.  Can be specified as a list of
 * values for each cell or separate vectors for each transform component.
 */
class EXPCL_PANDA_MATERIAL MaterialParamMatrix final : public MaterialParamBase {
PUBLISHED:
  INLINE MaterialParamMatrix(const std::string &name, const LMatrix4 &default_value = LMatrix4::ident_mat());

  INLINE void set_value(const LMatrix4 &mat);
  INLINE const LMatrix4 &get_value() const;
  MAKE_PROPERTY(value, get_value, set_value);

private:
  LMatrix4 _value;

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
    register_type(_type_handle, "MaterialParamMatrix",
                  MaterialParamBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "materialParamMatrix.I"

#endif // MATERIALPARAMMATRIX_H
