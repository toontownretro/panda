/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderObject.h
 * @author brian
 * @date 2020-12-22
 */

#ifndef SHADEROBJECT_H
#define SHADEROBJECT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "internalName.h"
#include "shaderModule.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pvector.h"
#include "config_putil.h"
#include "virtualFile.h"
#include "vector_int.h"
#include "shader.h"
#include "shaderCompiler.h"
#include "shaderEnums.h"

class FactoryParams;

/**
 * This class represents what we call a shader object: a single shader module
 * containing SPIR-V byte code for each possible combination of preprocessor
 * defines that can be set, called combos.  The shader object simply contains
 * an array of SPIR-V shaders, originating from a single GLSL source file.
 * Each index into the array corresponds to a unique combination of
 * preprocessor definition values.
 *
 * This class also supports doing dynamic compilation of modules for each
 * combination specified.  In this mode, the shader source file will be read
 * and combo defintions populated, but the actual modules themselves will
 * remain uncompiled.  It is up to the user to then request a particular
 * variation index, in which case the object will compile that module on
 * the fly if it hasn't been compiled yet.
 */
class EXPCL_PANDA_SHADER ShaderObject : public TypedWritableReferenceCount {
PUBLISHED:
  class Combo {
  PUBLISHED:
    INLINE Combo();
    INLINE bool operator == (const Combo &other) const {
      return name == other.name;
    }

    CPT(InternalName) name;
    int min_val;
    int max_val;
    int scale;
  };

  /**
   * This class allows the user to build up a list of combo
   * values and return the shader module from the final set of
   * values.
   */
  class EXPCL_PANDA_SHADER VariationBuilder {
  PUBLISHED:
    INLINE VariationBuilder(ShaderObject *obj = nullptr);

    INLINE void reset(ShaderObject *obj);
    INLINE void set_combo_value(int combo, int value);
    INLINE void set_combo_value(const InternalName *combo_name, int value);

    INLINE ShaderObject *get_object() const { return _obj; }

    size_t get_module_index() const;

    ShaderModule *get_module(bool compile_if_necessary) const;

  public:
    ShaderObject *_obj;
    pvector<int> _combo_values;
  };

  class EXPCL_PANDA_SHADER SkipCommand {
  public:
    enum Command {
      C_and,
      C_or,
      C_not,
      C_ref,
      C_eq,
      C_neq,
      C_literal,
    };

    int eval(const VariationBuilder &builder) const;

    Command cmd;
    pvector<SkipCommand> arguments;
    int value;
    CPT_InternalName name;
  };
  typedef pvector<SkipCommand> SkipCommands;

  INLINE ShaderObject();

  INLINE void add_combo(Combo &&combo);
  INLINE void add_combo(const Combo &combo);
  INLINE bool has_combo(CPT_InternalName name) const;
  INLINE const Combo &get_combo(size_t n) const;
  INLINE const Combo &get_combo(CPT_InternalName name) const;
  INLINE size_t get_num_combos() const;

  INLINE int get_combo_index(const InternalName *name) const;

  INLINE void add_permutation(ShaderModule *module);
  INLINE void set_permutation(size_t n, ShaderModule *module);
  INLINE void resize_permutations(size_t count);
  INLINE ShaderModule *get_permutation(size_t n) const;
  INLINE size_t get_num_permutations() const;

  INLINE size_t get_total_combos() const;

  INLINE void add_skip_command(SkipCommand &&cmd);
  INLINE size_t get_num_skip_commands() const;
  INLINE const SkipCommand *get_skip_command(size_t n) const;

  /**
   * Returns the virtual file pointer of the shader source code, if the
   * ShaderObject has been read from source.
   *
   * This returns nullptr if this is a precompiled ShaderObject loaded
   * from bam.
   */
  INLINE VirtualFile *get_virtual_file() const { return _vfile; }
  INLINE Shader::ShaderLanguage get_shader_language() const { return _lang; }

public:
  INLINE ShaderModule::Stage get_shader_stage() const { return _stage; }
  static ShaderObject *read_source(Shader::ShaderLanguage lang, ShaderModule::Stage stage, Filename filename,
                                   const DSearchPath &search_path = get_model_path());

private:
  void calc_total_combos();

private:
  typedef pvector<Combo> Combos;
  Combos _combos;

  typedef pflat_hash_map<const InternalName *, int, pointer_hash> CombosByName;
  CombosByName _combos_by_name;

  typedef pvector<PT(ShaderModule)> Permutations;
  Permutations _permutations;

  size_t _total_combos;

private:
  // Stuff specific to doing dynamic compilation.
  PT(VirtualFile) _vfile;
  Shader::ShaderLanguage _lang;
  ShaderModule::Stage _stage;
  SkipCommands _skip_commands;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
	virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ShaderObject",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderObject.I"

#endif // SHADEROBJECT_H
