/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxTextureContext11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxTextureContext11.h"
#include "dxGraphicsStateGuardian11.h"
#include "texture.h"
#include "config_dxgsg11.h"

TypeHandle DXTextureContext11::_type_handle;

/**
 *
 */
DXTextureContext11::
DXTextureContext11(PreparedGraphicsObjects *pgo, DXGraphicsStateGuardian11 *gsg, Texture *tex, int view) :
  TextureContext(pgo, tex, view),
  _texture(tex),
  _device(gsg->get_device()),
  _d3d_tex(nullptr),
  _d3d_rtv(nullptr),
  _d3d_srv(nullptr),
  _d3d_dsv(nullptr)
{
  compute_d3d_format_and_ram_swizzle();
  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "D3D format " << _actual_d3d_format << " swizzle string "
      << _ram_image_swizzle << " num components " << _actual_num_components << "\n";
  }
  create_texture();
  create_srv();
}

/**
 *
 */
DXTextureContext11::
~DXTextureContext11() {
  if (_d3d_tex != nullptr) {
    _d3d_tex->Release();
  }
  if (_d3d_srv != nullptr) {
    _d3d_srv->Release();
  }
  if (_d3d_rtv != nullptr) {
    _d3d_rtv->Release();
  }
  if (_d3d_dsv != nullptr) {
    _d3d_dsv->Release();
  }
}

// NOTE: D3D11 has no 24bpp image formats!!!
// RGB8 textures have to be expanded to RGBA8.

/**
 * Returns the closest available DXGI_FORMAT for the given Texture that does
 * not have an exact DXGI_FORMAT equivalent, and a swizzle string to convert
 * the RAM image into the appropriate DXGI_FORMAT.
 */
DXGI_FORMAT
get_closest_dxgi_format_and_swizzle(const Texture *tex, std::string &swizzle, int &num_components) {
  Texture::Format format = tex->get_format();
  bool is_signed = !Texture::is_unsigned(tex->get_component_type());

  switch (format) {
  case Texture::F_green:
    // One channel image that represents the green channel only.
    // Make it a two channel 8bpc texture, with red zeroed and
    // the original channel moved into green.
    swizzle = "0R";
    num_components = 2;
    return is_signed ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_UNORM;
  case Texture::F_blue:
    // One channel image that represents the blue channel only.
    // Move original into blue channel and zero the others, with
    // opaque alpha.
    swizzle = "00R1";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
  case Texture::F_rgb:
  case Texture::F_rgb8:
    // There are no 24bpp formats in D3D11.  So expand to a 32bpp texture with
    // opaque alpha.
    swizzle = "BGR1";
    num_components = 4;
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case Texture::F_rgb12:
    // Implemented as 16bpc under the hood.
    swizzle = "RGB1";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_UNORM;
  case Texture::F_luminance:
    // One channel "grayscale" image.  Duplicate red channel to RGB with opaque
    // alpha.
    swizzle = "RRR1";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
  case Texture::F_luminance_alpha:
  case Texture::F_luminance_alphamask:
    // Grayscale image with an alpha channel.  Duplicate red channel to RGB.
    swizzle = "RRRA";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
  case Texture::F_rgb16:
    // Expand to RGBA.
    swizzle = "RGB1";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_UNORM;
  case Texture::F_srgb:
    swizzle = "BGR1";
    num_components = 4;
    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
  case Texture::F_sluminance:
    swizzle = "RRR1";
    num_components = 4;
    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  case Texture::F_sluminance_alpha:
    swizzle = "RRRA";
    num_components = 4;
    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  case Texture::F_rgb8i:
    swizzle = "RGB1";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R8G8B8A8_SINT : DXGI_FORMAT_R8G8B8A8_UINT;
  case Texture::F_rgba8i:
    swizzle = "RGBA";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R8G8B8A8_SINT : DXGI_FORMAT_R8G8B8A8_UINT;
  case Texture::F_rgb16i:
    swizzle = "RGB1";
    num_components = 4;
    return is_signed ? DXGI_FORMAT_R16G16B16A16_SINT : DXGI_FORMAT_R16G16B16A16_UINT;

  default:
    // No possible conversion into a DXGI_FORMAT.
    // Texture cannot be used.
    swizzle = std::string();
    num_components = 0;
    return DXGI_FORMAT_UNKNOWN;
  }
}

