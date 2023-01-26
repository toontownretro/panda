/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleDXBC.cxx
 * @author brian
 * @date 2022-03-02
 */

#ifndef CPPPARSER

#include "shaderModuleDXBC.h"
#include "config_gobj.h"

#include <d3dcompiler.h>

TypeHandle ShaderModuleDXBC::_type_handle;

/**
 *
 */
ShaderModuleDXBC::
ShaderModuleDXBC(Stage stage) :
  ShaderModule(stage),
  _byte_code(nullptr)
{
}

/**
 *
 */
ShaderModuleDXBC::
ShaderModuleDXBC(Stage stage, ID3DBlob *byte_code) :
  ShaderModule(stage),
  _byte_code(byte_code)
{
  reflect_module();

#if 0 // FIXME if/when D3D11 is returned to.
  _used_caps = (
    Shader::C_basic_shader |
    Shader::C_vertex_texture |
    Shader::C_shadow_samplers |
    Shader::C_invariant |
    Shader::C_matrix_non_square |
    Shader::C_integer |
    Shader::C_texture_lod |
    Shader::C_texture_fetch |
    Shader::C_sampler_cube_shadow |
    Shader::C_vertex_id |
    Shader::C_round_even |
    Shader::C_instance_id |
    Shader::C_buffer_texture |
    Shader::C_geometry_shader |
    Shader::C_primitive_id |
    Shader::C_bit_encoding |
    Shader::C_texture_gather |
    Shader::C_double |
    Shader::C_cube_map_array |
    Shader::C_tessellation_shader |
    Shader::C_sample_variables |
    Shader::C_extended_arithmetic |
    Shader::C_texture_query_lod |
    Shader::C_image_load_store |
    Shader::C_compute_shader |
    Shader::C_texture_query_levels |
    Shader::C_enhanced_layouts |
    Shader::C_derivative_control |
    Shader::C_texture_query_samples
  );
#endif

  spew_module();
}

/**
 *
 */
void ShaderModuleDXBC::
spew_module() {
  shaderpipeline_cat.info()
    << "ShaderModuleDXBC:\n";
  shaderpipeline_cat.info()
    << "Stage " << _stage << "\n";
  shaderpipeline_cat.info()
    << _inputs.size() << " inputs\n";
  for (const Variable &input : _inputs) {
    shaderpipeline_cat.info()
      << "\tName: " << input.name->get_name() << "\n";
    shaderpipeline_cat.info()
      << "\tLocation: " << input._location << "\n";
    shaderpipeline_cat.info()
      << "\tType: " << *input.type << "\n";
  }
  shaderpipeline_cat.info()
    << _outputs.size() << " outputs\n";
  for (const Variable &output : _outputs) {
    shaderpipeline_cat.info()
      << "\tName: " << output.name->get_name() << "\n";
    shaderpipeline_cat.info()
      << "\tLocation: " << output._location << "\n";
    shaderpipeline_cat.info()
      << "\tType: " << *output.type << "\n";
  }
  shaderpipeline_cat.info()
    << _parameters.size() << " parameters\n";
  for (const Variable &param : _parameters) {
    shaderpipeline_cat.info()
      << "\tName: " << param.name->get_name() << "\n";
    shaderpipeline_cat.info()
      << "\tLocation: " << param._location << "\n";
    shaderpipeline_cat.info()
      << "\tType: " << *param.type << "\n";
  }
}

/**
 *
 */
ShaderModuleDXBC::
ShaderModuleDXBC(const ShaderModuleDXBC &copy) :
  ShaderModule(copy),
  _byte_code(copy._byte_code)
{
  _used_caps = copy._used_caps;
  if (_byte_code != nullptr) {
    _byte_code->AddRef();
  }
}

/**
 *
 */
ShaderModuleDXBC::
~ShaderModuleDXBC() {
  if (_byte_code != nullptr) {
    _byte_code->Release();
  }
}

/**
 *
 */
