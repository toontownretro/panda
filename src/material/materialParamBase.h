/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamBase.h
 * @author brian
 * @date 2021-03-07
 */

#ifndef MATERIALPARAMBASE_H
#define MATERIALPARAMBASE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "factoryParams.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pdxValue.h"

/**
 * Base material parameter class.
 */
class EXPCL_PANDA_MATERIAL MaterialParamBase : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  INLINE MaterialParamBase(const std::string &name);

public:
  // Called when reading in the material from a keyvalues file.  The search
  // path includes the directory of the material file and the model path, so
  // texture filenames can be resolved.
  virtual bool from_pdx(const PDXValue &value, const DSearchPath &search_path)=0;

  // Called when writing the material to a keyvalues file.  The output filename
  // of the material is used for making texture parameter pathnames relative to
  // the output filename.
  virtual void to_pdx(PDXValue &value, const Filename &filename)=0;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MaterialParamBase",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "materialParamBase.I"

#endif // MATERIALPARAMBASE_H