/**
 * Returns the DXGI_FORMAT that exactly matches the given Texture's format,
 * component type, and compression mode.
 *
 * If DXGI_FORMAT_UNKNOWN is returned, there is no exact equivalent, and
 * get_closest_dxgi_format_and_swizzle() should be called to get the closest
 * DXGI_FORMAT to the Texture and a swizzle string to convert the RAM image
 * into the appropriate format for uploading to D3D.  If DXGI_FORMAT_UNKNOWN
 * is returned from *that*, the Texture cannot be used.
 */
DXGI_FORMAT
texture_format_to_dxgi_format(const Texture *tex) {
  Texture::Format format = tex->get_format();
  Texture::ComponentType component_type = tex->get_component_type();
  Texture::CompressionMode compression = tex->get_compression();
  if (compression == Texture::CM_default) {
    compression = compression ? Texture::CM_on : Texture::CM_off;
  }

  int num_components = tex->get_num_components();

  bool is_signed = !Texture::is_unsigned(component_type);
  bool is_srgb = Texture::is_srgb(format);

  // If compression enabled on texture, choose a compressed format.
  switch (compression) {
  case Texture::CM_on:
    if (!is_srgb && num_components == 1) {
      return is_signed ? DXGI_FORMAT_BC4_SNORM : DXGI_FORMAT_BC4_UNORM;

    } else if (!is_srgb && num_components == 2) {
      return is_signed ? DXGI_FORMAT_BC5_SNORM : DXGI_FORMAT_BC5_UNORM;

    } else if (Texture::has_binary_alpha(format)) {
      return is_srgb ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;

    } else if (format == Texture::F_rgba4 || format == Texture::F_rgb10_a2) {
      return is_srgb ? DXGI_FORMAT_BC2_UNORM_SRGB : DXGI_FORMAT_BC2_UNORM;

    } else if (Texture::has_alpha(format)) {
      return is_srgb ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;

    } else {
      return is_srgb ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
    }
  case Texture::CM_dxt1:
    return is_srgb ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
  case Texture::CM_dxt3:
    return is_srgb ? DXGI_FORMAT_BC2_UNORM_SRGB : DXGI_FORMAT_BC2_UNORM;
  case Texture::CM_dxt5:
    return is_srgb ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
  case Texture::CM_rgtc:
    if (num_components == 1) {
      return is_signed ? DXGI_FORMAT_BC4_SNORM : DXGI_FORMAT_BC4_UNORM;
    } else {
      return is_signed ? DXGI_FORMAT_BC5_SNORM : DXGI_FORMAT_BC5_UNORM;
    }
  default:
    // Compression mode not supported.
    break;
  }

  switch (format) {
  case Texture::F_depth_stencil:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case Texture::F_color_index:
    return DXGI_FORMAT_R8_UINT;
  case Texture::F_red:
    return is_signed ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_UNORM;
  case Texture::F_green:
    // We can expand this to rg8.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_blue:
    // We can expand this to rgba8.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_alpha:
    return DXGI_FORMAT_A8_UNORM;
  case Texture::F_rgb:
    // We can expand this to rgba8.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_rgb5:
    return DXGI_FORMAT_B5G6R5_UNORM;
  case Texture::F_rgb8:
    // We can expand this to rgba8.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_rgb12:
    // We can expand this to rgba16.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_rgb332:
    return DXGI_FORMAT_B5G6R5_UNORM;
  case Texture::F_rgba:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case Texture::F_rgbm:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case Texture::F_rgba4:
    return DXGI_FORMAT_B4G4R4A4_UNORM;
  case Texture::F_rgba5:
    return DXGI_FORMAT_B5G5R5A1_UNORM;
  case Texture::F_rgba8:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case Texture::F_rgba12:
    return is_signed ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_UNORM;
  case Texture::F_luminance:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_luminance_alpha:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_luminance_alphamask:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_rgba16:
    return is_signed ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_UNORM;
  case Texture::F_rgba32:
    return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case Texture::F_depth_component:
    return DXGI_FORMAT_D16_UNORM;
  case Texture::F_depth_component16:
    return DXGI_FORMAT_D16_UNORM;
  case Texture::F_depth_component24:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case Texture::F_depth_component32:
    return DXGI_FORMAT_D32_FLOAT;
  case Texture::F_r16:
    return is_signed ? DXGI_FORMAT_R16_SNORM : DXGI_FORMAT_R16_UNORM;
  case Texture::F_rg16:
    return is_signed ? DXGI_FORMAT_R16G16_SNORM : DXGI_FORMAT_R16G16_UNORM;
  case Texture::F_rgb16:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_srgb:
    // Needs to be expanded to BGRA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_srgb_alpha:
    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
  case Texture::F_sluminance:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_sluminance_alpha:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_r32i:
    return is_signed ? DXGI_FORMAT_R32_SINT : DXGI_FORMAT_R32_UINT;
  case Texture::F_r32:
    return DXGI_FORMAT_R32_FLOAT;
  case Texture::F_rg32:
    return DXGI_FORMAT_R32G32_FLOAT;
  case Texture::F_rgb32:
    return DXGI_FORMAT_R32G32B32_FLOAT;
  case Texture::F_r8i:
    return is_signed ? DXGI_FORMAT_R8_SINT : DXGI_FORMAT_R8_UINT;
  case Texture::F_rg8i:
    return is_signed ? DXGI_FORMAT_R8G8_SINT : DXGI_FORMAT_R8G8_UINT;
  case Texture::F_rgb8i:
    // Needs to be swizzled from BGR to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_rgba8i:
    // Needs to be swizzled from BGRA to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_r11_g11_b10:
    return DXGI_FORMAT_R11G11B10_FLOAT;
  case Texture::F_rgb9_e5:
    return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
  case Texture::F_rgb10_a2:
    return DXGI_FORMAT_R10G10B10A2_UNORM;
  case Texture::F_rg:
    return is_signed ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_UNORM;
  case Texture::F_r16i:
    return is_signed ? DXGI_FORMAT_R16_SINT : DXGI_FORMAT_R16_UINT;
  case Texture::F_rg16i:
    return is_signed ? DXGI_FORMAT_R16G16_SINT : DXGI_FORMAT_R16G16_UINT;
  case Texture::F_rgb16i:
    // Needs to be expanded to RGBA.
    return DXGI_FORMAT_UNKNOWN;
  case Texture::F_rgba16i:
    return is_signed ? DXGI_FORMAT_R16G16B16A16_SINT : DXGI_FORMAT_R16G16B16A16_UINT;
  case Texture::F_rg32i:
    return is_signed ? DXGI_FORMAT_R32G32_SINT : DXGI_FORMAT_R32G32_UINT;
  case Texture::F_rgb32i:
    return is_signed ? DXGI_FORMAT_R32G32B32_SINT : DXGI_FORMAT_R32G32B32_UINT;
  case Texture::F_rgba32i:
    return is_signed ? DXGI_FORMAT_R32G32B32A32_SINT : DXGI_FORMAT_R32G32B32A32_UINT;

  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

/**
 * Creates a D3D11 texture object for the context's associated Panda Texture
 * object.
 *
 * Returns true on success, or false if the texture could not be created for
 * some reason.
 */
bool DXTextureContext11::
create_texture() {
  if (_actual_d3d_format == DXGI_FORMAT_UNKNOWN) {
    dxgsg11_cat.error()
      << "Cannot create D3D texture for Texture with unsupported format " << _texture->get_format() << "\n";
    return false;
  }

  switch (_texture->get_texture_type()) {
  case Texture::TT_1d_texture:
  case Texture::TT_1d_texture_array:
    return create_1d_texture();
    break;
  case Texture::TT_2d_texture:
  case Texture::TT_2d_texture_array:
  case Texture::TT_cube_map:
  case Texture::TT_cube_map_array:
    return create_2d_texture();
    break;
  case Texture::TT_3d_texture:
    return create_3d_texture();
    break;
  default:
    return false;
  }
}

/**
 *
 */
bool DXTextureContext11::
create_1d_texture() {
  D3D11_TEXTURE1D_DESC desc;
  desc.Width = _texture->get_x_size();
  desc.ArraySize = _texture->get_z_size();
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.MipLevels = _texture->get_expected_num_mipmap_levels();
  desc.Format = _actual_d3d_format;

  HRESULT hr = _device->CreateTexture1D(&desc, nullptr, (ID3D11Texture1D **)&_d3d_tex);
  return SUCCEEDED(hr);
}

/**
 *
 */
bool DXTextureContext11::
create_2d_texture() {
  D3D11_TEXTURE2D_DESC desc;
  desc.Width = _texture->get_x_size();
  desc.Height = _texture->get_y_size();
  desc.ArraySize = _texture->get_z_size();
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.MipLevels = 1;//_texture->get_expected_num_mipmap_levels();
  desc.Format = _actual_d3d_format;

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Creating 2D texture, " << desc.Width << "x" << desc.Height << "x" << desc.ArraySize << " d3d fmt " << _actual_d3d_format << "\n";
  }

  HRESULT hr = _device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D **)&_d3d_tex);
  if (FAILED(hr) || _d3d_tex == nullptr) {
    dxgsg11_cat.error()
      << "Failed to create D3D texture 2D, hr = " << hr << "\n";
  }
  return SUCCEEDED(hr);
}

