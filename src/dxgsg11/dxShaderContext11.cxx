/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxShaderContext11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxShaderContext11.h"
#include "shaderModuleDXBC.h"
#include "config_dxgsg11.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexColumn.h"

TypeHandle DXShaderContext11::_type_handle;

/**
 *
 */
DXShaderContext11::
DXShaderContext11(Shader *shader, DXGraphicsStateGuardian11 *gsg) :
  ShaderContext(shader),
  _device(gsg->get_device()),
  _vshader(nullptr),
  _gshader(nullptr),
  _pshader(nullptr)
{
  nassertv(shader->get_language() == Shader::SL_HLSL);

  ID3D11Device *device = gsg->get_device();
  ID3D11DeviceContext *context = gsg->get_context();

  // Create a D3D shader for each module that we have.
  for (const Shader::LinkedModule &lmodule : shader->_modules) {
    const ShaderModule *module = lmodule._module.get_read_pointer();

    // Expect a DirectX Byte Code shader module.  In the future we could
    // transpile SPIR-V shader modules into HLSL and create a
    // DXBC shader module on-the-fly.
    nassertv(module->is_exact_type(ShaderModuleDXBC::get_class_type()));

    const ShaderModuleDXBC *dx_module = DCAST(ShaderModuleDXBC, module);
    ID3DBlob *module_blob = dx_module->get_byte_code();
    const void *byte_code = module_blob->GetBufferPointer();
    size_t byte_code_size = module_blob->GetBufferSize();

    switch (dx_module->get_stage()) {
    case ShaderModule::Stage::vertex:
      {
        HRESULT hr = device->CreateVertexShader(byte_code, byte_code_size, nullptr, &_vshader);
        nassertv(SUCCEEDED(hr) && _vshader != nullptr);
      }
      break;
    case ShaderModule::Stage::geometry:
      {
        HRESULT hr = device->CreateGeometryShader(byte_code, byte_code_size, nullptr, &_gshader);
        nassertv(SUCCEEDED(hr) && _gshader != nullptr);
      }
      break;
    case ShaderModule::Stage::fragment:
      {
        HRESULT hr = device->CreatePixelShader(byte_code, byte_code_size, nullptr, &_pshader);
        nassertv(SUCCEEDED(hr) && _pshader != nullptr);
      }
      break;
    default:
      break;
    }
  }
}

/**
 * Converts a NumericType and number of components to a DXGI_FORMAT enum value.
 *
 * If normalized is true, returns one of the _SNORM/_UNORM types for 16-bit and
 * 8-bit signed/unsigned integer types.
 *
 * Note that there are no DXGI_FORMAT_R8B8G8_* or DXGI_FORMAT_R16B16G16_* formats.
 *
 * Furthermore, NT_float64 is not supported.
 */
