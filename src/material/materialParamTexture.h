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
class EXPCL_PANDA_MATERIAL MaterialParamTexture final : public MaterialParamBase {
PUBLISHED:
  INLINE MaterialParamTexture(const std::string &name, Texture *default_value = nullptr);

  INLINE void set_value(Texture *tex, int view = 0);
  INLINE Texture *get_value() const;
  MAKE_PROPERTY(value, get_value, set_value);

  INLINE void set_view(int view);
  INLINE int get_view() const;
  MAKE_PROPERTY(view, get_view, set_view);

  INLINE void set_sampler_state(const SamplerState &sampler);
  INLINE const SamplerState &get_sampler_state() const;
  INLINE void clear_sampler_state();
  INLINE bool has_sampler_state() const;
  MAKE_PROPERTY(sampler_state, get_sampler_state, set_sampler_state);

private:
  PT(Texture) _value;

  // For multi-view textures, specifies the view index to use for
  // this parameter.
  int _view;

  // The parameter can use the Texture's default sampler or specify
  // a custom one here for this parameter only.
  bool _has_sampler;
  SamplerState _sampler;

public:
  virtual bool from_pdx(const PDXValue &val, const DSearchPath &search_path) override;
  virtual void to_pdx(PDXValue &val, const Filename &filename) override;

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
