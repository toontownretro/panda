/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderSetup.h
 * @author brian
 * @date 2022-08-22
 */

#ifndef SHADERSETUP_H
#define SHADERSETUP_H

#include "pandabase.h"
#include "bitMask.h"
#include "pmap.h"
#include "shader.h"
#include "shaderInput.h"
#include "pvector.h"
#include "shaderStage.h"
#include "filename.h"
#include "internalName.h"

class ShaderSetup {
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

    SF_all = (SF_vertex | SF_pixel | SF_geometry | SF_tess | SF_tess_eval),
  };

  // Setup specific to the generated Shader object.
  class ShaderObjectSetup {
  public:
    BitMask32 _stage_flags;
    ShaderStage _stages[S_COUNT];
    Shader::ShaderLanguage _language;
    pmap<const InternalName *, unsigned int> _spec_constants;

    INLINE ShaderObjectSetup();
    INLINE ShaderObjectSetup(const ShaderObjectSetup &other);

    INLINE void clear();
    INLINE size_t get_hash() const;

    INLINE void calc_variation_indices();

    INLINE bool operator < (const ShaderObjectSetup &other) const;
    INLINE bool operator == (const ShaderObjectSetup &other) const;
    bool operator != (const ShaderObjectSetup &other) const {
      return !operator ==(other);
    }
  };

  // Setup specific to the generated ShaderAttrib.
  class ShaderAttrSetup {
  public:
    int _flags;
    pvector<ShaderInput> _inputs;
    int _instance_count;

    INLINE ShaderAttrSetup();
    INLINE ShaderAttrSetup(const ShaderAttrSetup &other);

    INLINE void clear();
    INLINE size_t get_hash() const;

    INLINE bool operator < (const ShaderAttrSetup &other) const;
    INLINE bool operator == (const ShaderAttrSetup &other) const;
    bool operator != (const ShaderAttrSetup &other) const {
      return !operator ==(other);
    }
  };

  INLINE size_t get_num_inputs() const;
  INLINE const pvector<ShaderInput> &get_inputs() const;
  INLINE pvector<ShaderInput> &&move_inputs();

  INLINE ShaderStage &get_stage(Stage stage);
  INLINE bool has_stage(StageFlags flags) const;

  INLINE int get_flags() const;

  INLINE int get_instance_count() const;

  INLINE Shader::ShaderLanguage get_language() const;

  INLINE void set_vertex_shader(const Filename &filename);
  INLINE void set_vertex_shader_combo(size_t n, int value);
  INLINE void set_vertex_shader_combo(const InternalName *name, int value);

  INLINE void set_pixel_shader(const Filename &filename);
  INLINE void set_pixel_shader_combo(size_t n, int value);
  INLINE void set_pixel_shader_combo(const InternalName *name, int value);

  INLINE void set_geometry_shader(const Filename &filename);
  INLINE void set_geometry_shader_combo(size_t n, int value);
  INLINE void set_geometry_shader_combo(const InternalName *name, int value);

  INLINE void set_tess_shader(const Filename &filename);
  INLINE void set_tess_shader_combo(size_t n, int value);
  INLINE void set_tess_shader_combo(const InternalName *name, int value);

  INLINE void set_tess_eval_shader(const Filename &filename);
  INLINE void set_tess_eval_shader_combo(size_t n, int value);
  INLINE void set_tess_eval_shader_combo(const InternalName *name, int value);

  INLINE void set_input(const ShaderInput &input);
  INLINE void set_input(ShaderInput &&input);

  INLINE void set_flags(int flags);

  INLINE void set_instance_count(int count);

  INLINE void set_language(Shader::ShaderLanguage language);

  INLINE void set_spec_constant(const InternalName *name, bool value);
  INLINE void set_spec_constant(const InternalName *name, float value);
  INLINE void set_spec_constant(const InternalName *name, int value);
  INLINE void set_spec_constant(const InternalName *name, unsigned int value);

public:
  ShaderObjectSetup _obj_setup;
  ShaderAttrSetup _setup;
};



#include "shaderSetup.I"

#endif // SHADERSETUP_H
