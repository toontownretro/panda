/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsStateGuardian11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxGraphicsStateGuardian11.h"
#include "coordinateSystem.h"
#include "dxGeomMunger11.h"
#include "dxVertexBufferContext11.h"
#include "dxIndexBufferContext11.h"
#include "dxGraphicsDevice11.h"
#include "dxShaderContext11.h"
#include "dxTextureContext11.h"
#include "dxSamplerContext11.h"
#include "clockObject.h"

// Rasterizer state attribs.
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "depthOffsetAttrib.h"
#include "depthBiasAttrib.h"
#include "scissorAttrib.h"
#include "antialiasAttrib.h"
// Blend state attribs.
#include "colorBlendAttrib.h"
#include "colorWriteAttrib.h"
#include "colorScaleAttrib.h"
// Depth-stencil state attribs.
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "stencilAttrib.h"

#include "config_dxgsg11.h"

#include <d3d11.h>

TypeHandle DXGraphicsStateGuardian11::_type_handle;

/**
 *
 */
DXGraphicsStateGuardian11::
DXGraphicsStateGuardian11(GraphicsEngine *engine, GraphicsPipe *pipe,
                          DXGraphicsDevice11 *device) :
  GraphicsStateGuardian(CS_default, engine, pipe),
  _pdevice(device),
  _device(device->get_device()),
  _context(device->get_context()),
  _current_shader_context(nullptr),
  _curr_input_layout(nullptr),
  _curr_vertex_shader(nullptr),
  _curr_geometry_shader(nullptr),
  _curr_pixel_shader(nullptr),
  _curr_index_buffer(nullptr),
  _curr_index_type(GeomEnums::NT_uint16),
  _curr_vertex_buffers{ nullptr },
  _curr_vertex_buffer_strides{ 0 },
  _curr_topology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED),
  _instance_count(1),
  _curr_render_target_view(nullptr),
  _curr_depth_stencil_view(nullptr),
  _temp_cbuffer(nullptr),
  _temp_cbuffer_data(nullptr),
  _temp_cbuffer_stale(false),
  _curr_tex(nullptr),
  _curr_sampler(nullptr)
{
  _curr_pipeline_states._rasterizer = nullptr;
  _curr_pipeline_states._blend = nullptr;
  _curr_pipeline_states._depth_stencil = nullptr;
  _curr_pipeline_states._blend_factor.set(1.0f, 1.0f, 1.0f, 1.0f);

  determine_capabilities();
}

/**
 *
 */
void DXGraphicsStateGuardian11::
reset() {
  GraphicsStateGuardian::reset();

  _current_shader_context = nullptr;
  _curr_input_layout = nullptr;
  _curr_vertex_shader = nullptr;
  _curr_geometry_shader = nullptr;
  _curr_pixel_shader = nullptr;
  _curr_index_buffer = nullptr;
  _curr_index_type = GeomEnums::NT_uint16;
  memset(_curr_vertex_buffers, 0, sizeof(_curr_vertex_buffers));
  memset(_curr_vertex_buffer_strides, 0, sizeof(_curr_vertex_buffer_strides));
  _curr_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
  _instance_count = 1;
  _curr_pipeline_states._rasterizer = nullptr;
  _curr_pipeline_states._blend = nullptr;
  _curr_pipeline_states._depth_stencil = nullptr;
  _curr_pipeline_states._blend_factor.set(1.0f, 1.0f, 1.0f, 1.0f);

  _curr_render_target_view = nullptr;
  _curr_depth_stencil_view = nullptr;

  _curr_tex = nullptr;
  _curr_sampler = nullptr;

  if (_temp_cbuffer != nullptr) {
    _temp_cbuffer->Release();
  }
  if (_temp_cbuffer_data != nullptr) {
    delete[] _temp_cbuffer_data;
  }

  determine_capabilities();

  _temp_cbuffer_data = new LMatrix4[2];
  for (int i = 0; i < 2; ++i) {
    _temp_cbuffer_data[i] = LMatrix4::ident_mat();
  }

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = _temp_cbuffer_data;
  data.SysMemPitch = 0;
  data.SysMemSlicePitch = 0;
  CD3D11_BUFFER_DESC cdesc(sizeof(LMatrix4) * 2, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
  HRESULT hr = _device->CreateBuffer(&cdesc, &data, &_temp_cbuffer);
  nassertv(SUCCEEDED(hr) && _temp_cbuffer != nullptr);

  _temp_cbuffer_stale = false;
}

/**
 * Determines the capabilities of the graphics driver/device based on the
 * reported D3D feature level of the device.
 */
void DXGraphicsStateGuardian11::
determine_capabilities() {
  _supports_2d_texture_array = true;
  _supports_3d_texture = true;
  _supports_buffer_texture = true;
  _supports_cube_map = true;
  _supports_cube_map_array = true;
  _supports_compressed_texture = true;
  _supports_depth_stencil = true;
  _supports_depth_texture = true;
  _supports_hlsl = true;
  _supports_glsl = false;
  _supports_dual_source_blending = true;
  _supports_framebuffer_blit = true;
  _supports_framebuffer_multisample = true;
  _supports_generate_mipmap = true;
  _supports_geometry_instancing = true;
  _supports_indirect_draw = true;
  _supports_luminance_texture = true;
  _supports_multisample = true;
  _supports_occlusion_query = true;
  _supports_sampler_objects = true;
  _supports_shadow_filter = true;
  _supports_spir_v = true;
  _supports_stencil = true;
  _supports_stencil_wrap = true;
  _supports_tex_non_pow2 = true;
  _supports_texture_combine = true;
  _supports_texture_dot3 = true;
  _supports_texture_saved_result = true;
  _supports_texture_srgb = true;
  _supports_timer_query = true;
  _supports_two_sided_stencil = true;

  _shader_model = SM_50;

#if 0 // FIXME if/when D3D11 is returned to.
  _supported_shader_caps = (
    ShaderModule::C_basic_shader |
    ShaderModule::C_vertex_texture |
    ShaderModule::C_sampler_shadow |
    ShaderModule::C_invariant |
    ShaderModule::C_matrix_non_square |
    ShaderModule::C_integer |
    ShaderModule::C_texture_lod |
    ShaderModule::C_texture_fetch |
    ShaderModule::C_sampler_cube_shadow |
    ShaderModule::C_vertex_id |
    ShaderModule::C_round_even |
    ShaderModule::C_instance_id |
    ShaderModule::C_buffer_texture |
    ShaderModule::C_geometry_shader |
    ShaderModule::C_primitive_id |
    ShaderModule::C_bit_encoding |
    ShaderModule::C_texture_gather |
    ShaderModule::C_double |
    ShaderModule::C_cube_map_array |
    ShaderModule::C_tessellation_shader |
    ShaderModule::C_sample_variables |
    ShaderModule::C_extended_arithmetic |
    ShaderModule::C_texture_query_lod |
    ShaderModule::C_image_load_store |
    ShaderModule::C_compute_shader |
    ShaderModule::C_texture_query_levels |
    ShaderModule::C_enhanced_layouts |
    ShaderModule::C_derivative_control |
    ShaderModule::C_texture_query_samples
  );
#endif

  _supported_geom_rendering = (
    GeomEnums::GR_indexed_point |
    GeomEnums::GR_indexed_other |
    GeomEnums::GR_indexed_bits |
    GeomEnums::GR_point |
    GeomEnums::GR_point_uniform_size |
    GeomEnums::GR_per_point_size |
    GeomEnums::GR_point_perspective |
    GeomEnums::GR_point_aspect_ratio |
    GeomEnums::GR_point_scale |
    GeomEnums::GR_point_rotate |
    GeomEnums::GR_point_sprite_tex_matrix |
    GeomEnums::GR_point_bits |
    GeomEnums::GR_triangle_strip |
    GeomEnums::GR_line_strip |
    GeomEnums::GR_strip_cut_index |
    GeomEnums::GR_shade_model_bits |
    GeomEnums::GR_render_mode_wireframe |
    GeomEnums::GR_render_mode_point |
    GeomEnums::GR_adjacency
  );
}

/**
 * Creates a new GeomMunger object to munge vertices appropriate to this GSG
 * for the indicated state.
 */
PT(GeomMunger) DXGraphicsStateGuardian11::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  PT(DXGeomMunger11) munger = new DXGeomMunger11(this, state);
  return GeomMunger::register_munger(munger, current_thread);
}

