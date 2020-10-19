/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.h
 * @author lachbr
 * @date 2020-10-16
 */

#ifndef SHADERBASE_H
#define SHADERBASE_H

#include "config_shader.h"
#include "typedObject.h"
#include "namable.h"
#include "renderAttrib.h"
#include "shaderStage.h"
#include "pvector.h"
#include "shaderInput.h"
#include "shader.h"

class GraphicsStateGuardianBase;
class RenderState;
class GeomVertexAnimationSpec;
class ShaderParamAttrib;

/**
 * Base shader class.
 */
class EXPCL_PANDA_SHADER ShaderBase : public TypedObject, public Namable {
public:
  enum Stage {
    S_vertex,
    S_pixel,
    S_geometry,
    S_tess,
    S_tess_eval,
    S_COUNT,
  };

  enum StageFlags {
    SF_none = 0,
    SF_vertex = 1,
    SF_pixel = 2,
    SF_geometry = 4,
    SF_tess = 8,
    SF_tess_eval = 16,
  };

  INLINE size_t get_num_inputs() const;
  INLINE const pvector<ShaderInput> &get_inputs() const;

  INLINE ShaderStage &get_stage(Stage stage);
  INLINE bool has_stage(StageFlags flags) const;

  INLINE int get_flags() const;

  INLINE Shader::ShaderLanguage get_language() const;

  virtual void reset();

  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               const ShaderParamAttrib *params,
                               const GeomVertexAnimationSpec &anim_spec) = 0;

protected:
  INLINE ShaderBase(const std::string &name);

  static void register_shader(ShaderBase *shader);

  INLINE void set_vertex_shader(const Filename &filename);
  INLINE void set_vertex_shader_source(const std::string &source);
  template <class T>
  INLINE void set_vertex_shader_define(const std::string &name, const T &value);
  INLINE void set_vertex_shader_define(const std::string &name,
                                       const std::string &value = "1");

  INLINE void set_pixel_shader(const Filename &filename);
  INLINE void set_pixel_shader_source(const std::string &source);
  template <class T>
  INLINE void set_pixel_shader_define(const std::string &name, const T &value);
  INLINE void set_pixel_shader_define(const std::string &name,
                                      const std::string &value = "1");


  INLINE void set_geometry_shader(const Filename &filename);
  INLINE void set_geometry_shader_source(const std::string &source);
  template <class T>
  INLINE void set_geometry_shader_define(const std::string &name, const T &value);
  INLINE void set_geometry_shader_define(const std::string &name,
                                         const std::string &value = "1");

  INLINE void set_tess_shader(const Filename &filename);
  INLINE void set_tess_shader_source(const std::string &source);
  template <class T>
  INLINE void set_tess_shader_define(const std::string &name, const T &value);
  INLINE void set_tess_shader_define(const std::string &name,
                                     const std::string &value = "1");

  INLINE void set_tess_eval_shader(const Filename &filename);
  INLINE void set_tess_eval_shader_source(const std::string &source);
  template <class T>
  INLINE void set_tess_eval_shader_define(const std::string &name, const T &value);
  INLINE void set_tess_eval_shader_define(const std::string &name,
                                          const std::string &value = "1");

  INLINE void set_input(const ShaderInput &input);
  INLINE void set_input(ShaderInput &&input);

  INLINE void set_flags(int flags);

  INLINE void set_language(Shader::ShaderLanguage language);

protected:
  int _stage_flags;
  ShaderStage _stages[S_COUNT];

  int _flags;
  pvector<ShaderInput> _inputs;

  Shader::ShaderLanguage _language;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderBase",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderBase.I"
#include "shaderBase.T"

#endif // SHADERBASE_H
