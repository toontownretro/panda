/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialGroup.h
 * @author lachbr
 * @date 2021-02-14
 */

#ifndef MATERIALGROUP_H
#define MATERIALGROUP_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "renderState.h"
#include "pvector.h"

/**
 * Class that contains a group of materials/RenderStates.
 */
class EXPCL_PANDA_PGRAPH MaterialGroup : public TypedWritableReferenceCount {
PUBLISHED:
  INLINE void add_material(const RenderState *state);
  INLINE size_t get_num_materials() const;
  INLINE const RenderState *get_material(size_t n) const;
  INLINE void clear_materials();
  MAKE_SEQ(get_materials, get_num_materials, get_material);
  MAKE_SEQ_PROPERTY(materials, get_num_materials, get_material);

private:
  pvector<CPT(RenderState)> _states;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MaterialGroup",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "materialGroup.I"

#endif // MATERIALGROUP_H
