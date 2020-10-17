/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderParamAttrib.h
 * @author lachbr
 * @date 2020-10-15
 */

#ifndef SHADERPARAMATTRIB_H
#define SHADERPARAMATTRIB_H

#include "config_pgraph.h"
#include "renderAttrib.h"
#include "simpleHashMap.h"

/**
 * Render attribute that specifies a shader to be used and miscellaneous
 * parameters for the shader.
 */
class EXPCL_PANDA_PGRAPH ShaderParamAttrib : public RenderAttrib {
private:
  INLINE ShaderParamAttrib(const std::string &shader_name);
  INLINE ShaderParamAttrib(const ShaderParamAttrib &other);

PUBLISHED:
  INLINE static CPT(RenderAttrib) make(const std::string &shader_name);
  INLINE CPT(RenderAttrib) set_shader_name(const std::string &name) const;
  INLINE CPT(RenderAttrib) set_param(const std::string &key,
                                     const std::string &value) const;

  INLINE int get_num_params() const;
  INLINE int find_param(const std::string &key) const;
  INLINE bool has_param(const std::string &key) const;
  INLINE const std::string &get_param_key(int n) const;
  INLINE const std::string &get_param_value(int n) const;

  INLINE const std::string &get_shader_name() const;

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:
  std::string _shader_name;

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
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "ShaderParamAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 10, new ShaderParamAttrib("Default"));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "shaderParamAttrib.I"

#endif // SHADERPARAMATTRIB_H
