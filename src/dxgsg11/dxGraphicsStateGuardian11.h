/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsStateGuardian11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXGRAPHICSSTATEGUARDIAN11_H
#define DXGRAPHICSSTATEGUARDIAN11_H

#include "pandabase.h"
#include "graphicsStateGuardian.h"
#include "geomMunger.h"
#include "pointerTo.h"
#include "renderState.h"

#include <pmap.h>

#include <d3d11.h>

class ID3D11Device;
class ID3D11DeviceContext;
class DXGraphicsDevice11;
class DXShaderContext11;

/**
 * There is one DXGraphicsStateGuardian for each DXGraphicsDevice being used.
 */
class EXPCL_PANDA_DXGSG11 DXGraphicsStateGuardian11 : public GraphicsStateGuardian {
public:
  DXGraphicsStateGuardian11(GraphicsEngine *engine, GraphicsPipe *pipe,
                            DXGraphicsDevice11 *device);

  void determine_capabilities();

  virtual void reset() override;

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state, Thread *current_thread) override;

  INLINE ID3D11Device *get_device() const;
  INLINE ID3D11DeviceContext *get_context() const;

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     size_t num_instances, bool force) override;
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force) override;
  virtual bool draw_triangles_adj(const GeomPrimitivePipelineReader *reader,
                                  bool force) override;
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force) override;
  virtual bool draw_tristrips_adj(const GeomPrimitivePipelineReader *reader,
                                  bool force) override;
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force) override;
  virtual bool draw_patches(const GeomPrimitivePipelineReader *reader,
                            bool force) override;
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force) override;
  virtual bool draw_lines_adj(const GeomPrimitivePipelineReader *reader,
                              bool force) override;
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force) override;
  virtual bool draw_linestrips_adj(const GeomPrimitivePipelineReader *reader,
                                   bool force) override;
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force) override;

  virtual void set_state_and_transform(const RenderState *target, const TransformState *transform) override;

  virtual bool begin_frame(Thread *current_thread) override;
  virtual bool prepare_lens() override;

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr) override;

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data) override;
  virtual void release_vertex_buffer(VertexBufferContext *vbc) override;

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data) override;
  virtual void release_index_buffer(IndexBufferContext *ibc) override;

  virtual ShaderContext *prepare_shader(Shader *shader) override;
  virtual void release_shader(ShaderContext *sc) override;

  virtual TextureContext *prepare_texture(Texture *tex, int view) override;
  virtual bool update_texture(TextureContext *tc, bool force) override;
  virtual void release_texture(TextureContext *tc) override;
  virtual bool extract_texture_data(Texture *tex) override;

  virtual SamplerContext *prepare_sampler(const SamplerState &sampler);
  virtual void release_sampler(SamplerContext *sc);

  void get_pipeline_states_for_render_state(const RenderState *state, ID3D11RasterizerState *&rasterizer,
                                            ID3D11BlendState *&blend, ID3D11DepthStencilState *&depth_stencil,
                                            LVecBase4f &blend_factor);

  bool apply_index_buffer(const GeomPrimitivePipelineReader *reader);
  void set_topology(D3D11_PRIMITIVE_TOPOLOGY topology);

  bool draw(const GeomPrimitivePipelineReader *reader, D3D11_PRIMITIVE_TOPOLOGY topology);

  Shader *get_default_shader();

  void set_render_targets(ID3D11RenderTargetView **render_targets, int num_render_targets,
                          ID3D11DepthStencilView *depth_stencil);

  virtual void clear(DrawableRegion *clearable) override;

  virtual CPT(TransformState) calc_projection_mat(const Lens *lens) override;

private:
  DXGraphicsDevice11 *_pdevice;
  ID3D11Device *_device;
  ID3D11DeviceContext *_context;

  DXShaderContext11 *_current_shader_context;

  ID3D11InputLayout *_curr_input_layout;
  ID3D11VertexShader *_curr_vertex_shader;
  ID3D11GeometryShader *_curr_geometry_shader;
  ID3D11PixelShader *_curr_pixel_shader;

  ID3D11Buffer *_curr_index_buffer;
  GeomEnums::NumericType _curr_index_type;

  ID3D11Buffer *_curr_vertex_buffers[5];
  UINT _curr_vertex_buffer_strides[5];

  D3D11_PRIMITIVE_TOPOLOGY _curr_topology;

  size_t _instance_count;

  class PipelineStates {
  public:
    ID3D11RasterizerState *_rasterizer;
    ID3D11BlendState *_blend;
    LVecBase4f _blend_factor;
    ID3D11DepthStencilState *_depth_stencil;
  };
  PipelineStates _curr_pipeline_states;

  // Cache of RenderState pointers to D3D11 pipeline states.
  typedef pflat_hash_map<CPT(RenderState), PipelineStates> RenderStatePipelineStates;
  RenderStatePipelineStates _rs_pipeline_states;

  ID3D11RenderTargetView *_curr_render_target_view;
  ID3D11DepthStencilView *_curr_depth_stencil_view;

  ID3D11ShaderResourceView *_curr_tex;
  ID3D11SamplerState *_curr_sampler;

  ID3D11Buffer *_temp_cbuffer;
  LMatrix4 *_temp_cbuffer_data;
  bool _temp_cbuffer_stale;

  PT(Shader) _default_shader;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "DXGraphicsStateGuardian11",
                  GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGraphicsStateGuardian11.I"

#endif // DXGRAPHICSSTATEGUARDIAN11_H