static DXGI_FORMAT
numeric_type_to_dx_format(GeomEnums::NumericType numeric_type, int num_components, bool normalized) {
  switch (num_components) {
  case 1:
    switch (numeric_type) {
    case GeomEnums::NT_float32:
      return DXGI_FORMAT_R32_FLOAT;

    case GeomEnums::NT_stdfloat:
#ifdef STDFLOAT_DOUBLE
      return DXGI_FORMAT_UNKNOWN;
#else
      return DXGI_FORMAT_R32_FLOAT;
#endif

    case GeomEnums::NT_int16:
      if (normalized) {
        return DXGI_FORMAT_R16_SNORM;
      } else {
        return DXGI_FORMAT_R16_SINT;
      }
    case GeomEnums::NT_uint16:
      if (normalized) {
        return DXGI_FORMAT_R16_UNORM;
      } else {
        return DXGI_FORMAT_R16_UINT;
      }

    case GeomEnums::NT_int32:
      return DXGI_FORMAT_R32_SINT;
    case GeomEnums::NT_uint32:
      return DXGI_FORMAT_R32_UINT;

    case GeomEnums::NT_int8:
      if (normalized) {
        return DXGI_FORMAT_R8_SNORM;
      } else {
        return DXGI_FORMAT_R8_SINT;
      }
    case GeomEnums::NT_uint8:
      if (normalized) {
        return DXGI_FORMAT_R8_UNORM;
      } else {
        return DXGI_FORMAT_R8_UINT;
      }

    default:
      return DXGI_FORMAT_UNKNOWN;
    }

  case 2:
    switch (numeric_type) {
    case GeomEnums::NT_float32:
      return DXGI_FORMAT_R32G32_FLOAT;

    case GeomEnums::NT_stdfloat:
#ifdef STDFLOAT_DOUBLE
      return DXGI_FORMAT_UNKNOWN;
#else
      return DXGI_FORMAT_R32G32_FLOAT;
#endif

    case GeomEnums::NT_int16:
      if (normalized) {
        return DXGI_FORMAT_R16G16_SNORM;
      } else {
        return DXGI_FORMAT_R16G16_SINT;
      }
    case GeomEnums::NT_uint16:
      if (normalized) {
        return DXGI_FORMAT_R16G16_UNORM;
      } else {
        return DXGI_FORMAT_R16G16_UINT;
      }

    case GeomEnums::NT_int32:
      return DXGI_FORMAT_R32G32_SINT;
    case GeomEnums::NT_uint32:
      return DXGI_FORMAT_R32G32_UINT;

    case GeomEnums::NT_int8:
      if (normalized) {
        return DXGI_FORMAT_R8G8_SNORM;
      } else {
        return DXGI_FORMAT_R8G8_SINT;
      }
    case GeomEnums::NT_uint8:
      if (normalized) {
        return DXGI_FORMAT_R8G8_UNORM;
      } else {
        return DXGI_FORMAT_R8G8_UINT;
      }

    default:
      return DXGI_FORMAT_UNKNOWN;
    }

  case 3:
    switch (numeric_type) {
    case GeomEnums::NT_float32:
      return DXGI_FORMAT_R32G32B32_FLOAT;

    case GeomEnums::NT_stdfloat:
#ifdef STDFLOAT_DOUBLE
      return DXGI_FORMAT_UNKNOWN;
#else
      return DXGI_FORMAT_R32G32B32_FLOAT;
#endif

    case GeomEnums::NT_int16:
      return DXGI_FORMAT_UNKNOWN;

    case GeomEnums::NT_uint16:
      return DXGI_FORMAT_UNKNOWN;

    case GeomEnums::NT_int32:
      return DXGI_FORMAT_R32G32B32_SINT;
    case GeomEnums::NT_uint32:
      return DXGI_FORMAT_R32G32B32_UINT;

    case GeomEnums::NT_int8:
      return DXGI_FORMAT_UNKNOWN;

    case GeomEnums::NT_uint8:
      return DXGI_FORMAT_UNKNOWN;

    default:
      return DXGI_FORMAT_UNKNOWN;
    }

  case 4:
    switch (numeric_type) {
    case GeomEnums::NT_float32:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case GeomEnums::NT_stdfloat:
#ifdef STDFLOAT_DOUBLE
      return DXGI_FORMAT_UNKNOWN;
#else
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
#endif

    case GeomEnums::NT_int16:
      if (normalized) {
        return DXGI_FORMAT_R16G16B16A16_SNORM;
      } else {
        return DXGI_FORMAT_R16G16B16A16_SINT;
      }
    case GeomEnums::NT_uint16:
      if (normalized) {
        return DXGI_FORMAT_R16G16B16A16_UNORM;
      } else {
        return DXGI_FORMAT_R16G16B16A16_UINT;
      }

    case GeomEnums::NT_int32:
      return DXGI_FORMAT_R32G32B32A32_SINT;
    case GeomEnums::NT_uint32:
      return DXGI_FORMAT_R32G32B32A32_UINT;

    case GeomEnums::NT_int8:
      if (normalized) {
        return DXGI_FORMAT_R8G8B8A8_SNORM;
      } else {
        return DXGI_FORMAT_R8G8B8A8_SINT;
      }
    case GeomEnums::NT_uint8:
      if (normalized) {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
      } else {
        return DXGI_FORMAT_R8G8B8A8_UINT;
      }

    default:
      return DXGI_FORMAT_UNKNOWN;
    }

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

/**
 * Returns a D3D input layout object suitable for rendering this shader
 * with the indicated GeomVertexFormat.
 */
ID3D11InputLayout *DXShaderContext11::
get_input_layout(const GeomVertexFormat *format, BitMask32 &enabled_arrays) {
  auto it = _input_layouts.find(format);
  if (it != _input_layouts.end()) {
    enabled_arrays = (*it).second._enabled_arrays;
    return (*it).second._layout;
  }

  pvector<D3D11_INPUT_ELEMENT_DESC> inputs;
  inputs.reserve(_shader->_var_spec.size());

  vector_string names;
  names.resize(_shader->_var_spec.size());

  // Go through all the vertex inputs.
  int curr_offset = 0;
  for (size_t i = 0; i < _shader->_var_spec.size(); ++i) {
    const Shader::ShaderVarSpec &vspec = _shader->_var_spec[i];

    D3D11_INPUT_ELEMENT_DESC idesc;

    names[i] = vspec._name->get_name();

    idesc.SemanticName = names[i].c_str();
    idesc.SemanticIndex = 0;
    idesc.InstanceDataStepRate = 0;

    // Find the vertex array and column that matches this input.
    const GeomVertexColumn *column;
    int array_index;
    if (format->get_array_info(vspec._name, array_index, column)) {
      // Great!  The vertex format has this column in one of its arrays.
      idesc.InputSlot = array_index;
      enabled_arrays.set_bit(array_index);
      idesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

      idesc.Format = numeric_type_to_dx_format(column->get_numeric_type(), column->get_num_components(),
                                               column->get_contents() == GeomEnums::C_color);
      idesc.AlignedByteOffset = column->get_start();
      curr_offset += (int)idesc.AlignedByteOffset;

    } else {
      // Don't have it, but the shader needs something for it.
      idesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
      idesc.InputSlot = 15;
      idesc.Format = DXGI_FORMAT_UNKNOWN;
    }

    inputs.push_back(idesc);
  }

  // Fix-up fallback element byte offsets.
  for (size_t i = 0; i < _shader->_var_spec.size(); ++i) {
    D3D11_INPUT_ELEMENT_DESC &idesc = inputs[i];
    const Shader::ShaderVarSpec &vspec = _shader->_var_spec[i];

    if (idesc.InputSlotClass == D3D11_INPUT_PER_INSTANCE_DATA) {
      idesc.AlignedByteOffset = curr_offset;
      curr_offset += vspec._id._type->get_size_bytes();
    }
  }

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Input layout desc:\n";
    for (const D3D11_INPUT_ELEMENT_DESC &idesc : inputs) {
      dxgsg11_cat.debug()
        << "\tSemantic name: " << idesc.SemanticName << "\n";
      dxgsg11_cat.debug()
        << "\tSemantic index: " << idesc.SemanticIndex << "\n";
      dxgsg11_cat.debug()
        << "\tInputSlot: " << idesc.InputSlot << "\n";
      dxgsg11_cat.debug()
        << "\tInputSlotClass: " << idesc.InputSlotClass << "\n";
      dxgsg11_cat.debug()
        << "\tAlignedByteOffset: " << idesc.AlignedByteOffset << "\n";
      dxgsg11_cat.debug()
        << "\tFormat: " << idesc.Format << "\n";
    }
  }

  ID3DBlob *vshader_blob = nullptr;
  for (const Shader::LinkedModule &lmodule : _shader->_modules) {
    if (lmodule._module.get_read_pointer()->get_stage() == ShaderModule::Stage::vertex) {
      vshader_blob = DCAST(ShaderModuleDXBC, lmodule._module.get_read_pointer())->get_byte_code();
      break;
    }
  }
  assert(vshader_blob != nullptr);
  const void *vshader_byte_code = vshader_blob->GetBufferPointer();
  size_t vshader_byte_code_size = vshader_blob->GetBufferSize();

  ID3D11InputLayout *layout = nullptr;
  HRESULT hr = _device->CreateInputLayout(inputs.data(), inputs.size(),
    vshader_byte_code, vshader_byte_code_size, &layout);

  if (FAILED(hr) || layout == nullptr) {
    dxgsg11_cat.error()
      << "Failed to create input layout for vtx format " << format << " and shader " << this << ", hr = " << hr << "\n";
  }

  // Cache it for next time.
  InputLayout il;
  il._enabled_arrays = enabled_arrays;
  il._layout = layout;
  _input_layouts[format] = std::move(il);

  return layout;
}

/**
 *
 */
DXShaderContext11::
~DXShaderContext11() {
  if (_vshader != nullptr) {
    _vshader->Release();
  }
  if (_gshader != nullptr) {
    _gshader->Release();
  }
  if (_pshader != nullptr) {
    _pshader->Release();
  }

  for (auto it = _input_layouts.begin(); it != _input_layouts.end(); ++it) {
    if ((*it).second._layout != nullptr) {
      (*it).second._layout->Release();
    }
  }
}