/**
 * Called before a sequence of draw_primitive() functions are called, this
 * should prepare the vertex data for rendering.  It returns true if the
 * vertices are ok, false to abort this group of primitives.
 */
bool DXGraphicsStateGuardian11::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomVertexDataPipelineReader *data_reader,
                      size_t num_instances, bool force) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom_reader, data_reader, num_instances, force)) {
    return false;
  }
  nassertr(_data_reader != nullptr, false);

  // We must have a shader!
  nassertr(_current_shader_context != nullptr, false);

  // Get an input layout for this shader+vertex format combination.
  const GeomVertexFormat *format = data_reader->get_format();
  BitMask32 enabled_arrays;
  ID3D11InputLayout *layout = _current_shader_context->get_input_layout(format, enabled_arrays);
  //nassertr(layout != nullptr, false);
  if (layout == nullptr) {
    return false;
  }
  if (layout != _curr_input_layout) {
    _context->IASetInputLayout(layout);
  }

  int max_slot = enabled_arrays.get_highest_on_bit();

  // Make sure each array that will be used by the shader is prepared for
  // rendering.  We also need to check if the set of vertex arrays differs
  // from the current set.
  int min_changed_slot = 10000;
  bool changed = false;
  int index = enabled_arrays.get_lowest_on_bit();
  while (index >= 0) {
    const GeomVertexArrayDataHandle *array_reader = data_reader->get_array_reader(index);
    DXVertexBufferContext11 *dvbc = (DXVertexBufferContext11 *)array_reader->prepare_now(_prepared_objects, this);
    nassertr(dvbc != nullptr, false);
    // Make sure latest client data is uploaded to the GPU.
    dvbc->update_buffer(_context, array_reader);

    ID3D11Buffer *vertex_buffer = dvbc->get_buffer();
    nassertr(vertex_buffer != nullptr, false);

    UINT stride = (UINT)array_reader->get_array_format()->get_stride();

    if (_curr_vertex_buffers[index] != vertex_buffer || _curr_vertex_buffer_strides[index] != stride) {
      _curr_vertex_buffers[index] = vertex_buffer;
      _curr_vertex_buffer_strides[index] = stride;
      min_changed_slot = std::min(min_changed_slot, index);
      changed = true;
    }

    enabled_arrays.clear_bit(index);
    index = enabled_arrays.get_lowest_on_bit();
  }

  if (changed) {
    // Set of vertex arrays have changed.
    int num_changed_arrays = (max_slot - min_changed_slot) + 1;
    UINT *offsets = (UINT *)alloca(sizeof(UINT) * num_changed_arrays);
    memset(offsets, 0, sizeof(UINT) * num_changed_arrays);
    _context->IASetVertexBuffers(min_changed_slot, num_changed_arrays, _curr_vertex_buffers + min_changed_slot,
                                 _curr_vertex_buffer_strides + min_changed_slot, offsets);
  }

  _instance_count = num_instances;

  return true;
}

/**
 * Draws a series of disconnected triangles.
 */
