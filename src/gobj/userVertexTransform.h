/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userVertexTransform.h
 * @author drose
 * @date 2005-03-24
 */

#ifndef USERVERTEXTRANSFORM_H
#define USERVERTEXTRANSFORM_H

#include "pandabase.h"
#include "vertexTransform.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class FactoryParams;

/**
 * This is a specialization on VertexTransform that allows the user to specify
 * any arbitrary transform matrix he likes.  This is rarely used except for
 * testing.
 */
class EXPCL_PANDA_GOBJ UserVertexTransform : public VertexTransform {
PUBLISHED:
  explicit UserVertexTransform(const std::string &name);

  INLINE const std::string &get_name() const;

  virtual void output(std::ostream &out) const;

private:
  std::string _name;

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
    VertexTransform::init_type();
    register_type(_type_handle, "UserVertexTransform",
                  VertexTransform::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "userVertexTransform.I"

#endif
