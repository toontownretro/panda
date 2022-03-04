/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerHLSL.h
 * @author brian
 * @date 2022-03-02
 */

#ifndef SHADERCOMPILERHLSL_H
#define SHADERCOMPILERHLSL_H

#include "pandabase.h"

#include "shaderCompiler.h"
#include "shaderModuleDXBC.h"

/**
 * ShaderCompiler implementation that uses d3dcompiler to compile HLSL shaders
 * to Direct3D shader byte code.
 */
class EXPCL_PANDA_GOBJ ShaderCompilerHLSL : public ShaderCompiler {
public:
  ShaderCompilerHLSL();

  virtual std::string get_name() const override;
  virtual ShaderLanguages get_languages() const override;
  virtual PT(ShaderModule) compile_now(Stage stage, std::istream &in,
                                       const Filename &fullpath,
                                       const Options &options = Options(),
                                       BamCacheRecord *record = nullptr) const override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderCompiler::init_type();
    register_type(_type_handle, "ShaderCompilerHLSL",
                  ShaderCompiler::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "shaderCompilerHLSL.I"

#endif // SHADERCOMPILERHLSL_H