/**
 *
 */
bool DXTextureContext11::
create_3d_texture() {
  D3D11_TEXTURE3D_DESC desc;
  desc.Width = _texture->get_x_size();
  desc.Height = _texture->get_y_size();
  desc.Depth = _texture->get_z_size();
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.MipLevels = _texture->get_expected_num_mipmap_levels();
  desc.Format = _actual_d3d_format;

  HRESULT hr = _device->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D **)&_d3d_tex);
  return SUCCEEDED(hr);
}

/**
 * Determines the DXGI_FORMAT to use for the associated Texture object.
 *
 * If there is no exact equivalent, determines the closest available format
 * and a swizzle string to convert the RAM image to the closest format
 * when uploading the texture to the graphics card.
 */
void DXTextureContext11::
compute_d3d_format_and_ram_swizzle() {
  _actual_num_components = _texture->get_num_components();
  _actual_d3d_format = texture_format_to_dxgi_format(_texture);
  if (_actual_d3d_format == DXGI_FORMAT_UNKNOWN) {
    _actual_d3d_format = get_closest_dxgi_format_and_swizzle(_texture, _ram_image_swizzle, _actual_num_components);
    _needs_swizzle = true;
  } else {
    _needs_swizzle = false;
  }
}

/**
 * Uploads the current RAM image of the texture into GPU memory.
 */
