/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompiler.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERCOMPILER_H
#define SHADERCOMPILER_H

#include "pandabase.h"

#include "pvector.h"
#include "typedObject.h"
#include "shader.h"
#include "shaderModule.h"

typedef pvector<Shader::Shader::ShaderLanguage> ShaderLanguages;

/**
 * This is the base class for objects to compile various types of shader code.
 */
class EXPCL_PANDA_GOBJ ShaderCompiler : public TypedObject, public ShaderEnums {
protected:
  ShaderCompiler();

public:
  virtual ~ShaderCompiler();

PUBLISHED:
  class Options {
  PUBLISHED:

    class Define {
    PUBLISHED:
      INLINE Define();
      INLINE Define(const Define &other);
      INLINE bool operator == (const CPT_InternalName &other_name) const { return name == other_name; }

      CPT(InternalName) name;
      int value;
    };

    Options() = default;

    INLINE void set_define(CPT_InternalName name, int value);
    INLINE size_t get_num_defines() const;
    INLINE const Define *get_define(size_t n) const;
    INLINE const Define *get_define(CPT_InternalName name) const;

  private:
    pvector<Define> _defines;
  };

  virtual std::string get_name() const=0;
  virtual ShaderLanguages get_languages() const=0;
  virtual PT(ShaderModule) compile_now(Stage stage, const Filename &path,
                                       const Options &options = Options(),
                                       BamCacheRecord *record = nullptr) const;
  virtual PT(ShaderModule) compile_now(Stage stage, std::istream &in,
                                       const Filename &fullpath,
                                       const Options &options = Options(),
                                       BamCacheRecord *record = nullptr) const=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderCompiler",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderCompiler.I"

#endif // SHADERCOMPILER_H