void ShaderModuleDXBC::
reflect_module() {
  if (_byte_code == nullptr) {
    return;
  }

#if 1
  ID3D11ShaderReflection *reflector = nullptr;
  HRESULT hr = D3DReflect(_byte_code->GetBufferPointer(), _byte_code->GetBufferSize(),
                          IID_ID3D11ShaderReflection, (void **)&reflector);
  nassertv(SUCCEEDED(hr) && reflector != nullptr);

  D3D11_SHADER_DESC desc;
  hr = reflector->GetDesc(&desc);
  nassertv(SUCCEEDED(hr));

  // Reflect input parameters.
  for (UINT i = 0; i < desc.InputParameters; ++i) {
    D3D11_SIGNATURE_PARAMETER_DESC pdesc;
    hr = reflector->GetInputParameterDesc(i, &pdesc);
    nassertv(SUCCEEDED(hr));

    Variable input;
    input._location = pdesc.Register;
    input.name = InternalName::make(pdesc.SemanticName);
    ShaderType::ScalarType scalar_type;
    switch (pdesc.ComponentType) {
    case D3D_REGISTER_COMPONENT_FLOAT32:
      scalar_type = ShaderType::ST_float;
      break;
    case D3D_REGISTER_COMPONENT_SINT32:
      scalar_type = ShaderType::ST_int;
      break;
    case D3D_REGISTER_COMPONENT_UINT32:
      scalar_type = ShaderType::ST_uint;
      break;
    default:
      scalar_type = ShaderType::ST_unknown;
      break;
    }
    input.type = ShaderType::register_type(ShaderType::Vector(scalar_type, 4));

    _inputs.push_back(std::move(input));
  }

  // Reflect output parameters.
  for (UINT i = 0; i < desc.OutputParameters; ++i) {
    D3D11_SIGNATURE_PARAMETER_DESC pdesc;
    hr = reflector->GetOutputParameterDesc(i, &pdesc);
    nassertv(SUCCEEDED(hr));

    Variable output;
    output._location = pdesc.Register;
    output.name = InternalName::make(pdesc.SemanticName);
    ShaderType::ScalarType scalar_type;
    switch (pdesc.ComponentType) {
    case D3D_REGISTER_COMPONENT_FLOAT32:
      scalar_type = ShaderType::ST_float;
      break;
    case D3D_REGISTER_COMPONENT_SINT32:
      scalar_type = ShaderType::ST_int;
      break;
    case D3D_REGISTER_COMPONENT_UINT32:
      scalar_type = ShaderType::ST_uint;
      break;
    default:
      scalar_type = ShaderType::ST_unknown;
      break;
    }
    output.type = ShaderType::register_type(ShaderType::Vector(scalar_type, 4));

    _outputs.push_back(std::move(output));
  }

  // Reflect uniform/constant parameters.
  for (UINT i = 0; i < desc.ConstantBuffers; ++i) {
    ID3D11ShaderReflectionConstantBuffer *crefl = nullptr;
    crefl = reflector->GetConstantBufferByIndex(i);
    nassertv(crefl != nullptr);

    D3D11_SHADER_BUFFER_DESC cdesc;
    crefl->GetDesc(&cdesc);

    int reg = -1;
    // Find the register of the constant buffer.
    for (UINT j = 0; j < desc.BoundResources; ++j) {
      D3D11_SHADER_INPUT_BIND_DESC ibdesc;
      reflector->GetResourceBindingDesc(j, &ibdesc);
      if (ibdesc.Type == D3D_SIT_CBUFFER) {
        if (!strcmp(ibdesc.Name, cdesc.Name)) {
          reg = (int)ibdesc.BindPoint;
          break;
        }
      }
    }

    ShaderType::Struct ctype;
    for (UINT j = 0; j < cdesc.Variables; ++j) {
      ID3D11ShaderReflectionVariable *vrefl = crefl->GetVariableByIndex(j);
      D3D11_SHADER_VARIABLE_DESC vdesc;
      vrefl->GetDesc(&vdesc);
      ID3D11ShaderReflectionType *trefl = vrefl->GetType();

      ctype.add_member(reflect_variable_type(trefl), vdesc.Name, vdesc.StartOffset);
    }

    nassertv(ctype.get_size_bytes() == cdesc.Size);

    Variable cvar;
    cvar.type = ShaderType::register_type(std::move(ctype));
    cvar.name = InternalName::make(cdesc.Name);
    cvar._location = reg;
    _parameters.push_back(std::move(cvar));
  }

#endif

}

/**
 * Converts a scalar D3D shader variable type to a ShaderType::ScalarType
 * enum value.
 */
static ShaderType::ScalarType
d3d_svt_to_scalar_type(D3D_SHADER_VARIABLE_TYPE type) {
  switch (type) {
  case D3D_SVT_BOOL:
    return ShaderType::ST_bool;
  case D3D_SVT_FLOAT:
    return ShaderType::ST_float;
  case D3D_SVT_DOUBLE:
    return ShaderType::ST_double;
  case D3D_SVT_INT:
    return ShaderType::ST_int;
  case D3D_SVT_UINT:
  case D3D_SVT_UINT8:
    return ShaderType::ST_uint;
  default:
    return ShaderType::ST_unknown;
  }
}

