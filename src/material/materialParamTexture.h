/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamTexture.h
 * @author lachbr
 * @date 2021-03-07
 */

#ifndef MATERIALPARAMTEXTURE_H
#define MATERIALPARAMTEXTURE_H

#include "materialParamBase.h"
#include "texture.h"

/**
 * A texture material parameter.
 */
class MaterialParamTexture final : public MaterialParamBase {
PUBLISHED:
  INLINE MaterialParamTexture(const std::string &name, Texture *default_value = nullptr);

  INLINE void set_value(Texture *tex);
  INLINE Texture *get_value() const;
  MAKE_PROPERTY(value, get_value, set_value);

private:
  PT(Texture) _value;

public:
  virtual bool from_string(const std::string &str, const DSearchPath &search_path) override;
  virtual void to_string(std::string &str, const Filename &filename) override;

  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;
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
    register_type(_type_handle, "MaterialParamTexture",
                  MaterialParamBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "materialParamTexture.I"

#endif // MATERIALPARAMTEXTURE_H
