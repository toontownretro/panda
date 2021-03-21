/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialBase.h
 * @author lachbr
 * @date 2021-03-06
 */

#ifndef MATERIALBASE_H
#define MATERIALBASE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "pointerTo.h"
#include "texture.h"
#include "luse.h"
#include "pmap.h"
#include "keyValues.h"
#include "internalName.h"
#include "materialParamBase.h"
#include "config_putil.h"

/**
 * This is an abstract base class for any kind of material that can be applied
 * to a surface.  At its core, a material can have a name and a number of
 * named parameters.  It is up to derived material types to expose the
 * parameters that can be set for that particular material.
 */
class EXPCL_PANDA_GOBJ MaterialBase : public TypedWritableReferenceCount, public Namable {
public:
  MaterialBase(const std::string &name);

  virtual void read_keyvalues(KeyValues *kv, const DSearchPath &search_path);
  virtual void write_keyvalues(KeyValues *kv, const Filename &filename);

PUBLISHED:
  INLINE MaterialParamBase *get_param(CPT_InternalName name) const;

  INLINE void set_filename(const Filename &filename);
  INLINE const Filename &get_filename() const;
  MAKE_PROPERTY(filename, get_filename, set_filename);

  INLINE void set_fullpath(const Filename &fullpath);
  INLINE const Filename &get_fullpath() const;
  MAKE_PROPERTY(fullpath, get_fullpath, set_fullpath);

  void write_pmat(const Filename &filename);
  bool write_mto(const Filename &filename);

protected:
  INLINE void set_param(MaterialParamBase *param);
  INLINE void clear_param(MaterialParamBase *param);

protected:
  Filename _filename;
  Filename _fullpath;

  typedef pmap<CPT_InternalName, PT(MaterialParamBase)> Params;
  Params _params;

private:
  // Only used during Bam reading.
  int _num_params;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

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
    register_type(_type_handle, "MaterialBase",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


#include "materialBase.I"

#endif // MATERIALBASE_H