bool DXGraphicsStateGuardian11::
draw_triangles(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

/**
 * Draws a series of disconnected triangles with adjacency information.
 */
bool DXGraphicsStateGuardian11::
draw_triangles_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
}

/**
 * Draws a series of triangle strips.
 *
 * Under D3D11, primitive restart for strips is always enabled and the
 * strip cut index is inferred from the index format of the currently bound
 * index buffer.
 */
bool DXGraphicsStateGuardian11::
draw_tristrips(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

/**
 * Draws a series of triangle strips with adjacency information.
 *
 * Under D3D11, primitive restart for strips is always enabled and the
 * strip cut index is inferred from the index format of the currently bound
 * index buffer.
 */
bool DXGraphicsStateGuardian11::
draw_tristrips_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ);
}

/**
 * Draws a series of triangle fans.
 */
bool DXGraphicsStateGuardian11::
draw_trifans(const GeomPrimitivePipelineReader *, bool) {
  // D3D11 dropped support for trifan primitives, so we'll bitch about it to
  // the user every so often.
  static double last_bitch_time = 0.0;
  static constexpr double bitch_interval = 1.0;
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now - last_bitch_time >= bitch_interval) {
    dxgsg11_cat.warning()
      << "Attempted to draw trifans, which are not supported under Direct3D 11.\n";
    last_bitch_time = now;
  }
  return false;
}

/**
 * Draws a series of "patches", which can only be processed by a tessellation
 * shader.
 */
bool DXGraphicsStateGuardian11::
draw_patches(const GeomPrimitivePipelineReader *reader, bool force) {
  int num_control_points = reader->get_object()->get_num_vertices_per_primitive();
  nassertr(num_control_points >= 1 && num_control_points <= 32, false);
  return draw(reader, (D3D11_PRIMITIVE_TOPOLOGY)(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (num_control_points - 1)));
}

/**
 * Draws a series of disconnected line segments.
 */
bool DXGraphicsStateGuardian11::
draw_lines(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

/**
 * Draws a series of disconnected line segments with adjacency information.
 */
bool DXGraphicsStateGuardian11::
draw_lines_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);
}

/**
 * Draws a series of line strips.
 *
 * Under D3D11, primitive restart for strips is always enabled and the
 * strip cut index is inferred from the index format of the currently bound
 * index buffer.
 */
bool DXGraphicsStateGuardian11::
draw_linestrips(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

/**
 * Draws a series of line strips with adjacency information.
 *
 * Under D3D11, primitive restart for strips is always enabled and the
 * strip cut index is inferred from the index format of the currently bound
 * index buffer.
 */
bool DXGraphicsStateGuardian11::
draw_linestrips_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
}

/**
 * Draws a series of disconnected points.
 */
