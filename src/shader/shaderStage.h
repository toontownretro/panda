/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.h
 * @author brian
 * @date 2020-10-18
 */

#ifndef SHADERSTAGE_H
#define SHADERSTAGE_H

#include "config_shader.h"
#include "filename.h"
#include "shaderObject.h"
#include "lightMutex.h"

#include <string>

/**
 * This class represents a single stage of a generated shader program.  It
 * contains a set of #defines that will be inserted into the associated
 * source when creating the shader program.
 */
class EXPCL_PANDA_SHADER ShaderStage {
public:
  INLINE ShaderStage();
  INLINE void operator = (const ShaderStage &other);

  INLINE void reset();

  INLINE void set_source_filename(const Filename &filename);

  INLINE size_t add_hash(size_t hash) const;
  INLINE bool operator < (const ShaderStage &other) const;
  INLINE bool operator == (const ShaderStage &other) const;
  INLINE bool operator != (const ShaderStage &other) const {
    return !operator ==(other);
  }

  INLINE void set_combo_value(size_t i, int value);
  INLINE void set_combo_value(const InternalName *name, int value);

  INLINE void calc_variation_index();
  INLINE size_t get_variation_index() const;

  void spew_variation();

  INLINE const ShaderObject *get_object() const;
  INLINE ShaderModule *get_module() const;

  static const ShaderObject *load_shader_object(const Filename &filename);
  static void clear_sho_cache();

private:
  // The shader object containing precompiled shader modules for each
  // combination of preprocessor values.
  const ShaderObject *_object;

  // Values for each combo that the shader object contains.
  // By default, all combo values are initialized to 0.
  // At the end of shader generation, the variation index will be
  // computed from the values of all combos.
  vector_int _combo_values;
  pset<int> _specified_combos;
  size_t _variation_index;

  typedef pflat_hash_map<Filename, CPT(ShaderObject), string_hash> ObjectCache;
  static ObjectCache _object_cache;
  static LightMutex _object_cache_lock;
};

#include "shaderStage.I"

#endif // SHADERSTAGE_H
