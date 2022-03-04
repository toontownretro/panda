/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerHLSL.cxx
 * @author brian
 * @date 2022-03-02
 */

#ifndef CPPPARSER

#include "shaderCompilerHLSL.h"
#include "virtualFile.h"
#include "string_utils.h"
#include "config_gobj.h"
#include "config_putil.h"
#include "virtualFileSystem.h"

#include <d3dcompiler.h>

TypeHandle ShaderCompilerHLSL::_type_handle;

/**
 *
 */
class D3DInclude : public ID3DInclude {
public:
  D3DInclude(const Filename &main_shader_dir) {
    _search_path = get_model_path();
    _search_path.append_directory(main_shader_dir);
  }

  virtual HRESULT
  Open(D3D_INCLUDE_TYPE include_type, LPCSTR filename,
       LPCVOID parent_data, LPCVOID *data, UINT *length) override {

    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    Filename panda_filename = Filename::from_os_specific(filename);
    // Resolve on model path + main shader directory.
    if (!vfs->resolve_filename(panda_filename, _search_path)) {
      shaderpipeline_cat.error()
        << "Could not resolve HLSL shader include file " << panda_filename
        << " on search path " << _search_path << "\n";
      return S_OK;
    }

    // Append directory of included shader to search path.
    _search_path.append_directory(panda_filename.get_dirname());

    std::string str = vfs->read_file(panda_filename, true);

    unsigned char *buf = new unsigned char[str.size()];
    memcpy(buf, str.c_str(), str.size());
    *data = buf;
    *length = str.size();

    return S_OK;
  }

  virtual HRESULT
  Close(LPCVOID data) override {
    unsigned char *buf = (unsigned char *)data;
    delete[] buf;
    return S_OK;
  }

private:
  DSearchPath _search_path;
};

/**
 *
 */
ShaderCompilerHLSL::
ShaderCompilerHLSL() {
}

/**
 *
 */
std::string ShaderCompilerHLSL::
get_name() const {
  return "HLSL D3DCompile";
}

/**
 *
 */
ShaderLanguages ShaderCompilerHLSL::
get_languages() const {
  return {
    Shader::SL_HLSL
  };
}

/**
 * Compiles the source code from the given input stream, producing a
 * ShaderModule on success.
 */
PT(ShaderModule) ShaderCompilerHLSL::
compile_now(ShaderModule::Stage stage, std::istream &in,
            const Filename &fullpath, const Options &options,
            BamCacheRecord *record) const {
  vector_uchar code;
  if (!VirtualFile::simple_read_file(&in, code)) {
    shader_cat.error()
      << "Failed to read " << stage << " shader from stream.\n";
    return nullptr;
  }

  pvector<D3D_SHADER_MACRO> macros;
  vector_string names;
  vector_string defs;
  if (options.get_num_defines() > 0u) {
    macros.resize(options.get_num_defines() + 1);
    memset(macros.data(), 0, sizeof(D3D_SHADER_MACRO) * (options.get_num_defines() + 1));

    names.resize(options.get_num_defines());
    defs.resize(options.get_num_defines());

    for (int i = 0; i < options.get_num_defines(); ++i) {
      names[i] = options.get_define(i)->name->get_name();
      std::ostringstream ss;
      ss << options.get_define(i)->value;
      defs[i] = ss.str();

      macros[i].Definition = defs[i].c_str();
      macros[i].Name = names[i].c_str();
    }
  }

  // Infer shader model and stage from filename.
  std::string basename = fullpath.get_basename_wo_extension();
  vector_string words;
  tokenize(basename, words, ".", true);
  if (words.size() == 1) {
    shaderpipeline_cat.error()
      << "HLSL shader filename `" << fullpath.get_fullpath()
      << "` does not specify a shader target\n";
    return nullptr;
  }
  std::string target = words[words.size() - 1u];

  char stage_char;
  switch (stage) {
  case ShaderModule::Stage::vertex:
    stage_char = 'v';
    break;
  case ShaderModule::Stage::fragment:
    stage_char = 'p';
    break;
  case ShaderModule::Stage::geometry:
    stage_char = 'g';
    break;
  case ShaderModule::Stage::compute:
    stage_char = 'c';
    break;
  default:
    stage_char = 'x';
    break;
  }

  if (target[0] != stage_char) {
    shaderpipeline_cat.error()
      << "HLSL shader target `" << target << "` from filename `" << fullpath.get_fullpath()
      << "` does not match specified shader module stage " << stage << "\n";
    return nullptr;
  }

  D3DInclude include(fullpath.get_dirname());

  ID3DBlob *byte_code = nullptr;
  ID3DBlob *error_msgs = nullptr;
  HRESULT hr = D3DCompile(code.data(), code.size(), nullptr, macros.empty() ? nullptr : macros.data(),
                          &include, "main", target.c_str(), D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0,
                          &byte_code, &error_msgs);
  if (FAILED(hr)) {
    return nullptr;
  }

  nassertr(byte_code != nullptr, nullptr);

  return new ShaderModuleDXBC(stage, byte_code);
}

#endif
