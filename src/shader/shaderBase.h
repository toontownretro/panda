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
#include "pmap.h"
#include "string_utils.h"
#include "stl_compares.h"
#include "lightMutex.h"
#include "shaderSetup.h"

class GraphicsStateGuardianBase;
class RenderState;
class GeomVertexAnimationSpec;
class Material;

/**
 * Base shader class.
 */
class EXPCL_PANDA_SHADER ShaderBase : public TypedObject, public Namable {
public:

  INLINE void add_alias(const std::string &alias);
  INLINE size_t get_num_aliases() const;
  INLINE const std::string &get_alias(size_t n) const;

  void clear_cache();

  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) = 0;

  bool has_additive_blend(const RenderState *state) const;
  bool has_modulate_blend(const RenderState *state) const;

protected:
  INLINE ShaderBase(const std::string &name);

  static void register_shader(ShaderBase *shader);
  static void register_shader(ShaderBase *shader, TypeHandle material_type);

private:
  typedef pflat_hash_map<ShaderSetup::ShaderObjectSetup, PT(Shader)> ObjectSetupCache;
  typedef pflat_hash_map<ShaderSetup::ShaderAttrSetup, CPT(RenderAttrib)> SetupCache;
  ObjectSetupCache _obj_cache;
  SetupCache _cache;
  // This mutex protects the above caches.
  LightMutex _lock;

  vector_string _aliases;

  friend class ShaderManager;

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

#endif // SHADERBASE_H