bool DXTextureContext11::
upload_texture(ID3D11DeviceContext *context) {
  if (_d3d_tex == nullptr) {
    return false;
  }

  CPTA_uchar ram_image;
  if (_needs_swizzle) {
    ram_image = _texture->get_ram_image_as(_ram_image_swizzle);
  } else {
    ram_image = _texture->get_ram_image();
  }
  if (ram_image == nullptr) {
    // No image to upload.
    return false;
  }

  int texel_stride = _texture->get_component_width() * _actual_num_components;
  int width = _texture->get_x_size();
  int height = _texture->get_y_size();

  nassertr(ram_image.size() == texel_stride * width * height, false);

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Uploading tex image, row pitch " << texel_stride * width << ", " << width << "x" << height << ", ram image " << (const void *)ram_image.p() << "\n";
  }

  D3D11_BOX box;
  box.left = 0;
  box.right = width;
  box.top = 0;
  box.bottom = height;
  box.back = 1;
  box.front = 0;
  nassertr(Thread::get_current_thread() == Thread::get_main_thread(), false);
  context->UpdateSubresource(_d3d_tex, D3D11CalcSubresource(0, 0, 1), &box, (const void *)ram_image.p(),
                             texel_stride * width, 0);

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Finished\n";
  }

  mark_loaded();

  return true;
}

/**
 * Creates a shader resource view for this texture so it can be used
 * as inputs to shaders.
 */
void DXTextureContext11::
create_srv() {
  if (_d3d_srv != nullptr || _d3d_tex == nullptr) {
    return;
  }

  D3D11_SRV_DIMENSION dim;
  switch (_texture->get_texture_type()) {
  case Texture::TT_1d_texture:
    dim = D3D11_SRV_DIMENSION_TEXTURE1D;
    break;
  case Texture::TT_1d_texture_array:
    dim = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
    break;
  case Texture::TT_2d_texture:
    dim = D3D11_SRV_DIMENSION_TEXTURE2D;
    break;
  case Texture::TT_2d_texture_array:
    dim = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    break;
  case Texture::TT_3d_texture:
    dim = D3D11_SRV_DIMENSION_TEXTURE3D;
    break;
  case Texture::TT_cube_map:
    dim = D3D11_SRV_DIMENSION_TEXTURECUBE;
    break;
  case Texture::TT_cube_map_array:
    dim = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
    break;
  default:
    dim = D3D11_SRV_DIMENSION_TEXTURE2D;
    break;
  }

  CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc(dim, _actual_d3d_format);
  HRESULT hr = _device->CreateShaderResourceView(_d3d_tex, &srv_desc, &_d3d_srv);
  nassertv(SUCCEEDED(hr) && _d3d_srv != nullptr);
}

/**
 * Creates a render target view for this texture so it can be used as a
 * render target output.
 */
void DXTextureContext11::
create_rtv() {
}

/**
 * Creates a depth-stencil view for this texture so it can be used as a
 * depth-stencil output.
 */
void DXTextureContext11::
create_dsv() {
}
