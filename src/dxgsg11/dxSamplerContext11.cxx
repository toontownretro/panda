/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxSamplerContext11.cxx
 * @author brian
 * @date 2022-03-05
 */

#include "dxSamplerContext11.h"
#include "dxGraphicsStateGuardian11.h"

TypeHandle DXSamplerContext11::_type_handle;

/**
 *
 */
static D3D11_FILTER
filter_mode_to_d3d_filter(SamplerState::FilterType minfilter, SamplerState::FilterType magfilter) {
  switch (minfilter) {
  case SamplerState::FT_nearest:
  case SamplerState::FT_nearest_mipmap_nearest:
    switch (magfilter) {
    case SamplerState::FT_nearest:
      return D3D11_FILTER_MIN_MAG_MIP_POINT;
    case SamplerState::FT_linear:
    default:
      return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    }

  case SamplerState::FT_nearest_mipmap_linear:
    switch (magfilter) {
    case SamplerState::FT_nearest:
      return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    case SamplerState::FT_linear:
    default:
      return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
    }

  case SamplerState::FT_linear:
  case SamplerState::FT_linear_mipmap_nearest:
  default:
    switch (magfilter) {
    case SamplerState::FT_nearest:
      return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    case SamplerState::FT_linear:
    default:
      return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    }

  case SamplerState::FT_linear_mipmap_linear:
    switch (magfilter) {
    case SamplerState::FT_nearest:
      return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case SamplerState::FT_linear:
    default:
      return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
  }
}

/**
 *
 */
static D3D11_TEXTURE_ADDRESS_MODE
wrap_mode_to_d3d_address_mode(SamplerState::WrapMode mode) {
  switch (mode) {
  case SamplerState::WM_repeat:
    return D3D11_TEXTURE_ADDRESS_WRAP;
  case SamplerState::WM_mirror:
    return D3D11_TEXTURE_ADDRESS_MIRROR;
  case SamplerState::WM_clamp:
    return D3D11_TEXTURE_ADDRESS_CLAMP;
  case SamplerState::WM_border_color:
    return D3D11_TEXTURE_ADDRESS_BORDER;
  case SamplerState::WM_mirror_once:
    return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
  default:
    return D3D11_TEXTURE_ADDRESS_WRAP;
  }
}

/**
 *
 */
DXSamplerContext11::
DXSamplerContext11(const SamplerState &sampler, DXGraphicsStateGuardian11 *gsg) :
  SamplerContext(sampler),
  _sampler_state(nullptr)
{
  ID3D11Device *device = gsg->get_device();

  D3D11_SAMPLER_DESC desc;

  LColorf border_color = LCAST(float, sampler.get_border_color());
  desc.BorderColor[0] = border_color[0];
  desc.BorderColor[1] = border_color[1];
  desc.BorderColor[2] = border_color[2];
  desc.BorderColor[3] = border_color[3];
  desc.MinLOD = sampler.get_min_lod();
  desc.MaxLOD = sampler.get_max_lod();
  desc.MaxAnisotropy = sampler.get_effective_anisotropic_degree();
  desc.MipLODBias = sampler.get_lod_bias();
  if (desc.MaxAnisotropy > 1) {
    desc.Filter = D3D11_FILTER_ANISOTROPIC;
  } else {
    desc.Filter = filter_mode_to_d3d_filter(sampler.get_effective_minfilter(), sampler.get_effective_magfilter());
  }
  desc.AddressU = wrap_mode_to_d3d_address_mode(sampler.get_wrap_u());
  desc.AddressV = wrap_mode_to_d3d_address_mode(sampler.get_wrap_v());
  desc.AddressW = wrap_mode_to_d3d_address_mode(sampler.get_wrap_w());

  HRESULT hr = device->CreateSamplerState(&desc, &_sampler_state);
  nassertv(SUCCEEDED(hr) && _sampler_state != nullptr);
}

/**
 *
 */
DXSamplerContext11::
~DXSamplerContext11() {
  if (_sampler_state != nullptr) {
    _sampler_state->Release();
  }
}