bool DXGraphicsStateGuardian11::
draw_points(const GeomPrimitivePipelineReader *reader, bool force) {
  return draw(reader, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

/**
 * Simultaneously resets the render state and the transform state.
 *
 * This transform specified is the "internal" net transform, already converted
 * into the GSG's internal coordinate space by composing it to
 * get_cs_transform().  (Previously, this used to be the "external" net
 * transform, with the assumption that that GSG would convert it internally,
 * but that is no longer the case.)
 *
 * Special case: if (state==NULL), then the target state is already stored in
 * _target.
 */
void DXGraphicsStateGuardian11::
set_state_and_transform(const RenderState *target,
                        const TransformState *transform) {
  if (transform != _internal_transform) {
    _internal_transform = transform;
    _temp_cbuffer_data[0] = transform->get_mat();
    //_temp_cbuffer_data[0].transpose_in_place();
    _temp_cbuffer_stale = true;
  }

  //_temp_cbuffer_data[0] = _projection_mat->compose(transform)->get_mat();
  //_temp_cbuffer_stale = true;

  if (target == _state_rs) {
    return;
  }
  _target_rs = target;

  // Set pipeline states.
  ID3D11RasterizerState *rasterizer;
  ID3D11BlendState *blend;
  LVecBase4f blend_factor;
  ID3D11DepthStencilState *depth_stencil;
  get_pipeline_states_for_render_state(target, rasterizer, blend, depth_stencil, blend_factor);

  if (rasterizer != _curr_pipeline_states._rasterizer) {
    _context->RSSetState(rasterizer);
    _curr_pipeline_states._rasterizer = rasterizer;
  }

  if (depth_stencil != _curr_pipeline_states._depth_stencil) {
    _context->OMSetDepthStencilState(depth_stencil, 0);
    _curr_pipeline_states._depth_stencil = depth_stencil;
  }

  if (blend != _curr_pipeline_states._blend ||
      blend_factor != _curr_pipeline_states._blend_factor) {
    _context->OMSetBlendState(blend, blend_factor.get_data(), 0xffffff);
    _curr_pipeline_states._blend = blend;
    _curr_pipeline_states._blend_factor = blend_factor;
  }

  determine_target_shader();

  if (_target_shader != _state_shader) {
    nassertv(_target_shader != nullptr);
    DXShaderContext11 *context = nullptr;
    Shader *shader = get_default_shader();
    //dxgsg11_cat.info()
    //  << "Default shader: " << shader << "\n";
    //if (_target_shader != nullptr) {
    //  shader = get_default_shader();//(Shader *)_target_shader->get_shader();
    //}

    //if (shader == nullptr) {
    //  shader = get_default_shader();
    //}

    if (shader != nullptr) {
      context = (DXShaderContext11 *)shader->prepare_now(_prepared_objects, this);

      //dxgsg11_cat.info()
      //  << "Default shader context: " << context << "\n";
    }

    nassertv(context != nullptr);

    if (context != _current_shader_context) {
      // Shader changed.

      _current_shader_context = (DXShaderContext11 *)context;

      // Bind shaders of new context.
      if (_current_shader_context->_vshader != _curr_vertex_shader) {
        _context->VSSetShader(_current_shader_context->_vshader, nullptr, 0);
        _curr_vertex_shader = _current_shader_context->_vshader;
      }
      if (_current_shader_context->_gshader != _curr_geometry_shader) {
        _context->GSSetShader(_current_shader_context->_gshader, nullptr, 0);
        _curr_geometry_shader = _current_shader_context->_gshader;
      }
      if (_current_shader_context->_pshader != _curr_pixel_shader) {
        _context->PSSetShader(_current_shader_context->_pshader, nullptr, 0);
        _curr_pixel_shader = _current_shader_context->_pshader;
      }

      _context->VSSetConstantBuffers(0, 1, &_temp_cbuffer);
    }

    _state_shader = _target_shader;
  }

  determine_target_texture();
  if (_target_texture != _state_texture) {
    Texture *tex = _target_texture->get_texture();
    if (tex != nullptr) {
      DXTextureContext11 *dtc = (DXTextureContext11 *)tex->prepare_now(0, _prepared_objects, this);
      if (dtc != nullptr) {
        ID3D11ShaderResourceView *srv = dtc->_d3d_srv;
        if (srv != _curr_tex) {
          _context->PSSetShaderResources(0, 1, &srv);
          _curr_tex = srv;
        }
      } else {
        ID3D11ShaderResourceView *null_srv[1] = { nullptr };
        _context->PSSetShaderResources(0, 1, null_srv);
      }

      DXSamplerContext11 *dsc = (DXSamplerContext11 *)tex->get_default_sampler().prepare_now(_prepared_objects, this);
      if (dsc != nullptr) {
        ID3D11SamplerState *ss = dsc->get_sampler_state();
        if (ss != _curr_sampler) {
          _context->PSSetSamplers(0, 1, &ss);
          _curr_sampler = ss;
        }
      } else {
        ID3D11SamplerState *null_samp[1] = { nullptr };
        _context->PSSetSamplers(0, 1, null_samp);
      }

    } else {
      ID3D11ShaderResourceView *null_srv[1] = { nullptr };
      _context->PSSetShaderResources(0, 1, null_srv);
    }

    _state_texture = _target_texture;
  }

  _state_rs = target;
}

/**
 * Called before each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup before beginning the frame.
 *
 * The return value is true if successful (in which case the frame will be
 * drawn and end_frame() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_frame() will not be called).
 */
bool DXGraphicsStateGuardian11::
begin_frame(Thread *current_thread) {
  return GraphicsStateGuardian::begin_frame(current_thread);
}

/**
 * Makes the current lens (whichever lens was most recently specified with
 * set_scene()) active, so that it will transform future rendered geometry.
 * Normally this is only called from the draw process, and usually it is
 * called by set_scene().
 *
 * The return value is true if the lens is acceptable, false if it is not.
 */
bool DXGraphicsStateGuardian11::
prepare_lens() {
  // No-op.  Projection matrix is a shader constant.
  //_temp_cbuffer_data[1] = _scene_setup->get_cs_world_transform()->get_mat();
  //_temp_cbuffer_data[1].transpose_in_place();
  _temp_cbuffer_data[1] = _projection_mat->get_mat();
  //_temp_cbuffer_data[2].transpose_in_place();
  _temp_cbuffer_stale = true;
  return true;
}

/**
 * Makes the specified DisplayRegion current.  All future drawing and clear
 * operations will be constrained within the given DisplayRegion.
 */
void DXGraphicsStateGuardian11::
prepare_display_region(DisplayRegionPipelineReader *dr) {
  GraphicsStateGuardian::prepare_display_region(dr);

  int count = dr->get_num_regions();
  bool do_scissor = dr->get_scissor_enabled();
  PN_stdfloat near_depth, far_depth;
  dr->get_depth_range(near_depth, far_depth);

  D3D11_VIEWPORT *vp = (D3D11_VIEWPORT *)alloca(sizeof(D3D11_VIEWPORT) * count);
  D3D11_RECT *rect;
  if (do_scissor) {
    rect = (D3D11_RECT *)alloca(sizeof(D3D11_RECT) * count);
  }

  for (int i = 0; i < count; ++i) {
    int xo, yo, w, h;
    dr->get_region_pixels_i(i, xo, yo, w, h);

    vp[i].Width = w;
    vp[i].Height = h;
    vp[i].TopLeftX = xo;
    vp[i].TopLeftY = yo;
    vp[i].MinDepth = near_depth;
    vp[i].MaxDepth = far_depth;

    if (do_scissor) {
      rect[i].left = xo;
      rect[i].top = yo;
      rect[i].right = w + xo;
      rect[i].bottom = h + yo;
    }
  }

  _context->RSSetViewports(count, vp);

  if (do_scissor) {
    _context->RSSetScissorRects(count, rect);
  }
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
VertexBufferContext *DXGraphicsStateGuardian11::
prepare_vertex_buffer(GeomVertexArrayData *data) {
  DXVertexBufferContext11 *dvbc = new DXVertexBufferContext11(this, _prepared_objects, data);
  return dvbc;
}

/**
 *
 */
void DXGraphicsStateGuardian11::
release_vertex_buffer(VertexBufferContext *vbc) {
  DXVertexBufferContext11 *dvbc = DCAST(DXVertexBufferContext11, vbc);
  delete dvbc;
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
IndexBufferContext *DXGraphicsStateGuardian11::
prepare_index_buffer(GeomPrimitive *prim) {
  DXIndexBufferContext11 *dibc = new DXIndexBufferContext11(this, _prepared_objects, prim);
  return dibc;
}

/**
 *
 */
void DXGraphicsStateGuardian11::
release_index_buffer(IndexBufferContext *ibc) {
  DXIndexBufferContext11 *dibc = DCAST(DXIndexBufferContext11, ibc);
  delete dibc;
}

/**
 *
 */
ShaderContext *DXGraphicsStateGuardian11::
prepare_shader(Shader *shader) {
  DXShaderContext11 *dsc = new DXShaderContext11(shader, this);
  return dsc;
}

/**
 *
 */
void DXGraphicsStateGuardian11::
release_shader(ShaderContext *sc) {
  DXShaderContext11 *dsc = (DXShaderContext11 *)sc;
  delete dsc;
}

/**
 * Creates whatever structures the GSG requires to represent the texture
 * internally, and returns a newly-allocated TextureContext object with this
 * data.  It is the responsibility of the calling function to later call
 * release_texture() with this same pointer (which will also delete the
 * pointer).
 *
 * This function should not be called directly to prepare a texture.  Instead,
 * call Texture::prepare().
 */
TextureContext *DXGraphicsStateGuardian11::
prepare_texture(Texture *tex, int view) {
  DXTextureContext11 *dtc = new DXTextureContext11(_prepared_objects, this, tex, view);
  dtc->upload_texture(_context);
  return dtc;
}

/**
 * Ensures that the current Texture data is refreshed onto the GSG.  This
 * means updating the texture properties and/or re-uploading the texture
 * image, if necessary.  This should only be called within the draw thread.
 *
 * If force is true, this function will not return until the texture has been
 * fully uploaded.  If force is false, the function may choose to upload a
 * simple version of the texture instead, if the texture is not fully resident
 * (and if get_incomplete_render() is true).
 */
bool DXGraphicsStateGuardian11::
update_texture(TextureContext *tc, bool force) {
  return true;
}

/**
 * Frees the resources previously allocated via a call to prepare_texture(),
 * including deleting the TextureContext itself, if it is non-NULL.
 */
void DXGraphicsStateGuardian11::
release_texture(TextureContext *tc) {
  DXTextureContext11 *dtc = (DXTextureContext11 *)tc;
  delete dtc;
}

/**
 * This method should only be called by the GraphicsEngine.  Do not call it
 * directly; call GraphicsEngine::extract_texture_data() instead.
 *
 * This method will be called in the draw thread to download the texture
 * memory's image into its ram_image value.  It returns true on success, false
 * otherwise.
 */
bool DXGraphicsStateGuardian11::
extract_texture_data(Texture *tex) {
  return false;
}

/**
 * Creates whatever structures the GSG requires to represent the sampler
 * internally, and returns a newly-allocated SamplerContext object with this
 * data.  It is the responsibility of the calling function to later call
 * release_sampler() with this same pointer (which will also delete the
 * pointer).
 *
 * This function should not be called directly to prepare a sampler.  Instead,
 * call Texture::prepare().
 */
SamplerContext *DXGraphicsStateGuardian11::
prepare_sampler(const SamplerState &sampler) {
  return new DXSamplerContext11(sampler, this);
}

/**
 * Frees the resources previously allocated via a call to prepare_sampler(),
 * including deleting the SamplerContext itself, if it is non-NULL.
 */
void DXGraphicsStateGuardian11::
release_sampler(SamplerContext *sc) {
  DXSamplerContext11 *dsc = (DXSamplerContext11 *)sc;
  delete dsc;
}

/**
 *
 */
static D3D11_COMPARISON_FUNC
panda_compare_to_d3d_compare(RenderAttrib::PandaCompareFunc func) {
  switch (func) {
  case RenderAttrib::M_never:
    return D3D11_COMPARISON_NEVER;
  case RenderAttrib::M_less:
    return D3D11_COMPARISON_LESS;
  case RenderAttrib::M_equal:
    return D3D11_COMPARISON_EQUAL;
  case RenderAttrib::M_less_equal:
    return D3D11_COMPARISON_LESS_EQUAL;
  case RenderAttrib::M_greater:
    return D3D11_COMPARISON_GREATER;
  case RenderAttrib::M_not_equal:
    return D3D11_COMPARISON_NOT_EQUAL;
  case RenderAttrib::M_greater_equal:
    return D3D11_COMPARISON_GREATER_EQUAL;
  case RenderAttrib::M_always:
  default:
    return D3D11_COMPARISON_ALWAYS;
  }
}

/**
 *
 */
static D3D11_BLEND
color_blend_operand_to_d3d_blend(ColorBlendAttrib::Operand op) {
  switch (op) {
  case ColorBlendAttrib::O_zero:
    return D3D11_BLEND_ZERO;
  case ColorBlendAttrib::O_one:
    return D3D11_BLEND_ONE;
  case ColorBlendAttrib::O_incoming_color:
    return D3D11_BLEND_SRC_COLOR;
  case ColorBlendAttrib::O_one_minus_incoming_color:
    return D3D11_BLEND_INV_SRC_COLOR;
  case ColorBlendAttrib::O_fbuffer_color:
    return D3D11_BLEND_DEST_COLOR;
  case ColorBlendAttrib::O_one_minus_fbuffer_color:
    return D3D11_BLEND_INV_DEST_COLOR;
  case ColorBlendAttrib::O_incoming_alpha:
    return D3D11_BLEND_SRC_ALPHA;
  case ColorBlendAttrib::O_one_minus_incoming_alpha:
    return D3D11_BLEND_INV_SRC_ALPHA;
  case ColorBlendAttrib::O_fbuffer_alpha:
    return D3D11_BLEND_DEST_ALPHA;
  case ColorBlendAttrib::O_one_minus_fbuffer_alpha:
    return D3D11_BLEND_INV_DEST_ALPHA;
  case ColorBlendAttrib::O_constant_color:
  case ColorBlendAttrib::O_constant_alpha:
  case ColorBlendAttrib::O_color_scale:
  case ColorBlendAttrib::O_alpha_scale:
    return D3D11_BLEND_BLEND_FACTOR;
  case ColorBlendAttrib::O_one_minus_constant_color:
  case ColorBlendAttrib::O_one_minus_constant_alpha:
  case ColorBlendAttrib::O_one_minus_color_scale:
  case ColorBlendAttrib::O_one_minus_alpha_scale:
    return D3D11_BLEND_INV_BLEND_FACTOR;
  case ColorBlendAttrib::O_incoming_color_saturate:
    return D3D11_BLEND_SRC_ALPHA_SAT;
  case ColorBlendAttrib::O_incoming1_color:
    return D3D11_BLEND_SRC1_COLOR;
  case ColorBlendAttrib::O_one_minus_incoming1_color:
    return D3D11_BLEND_INV_SRC1_COLOR;
  case ColorBlendAttrib::O_incoming1_alpha:
    return D3D11_BLEND_SRC1_ALPHA;
  case ColorBlendAttrib::O_one_minus_incoming1_alpha:
    return D3D11_BLEND_INV_SRC1_ALPHA;
  }
}

/**
 *
 */
static D3D11_BLEND_OP
color_blend_mode_to_d3d_blend_op(ColorBlendAttrib::Mode mode) {
  switch (mode) {
  case ColorBlendAttrib::M_add:
    return D3D11_BLEND_OP_ADD;
  case ColorBlendAttrib::M_subtract:
    return D3D11_BLEND_OP_SUBTRACT;
  case ColorBlendAttrib::M_inv_subtract:
    return D3D11_BLEND_OP_REV_SUBTRACT;
  case ColorBlendAttrib::M_min:
    return D3D11_BLEND_OP_MIN;
  case ColorBlendAttrib::M_max:
    return D3D11_BLEND_OP_MAX;
  default:
    // Shouldn't be called with M_none.
    dxgsg11_cat.error()
      << "color_blend_mode_to_d3d_blend_op() shouldn't be called with M_none!\n";
    return D3D11_BLEND_OP_ADD;
  }
}

/**
 *
 */
void DXGraphicsStateGuardian11::
get_pipeline_states_for_render_state(const RenderState *state, ID3D11RasterizerState *&rasterizer,
                                     ID3D11BlendState *&blend, ID3D11DepthStencilState *&depth_stencil,
                                     LVecBase4f &blend_factor) {
  auto it = _rs_pipeline_states.find(state);
  if (it != _rs_pipeline_states.end()) {
    const PipelineStates &states = (*it).second;
    rasterizer = states._rasterizer;
    blend = states._blend;
    blend_factor = states._blend_factor;
    depth_stencil = states._depth_stencil;
    return;
  }

  // Haven't used this RenderState before.  Need to create new pipeline
  // states.

  //
  // Fill out rasterizer state.
  //
  D3D11_RASTERIZER_DESC rdesc;
  rdesc.FrontCounterClockwise = 0;

  const RenderModeAttrib *rma;
  state->get_attrib_def(rma);
  switch (rma->get_mode()) {
  case RenderModeAttrib::M_filled:
  default:
    rdesc.FillMode = D3D11_FILL_SOLID;
    break;
  case RenderModeAttrib::M_wireframe:
    rdesc.FillMode = D3D11_FILL_WIREFRAME;
    break;
  }

  const CullFaceAttrib *cfa;
  state->get_attrib_def(cfa);
  switch (cfa->get_effective_mode()) {
  case CullFaceAttrib::M_cull_none:
    rdesc.CullMode = D3D11_CULL_NONE;
    break;
  case CullFaceAttrib::M_cull_clockwise:
    rdesc.CullMode = D3D11_CULL_FRONT;
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
  default:
    rdesc.CullMode = D3D11_CULL_BACK;
    break;
  }

  rdesc.DepthBias = 0.0f;
  rdesc.SlopeScaledDepthBias = 0.0f;
  rdesc.DepthBiasClamp = 0.0f;
  rdesc.DepthClipEnable = 0;

  const DepthOffsetAttrib *doa;
  if (state->get_attrib(doa)) {
    rdesc.DepthBias += doa->get_offset();
  }

  const DepthBiasAttrib *dba;
  if (state->get_attrib(dba)) {
    rdesc.DepthBias += dba->get_constant_factor();
    rdesc.SlopeScaledDepthBias += dba->get_slope_factor();
    rdesc.DepthBiasClamp += dba->get_clamp();
  }

  const ScissorAttrib *sca;
  state->get_attrib_def(sca);
  rdesc.ScissorEnable = !sca->is_off();

  const AntialiasAttrib *aa;
  state->get_attrib_def(aa);
  rdesc.MultisampleEnable = (aa->get_mode_type() == AntialiasAttrib::M_multisample);
  rdesc.AntialiasedLineEnable = (aa->get_mode_type() == AntialiasAttrib::M_line);

  HRESULT hr = _device->CreateRasterizerState(&rdesc, &rasterizer);
  nassertv(SUCCEEDED(hr));

  //
  // Fill out blend state.
  //
  CD3D11_BLEND_DESC bdesc(CD3D11_DEFAULT{});

  const ColorBlendAttrib *cba;
  state->get_attrib_def(cba);
  if (cba->get_mode() != ColorBlendAttrib::M_none) {
    // Doing a custom blend for RGB.
    // Overrides transparency.
    bdesc.RenderTarget[0].BlendEnable = 1;
    bdesc.RenderTarget[0].SrcBlend = color_blend_operand_to_d3d_blend(cba->get_operand_a());
    bdesc.RenderTarget[0].DestBlend = color_blend_operand_to_d3d_blend(cba->get_operand_b());
    bdesc.RenderTarget[0].BlendOp = color_blend_mode_to_d3d_blend_op(cba->get_mode());
    bdesc.RenderTarget[0].SrcBlendAlpha = color_blend_operand_to_d3d_blend(cba->get_alpha_operand_a());
    bdesc.RenderTarget[0].DestBlendAlpha = color_blend_operand_to_d3d_blend(cba->get_alpha_operand_b());
    bdesc.RenderTarget[0].BlendOpAlpha = color_blend_mode_to_d3d_blend_op(cba->get_alpha_mode());

  } else {
    // No color blend, check for transparency.
    const TransparencyAttrib *ta;
    state->get_attrib_def(ta);
    switch (ta->get_mode()) {
    case TransparencyAttrib::M_none:
    case TransparencyAttrib::M_binary:
      // No transparency or alpha tested transparency.
      // Alpha testing happens in the shader, no longer fixed-function.
      break;

    case TransparencyAttrib::M_alpha:
    case TransparencyAttrib::M_multisample:
    case TransparencyAttrib::M_multisample_mask:
    case TransparencyAttrib::M_dual:
      // Alpha blending.
      bdesc.RenderTarget[0].BlendEnable = 1;
      bdesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
      bdesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
      bdesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
      bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
      bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
      bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
      break;

    case TransparencyAttrib::M_premultiplied_alpha:
      bdesc.RenderTarget[0].BlendEnable = 1;
      bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
      bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
      bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
      break;

    default:
      dxgsg11_cat.error()
        << "Invalid transparency mode: " << ta->get_mode() << "\n";
      break;
    }
  }

  const ColorWriteAttrib *cwa;
  state->get_attrib_def(cwa);
  unsigned int channels = cwa->get_channels();
  if (channels & ColorWriteAttrib::C_red) {
    bdesc.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
  }
  if (channels & ColorWriteAttrib::C_green) {
    bdesc.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
  }
  if (channels & ColorWriteAttrib::C_blue) {
    bdesc.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
  }
  if (channels & ColorWriteAttrib::C_alpha) {
    bdesc.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
  }

  const ColorScaleAttrib *csa;
  state->get_attrib_def(csa);

  // If any of the operands include color scale or alpha scale,
  // use the color from the ColorScaleAttrib.
  if (cba->get_operand_a() == ColorBlendAttrib::O_color_scale ||
      cba->get_operand_a() == ColorBlendAttrib::O_alpha_scale ||
      cba->get_operand_a() == ColorBlendAttrib::O_one_minus_color_scale ||
      cba->get_operand_a() == ColorBlendAttrib::O_one_minus_alpha_scale ||
      cba->get_operand_b() == ColorBlendAttrib::O_color_scale ||
      cba->get_operand_b() == ColorBlendAttrib::O_alpha_scale ||
      cba->get_operand_b() == ColorBlendAttrib::O_one_minus_color_scale ||
      cba->get_operand_b() == ColorBlendAttrib::O_one_minus_alpha_scale ||
      cba->get_alpha_operand_a() == ColorBlendAttrib::O_color_scale ||
      cba->get_alpha_operand_a() == ColorBlendAttrib::O_alpha_scale ||
      cba->get_alpha_operand_a() == ColorBlendAttrib::O_one_minus_color_scale ||
      cba->get_alpha_operand_a() == ColorBlendAttrib::O_one_minus_alpha_scale ||
      cba->get_alpha_operand_b() == ColorBlendAttrib::O_color_scale ||
      cba->get_alpha_operand_b() == ColorBlendAttrib::O_alpha_scale ||
      cba->get_alpha_operand_b() == ColorBlendAttrib::O_one_minus_color_scale ||
      cba->get_alpha_operand_b() == ColorBlendAttrib::O_one_minus_alpha_scale) {
    blend_factor = csa->get_scale();

  } else {
    blend_factor = cba->get_color();
  }

  hr = _device->CreateBlendState(&bdesc, &blend);
  nassertv(SUCCEEDED(hr));

  //
  // Fill out depth-stencil state.
  //
  D3D11_DEPTH_STENCIL_DESC dsdesc;
  ZeroMemory(&dsdesc, sizeof(dsdesc));

  const DepthWriteAttrib *dwa;
  state->get_attrib_def(dwa);
  if (dwa->get_mode() == DepthWriteAttrib::M_on) {
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  } else {
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  }

  const DepthTestAttrib *dta;
  state->get_attrib_def(dta);
  if (dta->get_mode() == DepthTestAttrib::M_none) {
    dsdesc.DepthEnable = 0;
  } else {
    dsdesc.DepthEnable = 1;
    dsdesc.DepthFunc = panda_compare_to_d3d_compare(dta->get_mode());
  }

  //const StencilAttrib *sa;
  //state->get_attrib_def(sa);
  // TODO: Stencil.

  hr = _device->CreateDepthStencilState(&dsdesc, &depth_stencil);
  nassertv(SUCCEEDED(hr));

  // Cache it off.
  PipelineStates states;
  states._rasterizer = rasterizer;
  states._blend = blend;
  states._blend_factor = blend_factor;
  states._depth_stencil = depth_stencil;
  _rs_pipeline_states[state] = std::move(states);
}

/**
 *
 */
bool DXGraphicsStateGuardian11::
apply_index_buffer(const GeomPrimitivePipelineReader *reader) {
  DXIndexBufferContext11 *dibc = (DXIndexBufferContext11 *)reader->prepare_now(_prepared_objects, this);
  nassertr(dibc != nullptr, false);

  dibc->update_buffer(_context, reader);

  ID3D11Buffer *index_buffer = dibc->get_buffer();
  nassertr(index_buffer != nullptr, false);
  if (index_buffer != _curr_index_buffer || _curr_index_type != reader->get_index_type()) {
    DXGI_FORMAT index_type;
    switch (reader->get_index_type()) {
    case GeomEnums::NT_uint8:
      index_type = DXGI_FORMAT_R8_UINT;
      break;
    case GeomEnums::NT_uint16:
      index_type = DXGI_FORMAT_R16_UINT;
      break;
    case GeomEnums::NT_uint32:
      index_type = DXGI_FORMAT_R32_UINT;
      break;
    default:
      nassert_raise("Invalid index buffer numeric type");
      return false;
    }
    _context->IASetIndexBuffer(index_buffer, index_type, 0);
    _curr_index_buffer = index_buffer;
    _curr_index_type = reader->get_index_type();
  }

  return true;
}

/**
 *
 */
void DXGraphicsStateGuardian11::
set_topology(D3D11_PRIMITIVE_TOPOLOGY topology) {
  if (topology != _curr_topology) {
    _context->IASetPrimitiveTopology(topology);
  }
}

/**
 *
 */
bool DXGraphicsStateGuardian11::
draw(const GeomPrimitivePipelineReader *reader, D3D11_PRIMITIVE_TOPOLOGY topology) {
  if (_temp_cbuffer_stale) {
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = _context->Map(_temp_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
      memcpy(mapped.pData, _temp_cbuffer_data, sizeof(LMatrix4) * 2);
      _context->Unmap(_temp_cbuffer, 0);
    }
    _temp_cbuffer_stale = false;
  }

  if (reader->is_indexed()) {
    if (!apply_index_buffer(reader)) {
      return false;
    }

    if (topology != _curr_topology) {
      _context->IASetPrimitiveTopology(topology);
      _curr_topology = topology;
    }

    if (_instance_count != 1) {
      _context->DrawIndexedInstanced(reader->get_num_vertices(), _instance_count, 0, 0, 0);
    } else {
      _context->DrawIndexed(reader->get_num_vertices(), 0, 0);
    }

  } else {
    if (topology != _curr_topology) {
      _context->IASetPrimitiveTopology(topology);
      _curr_topology = topology;
    }

    if (_instance_count != 1) {
      _context->DrawInstanced(reader->get_num_vertices(), _instance_count, reader->get_first_vertex(), 0);
    } else {
      _context->Draw(reader->get_num_vertices(), reader->get_first_vertex());
    }
  }

  return true;
}

/**
 *
 */
Shader *DXGraphicsStateGuardian11::
get_default_shader() {
  if (_default_shader == nullptr) {
    _default_shader = Shader::load(Shader::SL_HLSL, "test.vs_5_0.hlsl", "test.ps_5_0.hlsl");
  }

  return _default_shader;
}

/**
 * Instructs the API to render into the given set of render targets and
 * depth buffer.
 */
void DXGraphicsStateGuardian11::
set_render_targets(ID3D11RenderTargetView **render_targets, int num_render_targets,
                   ID3D11DepthStencilView *depth_stencil) {
  _context->OMSetRenderTargets(num_render_targets, render_targets, depth_stencil);
  _curr_render_target_view = render_targets[0];
  _curr_depth_stencil_view = depth_stencil;
}

/**
 * Clears the framebuffer within the current DisplayRegion, according to the
 * flags indicated by the given DrawableRegion object.
 *
 * This does not set the DisplayRegion first.  You should call
 * prepare_display_region() to specify the region you wish the clear operation
 * to apply to.
 */
void DXGraphicsStateGuardian11::
clear(DrawableRegion *clearable) {
  if (!clearable->is_any_clear_active()) {
    return;
  }

  if (clearable->get_clear_color_active()) {
    if (_curr_render_target_view != nullptr) {
      LColorf clear_color = LCAST(float, clearable->get_clear_color());
      _context->ClearRenderTargetView(_curr_render_target_view, clear_color.get_data());
    }
  }

  if (clearable->get_clear_depth_active() || clearable->get_clear_stencil_active()) {
    if (_curr_depth_stencil_view != nullptr) {
      UINT clear_flags = 0;
      if (clearable->get_clear_depth_active()) {
        clear_flags |= D3D11_CLEAR_DEPTH;
      }
      if (clearable->get_clear_stencil_active()) {
        clear_flags |= D3D11_CLEAR_STENCIL;
      }
      _context->ClearDepthStencilView(_curr_depth_stencil_view, clear_flags,
        (float)clearable->get_clear_depth(), clearable->get_clear_stencil());
    }
  }

}

/**
 * Given a lens, calculates the appropriate projection matrix for use with
 * this gsg.  Note that the projection matrix depends a lot upon the
 * coordinate system of the rendering API.
 *
 * The return value is a TransformState if the lens is acceptable, NULL if it
 * is not.
 */
CPT(TransformState) DXGraphicsStateGuardian11::
calc_projection_mat(const Lens *lens) {
  if (lens == nullptr) {
    return nullptr;
  }

  if (!lens->is_linear()) {
    return nullptr;
  }

  // DirectX also uses a Z range of 0 to 1, whereas the Panda convention is
  // for the projection matrix to produce a Z range of -1 to 1.  We have to
  // rescale to compensate.
  static const LMatrix4 rescale_mat
    (1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 0.5, 0,
     0, 0, 0.5, 1);

  LMatrix4 result =
    LMatrix4::convert_mat(_internal_coordinate_system,
                          lens->get_coordinate_system()) *
    lens->get_projection_mat(_current_stereo_channel) *
    rescale_mat;

  if (_scene_setup->get_inverted()) {
    // If the scene is supposed to be inverted, then invert the projection
    // matrix.
    result *= LMatrix4::scale_mat(1.0f, -1.0f, 1.0f);
  }

  return TransformState::make_mat(result);
}