/**
 *
 */
const ShaderType *ShaderModuleDXBC::
reflect_variable_type(ID3D11ShaderReflectionType *trefl) {
  D3D11_SHADER_TYPE_DESC desc;
  trefl->GetDesc(&desc);

  switch (desc.Class) {
  case D3D_SVC_SCALAR:
    // Single scalar value.
    return ShaderType::register_type(ShaderType::Scalar(
      d3d_svt_to_scalar_type(desc.Type)));

  case D3D_SVC_VECTOR:
    {
      ShaderType::ScalarType scalar_type = d3d_svt_to_scalar_type(desc.Type);
      nassertr(scalar_type != ShaderType::ST_unknown && scalar_type != ShaderType::ST_bool, nullptr);
      return ShaderType::register_type(ShaderType::Vector(scalar_type, desc.Columns));
    }

  case D3D_SVC_MATRIX_COLUMNS:
  case D3D_SVC_MATRIX_ROWS:
    {
      ShaderType::ScalarType scalar_type = d3d_svt_to_scalar_type(desc.Type);
      nassertr(scalar_type != ShaderType::ST_unknown && scalar_type != ShaderType::ST_bool, nullptr);
      return ShaderType::register_type(ShaderType::Matrix(scalar_type, desc.Rows, desc.Columns));
    }

  case D3D_SVC_STRUCT:
    {
      ShaderType::Struct type;
      for (UINT i = 0; i < desc.Members; ++i) {
        D3D11_SHADER_TYPE_DESC mdesc;
        ID3D11ShaderReflectionType *mtype = trefl->GetMemberTypeByIndex(i);
        mtype->GetDesc(&mdesc);
        type.add_member(reflect_variable_type(mtype), trefl->GetMemberTypeName(i), mdesc.Offset);
      }
      return ShaderType::register_type(std::move(type));
    }

  case D3D_SVC_OBJECT:
    {
      switch (desc.Type) {
      case D3D_SVT_SAMPLER:
        return ShaderType::sampler_type;

      case D3D_SVT_TEXTURE1D:
        return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture, ShaderType::ST_float));
      case D3D_SVT_TEXTURE1DARRAY:
        return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture_array, ShaderType::ST_float));

      case D3D_SVT_TEXTURE:
      case D3D_SVT_TEXTURE2D:
        return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float));
      case D3D_SVT_TEXTURE2DARRAY:
        return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture_array, ShaderType::ST_float));

      case D3D_SVT_TEXTURE3D:
        return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_3d_texture, ShaderType::ST_float));

      //case D3D_SVT_
      }
    }

  }
}

/**
 *
 */
PT(CopyOnWriteObject) ShaderModuleDXBC::
make_cow_copy() {
  return new ShaderModuleDXBC(*this);
}

/**
 *
 */
std::string ShaderModuleDXBC::
get_ir() const {
  return std::string();
}

/**
 *
 */
void ShaderModuleDXBC::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void ShaderModuleDXBC::
write_datagram(BamWriter *manager, Datagram &me) {
  ShaderModule::write_datagram(manager, me);

  if (_byte_code != nullptr) {
    me.add_uint32(_byte_code->GetBufferSize());
    me.append_data(_byte_code->GetBufferPointer(), _byte_code->GetBufferSize());
  } else {
    me.add_uint32(0);
  }
}

/**
 *
 */
TypedWritable *ShaderModuleDXBC::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  Stage stage = (Stage)scan.get_uint8();
  ShaderModuleDXBC *mod = new ShaderModuleDXBC(stage);
  mod->fillin(scan, manager);

  return mod;
}

/**
 *
 */
void ShaderModuleDXBC::
fillin(DatagramIterator &scan, BamReader *manager) {
  _source_filename = scan.get_string();
  _used_caps = (int)scan.get_uint64();

  size_t data_size = scan.get_uint32();
  if (data_size > 0u) {
    HRESULT hr = D3DCreateBlob(data_size, &_byte_code);
    nassertv(SUCCEEDED(hr));
    nassertv(_byte_code != nullptr);
    size_t bytes_extracted = scan.extract_bytes((unsigned char *)_byte_code->GetBufferPointer(), data_size);
    nassertv(bytes_extracted == data_size);
  }
}

#endif
