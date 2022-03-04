/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleDXBC.h
 * @author brian
 * @date 2022-03-02
 */

#ifndef SHADERMODULEDXBC_H
#define SHADERMODULEDXBC_H

#include "pandabase.h"

#include "shaderModule.h"

#include <d3dcommon.h>

#ifdef CPPPARSER
class ID3DBlob;
#endif

class ID3D11ShaderReflectionType;

/**
 * ShaderModule that contains compiled DirectX shader bytecode.
 */
class EXPCL_PANDA_GOBJ ShaderModuleDXBC : public ShaderModule {
private:
  ShaderModuleDXBC(Stage stage);

public:
  ShaderModuleDXBC(Stage stage, ID3DBlob *byte_code);
  virtual ~ShaderModuleDXBC();

  void reflect_module();

  void spew_module();

  INLINE ID3DBlob *get_byte_code() const;

  virtual PT(CopyOnWriteObject) make_cow_copy() override;
  virtual std::string get_ir() const override;

protected:
  ShaderModuleDXBC(const ShaderModuleDXBC &copy);

  const ShaderType *reflect_variable_type(ID3D11ShaderReflectionType *trefl);

private:
  ID3DBlob *_byte_code;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderModule::init_type();
    register_type(_type_handle, "ShaderModuleDXBC",
                  ShaderModule::get_class_type());
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

#include "shaderModuleDXBC.I"

#endif // SHADERMODULEDXBC_H
