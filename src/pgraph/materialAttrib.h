/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.h
 * @author lachbr
 * @date 2021-03-21
 */

#ifndef MATERIALATTRIB_H
#define MATERIALATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"
#include "material.h"
#include "renderState.h"

/**
 * A render attribute that contains a material object.
 */
class EXPCL_PANDA_PGRAPH MaterialAttrib : public RenderAttrib {
private:
  INLINE MaterialAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make(Material *material);
  static CPT(RenderAttrib) make_default();

  INLINE Material *get_material() const;
  MAKE_PROPERTY(material, get_material);

  INLINE const RenderState *get_modifier_state() const;
  MAKE_PROPERTY(modifier_state, get_modifier_state);

  INLINE bool is_off() const;

private:
  void create_modifier_state();

private:
  PT(Material) _material;
  // This is a RenderState that contains the attributes modified by the
  // material itself, such as transparency, color scale, etc.  It is composed
  // with the RenderState of Geoms that use the material when recorded during
  // the Cull traversal.
  CPT(RenderState) _modifier_state;

  bool _is_off;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

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
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "MaterialAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 15, new MaterialAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;

};

#include "materialAttrib.I"

#endif // MATERIALATTRIB_H
