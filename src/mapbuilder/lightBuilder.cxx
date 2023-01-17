/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightBuilder.cxx
 * @author brian
 * @date 2021-09-18
 */

#include "lightBuilder.h"
#include "nodePath.h"
#include "materialAttrib.h"
#include "material.h"
#include "materialParamTexture.h"
#include "config_mapbuilder.h"
#include "mathutil_misc.h"
#include "graphicsEngine.h"
#include "graphicsPipe.h"
#include "graphicsOutput.h"
#include "geomVertexRewriter.h"
#include "geomPrimitive.h"
#include "geomTriangles.h"
#include "geomVertexData.h"
#include "graphicsPipeSelection.h"
#include "windowProperties.h"
#include "frameBufferProperties.h"
#include "nodePath.h"
#include "camera.h"
#include "orthographicLens.h"
#include "omniBoundingVolume.h"
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "shader.h"
#include "shaderInput.h"
#include "textureAttrib.h"
#include "materialAttrib.h"
#include "materialParamTexture.h"
#include "materialParamColor.h"
#include "materialParamBool.h"
#include "materialParamFloat.h"
#include "pfmFile.h"
#include "load_prc_file.h"
#include "antialiasAttrib.h"
#include "colorBlendAttrib.h"
#include "indent.h"
#include "colorRGBExp32.h"

#ifndef CPPPARSER
#include "OpenImageDenoise/oidn.hpp"
#include "fp16.h"
#endif

NotifyCategoryDef(lightbuilder, "");

CPT(InternalName) LightBuilder::_lightmap_uv_name = nullptr;

/**
 *
 */
static int
align_to_next_multiple(int offset, int alignment) {
  return ((offset + alignment - 1) / alignment) * alignment;
}

/**
 *
 */
LightBuilder::
LightBuilder() :
  _lightmap_size(-1),
  _bias(0.01f),
  _bounces(5), // 5
  _rays_per_luxel(256),
  _ray_region_size(128),
  _rays_per_region(32),
  _graphics_engine(GraphicsEngine::get_global_ptr()),
  _host_output(nullptr),
  _gsg(nullptr),
  _sky_color(0, 0, 0, 1),
  _sun_angular_extent(0.0f)
{
}

/**
 * Adds all Geoms from the indicated scene graph to the solver.
 */
void LightBuilder::
add_subgraph(NodePath root, const LVecBase2i &lightmap_size) {
  PandaNode *node = root.node();

  if (node->is_of_type(GeomNode::get_class_type())) {
    // Here's a GeomNode.  Collect all of its Geoms.
    add_geom_node(DCAST(GeomNode, node), lightmap_size);
  }

  // Traverse to children.
  for (int i = 0; i < node->get_num_children(); i++) {
    add_subgraph(NodePath(node->get_child(i)), lightmap_size);
  }
}

/**
 * Adds the Geoms from the indicated GeomNode to the solver.
 */
void LightBuilder::
add_geom_node(GeomNode *geom_node, const LVecBase2i &lightmap_size) {
  NodePath geom_np(geom_node);
  CPT(TransformState) net_transform = geom_np.get_net_transform();
  CPT(RenderState) net_state = geom_np.get_net_state();
  for (int i = 0; i < geom_node->get_num_geoms(); i++) {
    const Geom *geom = geom_node->get_geom(i);
    CPT(RenderState) state = net_state->compose(geom_node->get_geom_state(i));
    add_geom(geom, state, net_transform, lightmap_size, geom_node, i);
  }
}

/**
 *
 */
void LightBuilder::
add_geom(const Geom *geom, const RenderState *state, const TransformState *transform,
         const LVecBase2i &lightmap_size, GeomNode *geom_node, int geom_index,
         uint32_t contents) {
  if (geom->is_empty()) {
    lightbuilder_cat.info()
      << "Skipping empty Geom\n";
    return;
  }

  if (lightmap_size[0] <= 0 || lightmap_size[1] <= 0) {
    lightbuilder_cat.info()
      << "Skipping geom with invalid lightmap size\n";
    return;
  }

  if (geom->get_primitive_type() != Geom::PT_polygons) {
    // Only polygons get lightmapped.
    lightbuilder_cat.info()
      << "Skipping non-polygon Geom\n";
    return;
  }

  // Can't lightmap it if it doesn't have the lightmap UV set.
  if (!geom->get_vertex_data()->has_column(get_lightmap_uv_name())) {
    lightbuilder_cat.info()
      << "Skipping Geom without lightmap UV set\n";
    return;
  }

  PT(Geom) the_geom = geom->make_copy();
  // Decompose tristrips/trifans into triangles if there are any on the Geom.
  the_geom->decompose_in_place();

  bool has_transparency = false;
  const MaterialAttrib *mattr;
  if (state->get_attrib(mattr)) {
    // If we have a material attrib try to get the base color texture.
    Material *mat = mattr->get_material();
    if (mat != nullptr) {
      if (mat->_attrib_flags & Material::F_transparency) {
        if (mat->_transparency_mode > 0) {
          has_transparency = true;
        }
      } else if (mat->_attrib_flags & Material::F_alpha_test) {
        if (mat->_alpha_test_mode > 0) {
          has_transparency = true;
        }
      }
    }
  }

  LightmapGeom lgeom;
  lgeom.light_mode = LightmapGeom::LM_lightmap;
  lgeom.geom = the_geom;
  lgeom.state = state;
  lgeom.net_transform = transform;
  lgeom.vdata = geom->get_vertex_data();
  lgeom.source_geom_node = geom_node;
  lgeom.geom_index = geom_index;
  lgeom.lightmap_size = lightmap_size;
  lgeom.contents = contents;
  lgeom.num_triangles = 0;
  // Count number of tris.
  for (size_t i = 0; i < the_geom->get_num_primitives(); i++) {
    lgeom.num_triangles += the_geom->get_primitive(i)->get_num_primitives();
  }
  if (has_transparency) {
    lgeom.contents |= C_transparent;
  }
  _geoms.push_back(std::move(lgeom));
}

/**
 * Adds a geom that should have light baked per-vertex, rather than for each
 * texel of a lightmap texture.  Requires no special texture coordinates.
 */
void LightBuilder::
add_vertex_geom(const Geom *geom, const RenderState *state, const TransformState *transform,
                int model_index, int geom_index, uint32_t contents) {
  if (geom->is_empty()) {
    lightbuilder_cat.info()
      << "Skipping empty vertex Geom\n";
    return;
  }

  if (geom->get_primitive_type() != Geom::PT_polygons) {
    // Only polygons get lightmapped.
    lightbuilder_cat.info()
      << "Skipping non-polygon vertex Geom\n";
    return;
  }

  PT(Geom) the_geom = geom->make_copy();
  // Decompose tristrips/trifans into triangles if there are any on the Geom.
  the_geom->decompose_in_place();

  LightmapGeom lgeom;
  lgeom.palette = -2;
  lgeom.light_mode = LightmapGeom::LM_per_vertex;
  lgeom.geom = the_geom;
  lgeom.state = state;
  lgeom.net_transform = transform;
  lgeom.vdata = geom->get_vertex_data();
  lgeom.source_geom_node = nullptr;
  lgeom.geom_index = geom_index;
  lgeom.model_index = model_index;
  lgeom.contents = contents;
  lgeom.num_triangles = 0;
  // Count number of tris.
  for (size_t i = 0; i < the_geom->get_num_primitives(); i++) {
    lgeom.num_triangles += the_geom->get_primitive(i)->get_num_primitives();
  }
  _geoms.push_back(std::move(lgeom));
}

/**
 * Creates a graphics pipe and host offscreen buffer for dispatching compute
 * shaders and rasterizing the geometry.  When we rasterize the geometry into
 * lightmap-space textures, we will create an FBO using the offscreen buffer
 * as the host.
 */
bool LightBuilder::
initialize_pipe() {
  lightbuilder_cat.info()
    << "Initializing graphics pipe\n";

  nassertr(_graphics_engine != nullptr, false);

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  // Explicitly request an OpenGL pipe.
  _graphics_pipe = selection->make_module_pipe("pandagl");
  if (_graphics_pipe == nullptr) {
    lightbuilder_cat.error()
      << "Failed to create OpenGL graphics pipe!\n";
    return false;
  }

  // Need a host graphics output to get a GSG and a host for the rasterization
  // FBO.  Apparently we also need this to be able to dispatch compute shaders.
  FrameBufferProperties fbprops;
  fbprops.clear();
  WindowProperties winprops;
  winprops.clear();
  winprops.set_size(1, 1);
  _host_output = _graphics_engine->make_output(
    _graphics_pipe, "lightmap_host", 0, fbprops, winprops,
    GraphicsPipe::BF_refuse_window);
  if (_host_output == nullptr) {
    lightbuilder_cat.error()
      << "Failed to create host offscreen buffer!\n";
    return false;
  }

  _gsg = _host_output->get_gsg();
  nassertr(_gsg != nullptr, false);

  return true;
}

/**
 * Creates a set of lightmap palettes that contain the lightmaps of several
 * Geoms.  This is done to improve rendering performance and simplify the
 * computation process.  We can carry around the lightmaps for every Geom
 * in a single 2D array texture.
 */
bool LightBuilder::
make_palette() {
  lightbuilder_cat.info()
    << "Building lightmap palettes\n";

  pvector<size_t> sorted_geoms;
  for (size_t i = 0; i < _geoms.size(); ++i) {
    if (_geoms[i].light_mode != LightmapGeom::LM_lightmap) {
      continue;
    }
    sorted_geoms.push_back(i);
  }
  // Sort lightmapped Geoms by decreasing lightmap size.  Packs better into
  // palettes.
  std::sort(sorted_geoms.begin(), sorted_geoms.end(),
    [this](size_t a, size_t b) {
        const LightmapGeom &ga = _geoms[a];
        const LightmapGeom &gb = _geoms[b];

        if (ga.lightmap_size[1] < gb.lightmap_size[1]) {
          return false;

        } else if (gb.lightmap_size[1] < ga.lightmap_size[1]) {
          return true;

        } else if (ga.lightmap_size[0] < gb.lightmap_size[0]) {
          return false;

        } else if (gb.lightmap_size[0] > ga.lightmap_size[0]) {
          return true;

        } else {
          return false;
        }
    }
  );

  _pages.reserve(10);

  // Pack each Geom into a palette.
  for (size_t i : sorted_geoms) {
    LightmapGeom &lmgeom = _geoms[i];
    bool added = false;
    for (size_t j = 0; j < _pages.size(); j++) {
      LVecBase2i offset = _pages[j].packer.add_block(lmgeom.lightmap_size[0], lmgeom.lightmap_size[1]);
      if (offset[0] != -1) {
        // Geom was successfully added into this palette.
        added = true;
        // Record where the geom was placed.
        lmgeom.palette = j;
        lmgeom.palette_offset = offset;
        // Record the geom on the palette itself.
        _pages[j].geoms.push_back(i);
        break;
      }
    }

    if (!added) {
      // If the geom was not added into any palette above, that means that we
      // don't have any palettes yet, or the Geom couldn't fit into any
      // existing palette.  Try again on a fresh palette.
      LightmapPage page;
      page.index = _pages.size();
      page.packer.reset(0, 8192, 8192, 2);
      LVecBase2i offset = page.packer.add_block(lmgeom.lightmap_size[0], lmgeom.lightmap_size[1]);
      if (offset[0] != -1) {
        // Geom was successfully added into this palette.
        added = true;
        // Record where the geom was placed.
        lmgeom.palette = page.index;
        lmgeom.palette_offset = offset;
        // Record the geom on the palette itself.
        page.geoms.push_back(i);

      } else {
        // The geom couldn't even fit into a new palette.  This is a problem.
        lightbuilder_cat.error()
          << "Lightmap of size " << lmgeom.lightmap_size[0] << "x" << lmgeom.lightmap_size[1]
          << " is too big to fit into any palette!\n";
        return false;
      }
      _pages.push_back(page);
    }
  }

  lightbuilder_cat.info()
    << "Added " << _geoms.size() << " geoms into " << _pages.size() << " palettes\n";

  // Now that we've created a set of palettes, determine the width and height
  // to use for the array texture of palettes.  We have to find the maximum
  // width and height of all palettes.
  for (size_t i = 0; i < _pages.size(); i++) {
    LVecBase2i size = _pages[i].packer.get_minimum_dimensions_npot();
    // Align dimensions to next multiple of four for DXT compression.
    size[0] = align_to_next_multiple(size[0], 4);
    size[1] = align_to_next_multiple(size[1], 4);
    std::cout << "Page " << i << " size: " << size << "\n";
    _pages[i].dim = size;
    _lightmap_size[0] = std::max(_lightmap_size[0], size[0]);
    _lightmap_size[1] = std::max(_lightmap_size[1], size[1]);
  }

  lightbuilder_cat.info()
    << "Lightmap dimensions (max of all palettes): "
    << _lightmap_size[0] << "x" << _lightmap_size[1] << "\n";

  return true;
}

/**
 * Offsets the lightmap UVs of each Geom in the scene to correspond to where
 * it was placed in the lightmap palette.
 */
bool LightBuilder::
offset_geom_lightmap_uvs() {
  lightbuilder_cat.info()
    << "Offsetting Geom lightmap UVs\n";

  // To handle vertex datas being shared between Geoms.
  pmap<CPT(GeomVertexData), PT(GeomVertexData)> new_vdatas;

  for (size_t i = 0; i < _geoms.size(); i++) {
    LightmapGeom &lmgeom = _geoms[i];
    if (lmgeom.light_mode != LightmapGeom::LM_lightmap) {
      continue;
    }

    PT(GeomVertexData) vdata;
    auto it = new_vdatas.find(lmgeom.geom->get_vertex_data());
    if (it == new_vdatas.end()) {
      vdata = lmgeom.geom->modify_vertex_data();
      new_vdatas[lmgeom.geom->get_vertex_data()] = vdata;

    } else {
      // Another Geom shares our vertex data.  Modify the copy that they
      // made.
      vdata = (*it).second;
    }

    GeomVertexRewriter rewriter(vdata, get_lightmap_uv_name());
    nassertr(rewriter.has_column(), false);

    PN_stdfloat u_scale = 1.0f / (PN_stdfloat)_lightmap_size[0];
    PN_stdfloat u_offset = (PN_stdfloat)lmgeom.palette_offset[0] * u_scale;
    u_scale = (PN_stdfloat)lmgeom.lightmap_size[0] * u_scale;

    PN_stdfloat v_scale = 1.0f / (PN_stdfloat)_lightmap_size[1];
    PN_stdfloat v_offset = (PN_stdfloat)lmgeom.palette_offset[1] * v_scale;
    v_scale = (PN_stdfloat)lmgeom.lightmap_size[1] * v_scale;

    lmgeom.uv_mins[0] = u_offset;
    lmgeom.uv_mins[1] = v_offset;
    lmgeom.uv_maxs[0] = u_offset + u_scale;
    lmgeom.uv_maxs[1] = v_offset + v_scale;

    BitArray offset_vertices;

    // Only offset the vertices referenced by the Geom in case the vertex data
    // is shared between multiple Geoms.
    for (size_t j = 0; j < lmgeom.geom->get_num_primitives(); j++) {
      const GeomPrimitive *prim = lmgeom.geom->get_primitive(j);
      for (size_t k = 0; k < prim->get_num_primitives(); k++) {
        int start = prim->get_primitive_start(k);
        int end = prim->get_primitive_end(k);
        for (int v = start; v < end; v++) {
          int vertex = prim->get_vertex(v);
          if (offset_vertices.get_bit(vertex)) {
            // We've already offset this vertex.
            continue;
          }
          rewriter.set_row(vertex);
          LVecBase2 uv = rewriter.get_data2f();
          LVecBase2 new_uv(u_offset + uv[0] * u_scale,
                           v_offset + uv[1] * v_scale);
          rewriter.set_data2f(new_uv);
          offset_vertices.set_bit(vertex);
        }
      }
    }

    // Now apply the modified vertex data onto the Geom.
    lmgeom.geom->set_vertex_data(vdata);
    lmgeom.vdata = vdata;
  }

  return true;
}

/**
 * Creates the set of array textures that will be written to during the
 * lightmap computation process.  Each page of an array texture corresponds
 * to a lightmap palette.
 */
bool LightBuilder::
make_textures() {
  lightbuilder_cat.info()
    << "Creating lightmap textures\n";

  int max_vtx_width = 8192;
  int vtx_width;
  int vtx_height;
  vtx_width = std::max(1, std::min(max_vtx_width, _num_vertex_lit_vertices));
  vtx_height = (_num_vertex_lit_vertices / vtx_width) + 1;
  _vertex_palette_width = vtx_width;
  _vertex_palette_height = vtx_height;

  SamplerState sampler;
  sampler.set_minfilter(SamplerState::FT_linear);
  sampler.set_magfilter(SamplerState::FT_linear);
  sampler.set_min_lod(0);
  sampler.set_max_lod(0);
  sampler.set_anisotropic_degree(0);
  sampler.set_wrap_u(SamplerState::WM_clamp);
  sampler.set_wrap_v(SamplerState::WM_clamp);
  sampler.set_wrap_w(SamplerState::WM_clamp);

  // Color of direct lighting reaching a luxel.
  PT(Texture) direct = new Texture("lm_direct");
  direct->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size() * 4,
                                 Texture::T_float, Texture::F_rgba32);
  direct->set_clear_color(LColor(0, 0, 0, 0));
  direct->set_default_sampler(sampler);
  direct->set_compression(Texture::CM_off);
  direct->clear_image();
  _lm_textures["direct"] = direct;

  lightbuilder_cat.info()
    << "Lightmap is " << (float)direct->get_expected_ram_image_size() / 1000000.0f << " MB\n";

  PT(Texture) direct_dynamic = new Texture("lm_direct_dynamic");
  direct_dynamic->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                         Texture::T_float, Texture::F_rgba32);
  direct_dynamic->set_clear_color(LColor(0, 0, 0, 0));
  direct_dynamic->set_default_sampler(sampler);
  direct_dynamic->set_compression(Texture::CM_off);
  direct_dynamic->clear_image();
  _lm_textures["direct_dynamic"] = direct_dynamic;

  // Color of indirect lighting reaching a luxel.
  PT(Texture) indirect = new Texture("lm_indirect");
  indirect->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                   Texture::T_float, Texture::F_rgba32);
  indirect->set_clear_color(LColor(0, 0, 0, 0));
  indirect->set_default_sampler(sampler);
  indirect->set_compression(Texture::CM_off);
  indirect->clear_image();
  _lm_textures["indirect"] = indirect;

  // Intermediate indirect lighting accumulation.
  PT(Texture) indirect_accum = new Texture("lm_indirect_accum");
  indirect_accum->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                         Texture::T_float, Texture::F_rgba32);
  indirect_accum->set_clear_color(LColor(0, 0, 0, 0));
  indirect_accum->set_default_sampler(sampler);
  indirect_accum->set_compression(Texture::CM_off);
  indirect_accum->clear_image();
  _lm_textures["indirect_accum"] = indirect_accum;

  // Reflectivity = direct light * albedo + emission.  Color of light
  // being reflected off of a luxel.  Used during the bounce pass.
  // For each luxel, rays are sent off in random directions relative to
  // the surface normal.  If a ray hits a triangle, the reflectivity texture
  // it sampled to accumulate the indirect lighting color.
  PT(Texture) reflectivity = new Texture("lm_reflectivity");
  reflectivity->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size() * 4,
                                       Texture::T_float, Texture::F_rgba32);
  reflectivity->set_clear_color(LColor(0, 0, 0, 0));
  reflectivity->set_default_sampler(sampler);
  reflectivity->set_compression(Texture::CM_off);
  reflectivity->clear_image();
  _lm_textures["reflectivity"] = reflectivity;

  PT(Texture) emission = new Texture("lm_emission");
  emission->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                   Texture::T_float, Texture::F_rgba32);
  emission->set_clear_color(LColor(0, 0, 0, 0));
  emission->set_default_sampler(sampler);
  emission->set_compression(Texture::CM_off);
  emission->clear_image();
  _lm_textures["emission"] = emission;

  // This texture contains the output data for each ambient probe.
  // The data for each probe occupies nine texels for spherical harmonics.
  // The probe shader writes to this texture, which is then read back in on
  // the CPU to transfer to a friendly format.
  PT(Texture) probes = new Texture("lm_probes");
  probes->setup_buffer_texture(_probes.size() * 9, Texture::T_float, Texture::F_rgba32, GeomEnums::UH_static);
  probes->set_clear_color(LColor(0, 0, 0, 0));
  probes->set_default_sampler(sampler);
  probes->set_compression(Texture::CM_off);
  probes->clear_image();
  _lm_textures["probes"] = probes;

  PT(Texture) probes_flat = new Texture("lm_probes_flat");
  probes_flat->setup_buffer_texture(_probes.size(), Texture::T_float, Texture::F_rgba32, GeomEnums::UH_static);
  probes_flat->set_clear_color(LColor(0, 0, 0, 0));
  probes_flat->set_default_sampler(sampler);
  probes_flat->set_compression(Texture::CM_off);
  probes_flat->clear_image();
  _lm_textures["probes_flat"] = probes_flat;

  //
  // Rasterization outputs.
  //

  // Luxel base color/albedo.
  PT(Texture) albedo = new Texture("lm_albedo");
  albedo->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                 Texture::T_unsigned_byte, Texture::F_rgba);
  albedo->set_clear_color(LColor(1, 1, 1, 0));
  albedo->set_default_sampler(sampler);
  albedo->set_compression(Texture::CM_off);
  albedo->clear_image();
  _lm_textures["albedo"] = albedo;

  // Luxel world-space position.
  PT(Texture) position = new Texture("lm_position");
  position->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                   Texture::T_float, Texture::F_rgba32);
  position->set_clear_color(LColor(0, 0, 0, 0));
  position->set_default_sampler(sampler);
  position->set_compression(Texture::CM_off);
  position->clear_image();
  _lm_textures["position"] = position;

  // Luxel world-space normal.
  PT(Texture) normal = new Texture("lm_normal");
  normal->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                 Texture::T_float, Texture::F_rgb32);
  normal->set_clear_color(LColor(0, 0, 0, 0));
  normal->set_default_sampler(sampler);
  normal->set_compression(Texture::CM_off);
  normal->clear_image();
  _lm_textures["normal"] = normal;

  // Luxel unocclusion value.
  PT(Texture) unocclude = new Texture("lm_unocclude");
  unocclude->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                    Texture::T_float, Texture::F_rgba32);
  unocclude->set_clear_color(LColor(0, 0, 0, 0));
  unocclude->set_default_sampler(sampler);
  unocclude->set_compression(Texture::CM_off);
  unocclude->clear_image();
  _lm_textures["unocclude"] = unocclude;

  // Per-vertex albedo.
  PT(Texture) vtx_albedo = new Texture("vtx_albedo");
  vtx_albedo->setup_2d_texture(vtx_width, vtx_height, Texture::T_unsigned_byte, Texture::F_rgba);
  vtx_albedo->set_clear_color(LColor(1, 1, 1, 0));
  vtx_albedo->set_default_sampler(sampler);
  vtx_albedo->set_compression(Texture::CM_off);
  vtx_albedo->set_minfilter(SamplerState::FT_nearest);
  vtx_albedo->set_magfilter(SamplerState::FT_nearest);
  vtx_albedo->clear_image();
  _lm_textures["vtx_albedo"] = vtx_albedo;

  // Per-vertex reflectivity (direct light * albedo).
  PT(Texture) vtx_refl = new Texture("vtx_refl");
  vtx_refl->setup_2d_texture(vtx_width, vtx_height, Texture::T_float, Texture::F_rgba32);
  vtx_refl->set_clear_color(LColor(0, 0, 0, 0));
  vtx_refl->set_default_sampler(sampler);
  vtx_refl->set_compression(Texture::CM_off);
  vtx_refl->set_minfilter(SamplerState::FT_nearest);
  vtx_refl->set_magfilter(SamplerState::FT_nearest);
  vtx_refl->clear_image();
  _lm_textures["vtx_refl"] = vtx_refl;

  PT(Texture) vtx_refl_accum = new Texture("vtx_refl_accum");
  vtx_refl_accum->setup_2d_texture(vtx_width, vtx_height, Texture::T_float, Texture::F_rgba32);
  vtx_refl_accum->set_clear_color(LColor(0, 0, 0, 0));
  vtx_refl_accum->set_default_sampler(sampler);
  vtx_refl_accum->set_compression(Texture::CM_off);
  vtx_refl_accum->set_minfilter(SamplerState::FT_nearest);
  vtx_refl_accum->set_magfilter(SamplerState::FT_nearest);
  vtx_refl_accum->clear_image();
  _lm_textures["vtx_refl_accum"] = vtx_refl_accum;

  PT(Texture) vtx_light = new Texture("vtx_light");
  vtx_light->setup_2d_texture(vtx_width, vtx_height, Texture::T_float, Texture::F_rgba32);
  vtx_light->set_clear_color(LColor(0, 0, 0, 0));
  vtx_light->set_default_sampler(sampler);
  vtx_light->set_compression(Texture::CM_off);
  vtx_light->set_minfilter(SamplerState::FT_nearest);
  vtx_light->set_magfilter(SamplerState::FT_nearest);
  vtx_light->clear_image();
  _lm_textures["vtx_light"] = vtx_light;

  PT(Texture) vtx_light_dynamic = new Texture("vtx_light_dynamic");
  vtx_light_dynamic->setup_2d_texture(vtx_width, vtx_height, Texture::T_float, Texture::F_rgba32);
  vtx_light_dynamic->set_clear_color(LColor(0, 0, 0, 0));
  vtx_light_dynamic->set_default_sampler(sampler);
  vtx_light_dynamic->set_compression(Texture::CM_off);
  vtx_light_dynamic->set_minfilter(SamplerState::FT_nearest);
  vtx_light_dynamic->set_magfilter(SamplerState::FT_nearest);
  vtx_light_dynamic->clear_image();
  _lm_textures["vtx_light_dynamic"] = vtx_light_dynamic;

  return true;
}

class PosNormalPair {
public:
  LPoint3 pos;
  LVector3 normal;

  class Compare {
  public:
    bool operator () (const PosNormalPair &a, const PosNormalPair &b) const {
      int cmp = a.pos.compare_to(b.pos);
      if (cmp != 0) {
        return cmp < 0;
      }
      cmp = a.normal.compare_to(b.normal);
      if (cmp != 0) {
        return cmp < 0;
      }
      return false;
    }
  };
};

/**
 *
 */
void LightBuilder::
make_geom_vertices_and_triangles(LightmapGeom &geom, int &triangle, int &vertex) {
  geom.first_vertex = vertex;
  geom.first_triangle = triangle;
  geom.num_triangles = 0;
  geom.num_vertices = 0;

  LMatrix4 mat = geom.net_transform->get_mat();

  // Make a copy of the geom's vertex data containing only the vertices
  // referenced by the primitives of the geom.
  //PT(GeomVertexData) ref_vdata = new GeomVertexData(*geom.geom->get_vertex_data(), geom.geom->get_vertex_data()->get_format());

  //geom.ni_geom = new Geom(ref_vdata);
  //geom.ni_vdata = ref_vdata;

  pmap<int, int> index_remap;

  // Get the unique set of vertex indices referenced by the Geom.
  BitArray referenced_vertices;
  for (size_t i = 0; i < geom.geom->get_num_primitives(); ++i) {
    const GeomPrimitive *prim = geom.geom->get_primitive(i);
    GeomPrimitivePipelineReader prim_reader(prim, Thread::get_current_thread());
    prim_reader.get_referenced_vertices(referenced_vertices);
  }

  if (lightbuilder_cat.is_debug()) {
    lightbuilder_cat.debug()
      << referenced_vertices.get_num_on_bits() << " unique vertices for geom "
      << (int)(&geom - _geoms.data()) + 1 << " / " << _geoms.size() << "\n";
    lightbuilder_cat.debug()
      << "vdata has " << geom.geom->get_vertex_data()->get_num_rows() << " vertices\n";
    for (size_t i = 0; i < geom.geom->get_num_primitives(); ++i) {
      geom.geom->get_primitive(i)->write(lightbuilder_cat.debug(), 0);
    }
  }

  GeomVertexReader v_reader(geom.geom->get_vertex_data(), InternalName::get_vertex());
  GeomVertexReader n_reader(geom.geom->get_vertex_data(), InternalName::get_normal());
  GeomVertexReader luv_reader(geom.geom->get_vertex_data(), get_lightmap_uv_name());

  pmap<PosNormalPair, int, PosNormalPair::Compare> unique_positions;

  // Now copy in the vertex data for those vertices in increasing order.
  int index = referenced_vertices.get_lowest_on_bit();
  //int dest_row = 0;
  while (index >= 0) {
    //index_remap[index] = dest_row;
    //ref_vdata->copy_row_from(dest_row, geom.geom->get_vertex_data(), index, Thread::get_current_thread());

    // Now create a LightmapVertex for this vertex.

    LPoint3 pos;
    v_reader.set_row(index);
    pos = v_reader.get_data3f();

    LVector3 normal;
    n_reader.set_row(index);
    normal = n_reader.get_data3f();

    PosNormalPair pair;
    pair.pos = pos;
    pair.normal = normal;

    pmap<PosNormalPair, int, PosNormalPair::Compare>::const_iterator it = unique_positions.end();
    if (geom.light_mode == LightmapGeom::LM_per_vertex) {
      it = unique_positions.find(pair);
    }
    if (it != unique_positions.end()) {
      // We already have a vertex at this position.  Re-use it.
      index_remap[index] = (*it).second;

      _vertices[(*it).second].orig_vertices.insert(index);

    } else {
      unique_positions[pair] = (int)_vertices.size();

      LVecBase2 uv(0.0f, 0.0f);
      if (luv_reader.has_column()) {
        luv_reader.set_row(index);
        uv = luv_reader.get_data2f();
      }

      // Transform position and normal into world coordinates.
      pos = mat.xform_point(pos);
      normal = mat.xform_vec(normal).normalized();

      index_remap[index] = (int)_vertices.size();

      LightmapVertex l_vert;
      l_vert.pos = pos;
      l_vert.normal = normal;
      l_vert.uv = uv;
      l_vert.orig_vertex = index;
      l_vert.orig_vertices.insert(index);
      _vertices.push_back(std::move(l_vert));
      geom.num_vertices++;
    }

    //++dest_row;

    referenced_vertices.clear_bit(index);
    index = referenced_vertices.get_lowest_on_bit();
  }

  // Now build triangles and remapped GeomPrimitives.

  for (size_t i = 0; i < geom.geom->get_num_primitives(); ++i) {
    const GeomPrimitive *prim = geom.geom->get_primitive(i);
    //PT(GeomTriangles) nprim = new GeomTriangles(prim->get_usage_hint());
    for (size_t j = 0; j < prim->get_num_primitives(); ++j) {
      int start = prim->get_primitive_start(j);
      int end = prim->get_primitive_end(j);

      nassertv((end - start) == 3);

      LightmapTri tri;
      tri.mins = LPoint3(1e24);
      tri.maxs = LPoint3(-1e24);

      for (int k = start; k < end; ++k) {
        int vidx = prim->get_vertex(k);
        int remapped = index_remap[vidx];
        tri.indices[k - start] = remapped;

        const LightmapVertex &lvert = _vertices[remapped];
        tri.mins = tri.mins.fmin(lvert.pos);
        tri.maxs = tri.maxs.fmax(lvert.pos);
      }

      tri.palette = geom.palette;
      tri.contents = geom.contents;

      _triangles.push_back(std::move(tri));
      geom.num_triangles++;

      //nprim->close_primitive();
    }

    //geom.ni_geom->add_primitive(nprim);
  }

  geom.ni_geom = geom.geom->make_copy();

  // Increment vertex and triangle offsets for next Geom.
  vertex += geom.num_vertices;
  triangle += geom.num_triangles;
}

/**
 * Collects all of the vertices and triangles from the Geoms in the scene into
 * structures we can easily read from and ship off to the GPU.
 */
bool LightBuilder::
collect_vertices_and_triangles() {
  lightbuilder_cat.info()
    << "Collecting vertices and triangles\n";

  int vertex = 0;
  int triangle = 0;

  // Go in order of each page so everything is automatically nicely
  // sorted by page.

  for (size_t i = 0; i < _pages.size(); i++) {
    LightmapPage &page = _pages[i];

    for (size_t j = 0; j < page.geoms.size(); j++) {
      int geom_index = page.geoms[j];
      LightmapGeom &geom = _geoms[geom_index];
      make_geom_vertices_and_triangles(geom, triangle, vertex);
    }
  }

  _first_vertex_lit_tri = triangle;
  _first_vertex_lit_vertex = vertex;
  _num_vertex_lit_tris = 0;
  _num_vertex_lit_geoms = 0;
  _num_vertex_lit_vertices = 0;

  // Now do the vertex-lit geoms.
  for (size_t i = 0; i < _geoms.size(); ++i) {
    LightmapGeom &geom = _geoms[i];
    if (geom.light_mode != LightmapGeom::LM_per_vertex) {
      continue;
    }
    if (_first_vertex_lit_geom < 0) {
      _first_vertex_lit_geom = (int)i;
    }
    _num_vertex_lit_tris += geom.num_triangles;
    ++_num_vertex_lit_geoms;
    int orig = vertex;
    make_geom_vertices_and_triangles(geom, triangle, vertex);
    int diff = vertex - orig;
    _num_vertex_lit_vertices += diff;
  }

  // Now do occluder (not lightmapped but blocks light) triangles.
  for (size_t i = 0; i < _occluder_tris.size(); ++i) {
    const OccluderTri &otri = _occluder_tris[i];

    LightmapVertex v;
    v.pos = otri.a;
    _vertices.push_back(v);
    v.pos = otri.b;
    _vertices.push_back(v);
    v.pos = otri.c;
    _vertices.push_back(v);

    LPoint3 tmins(1e24);
    LPoint3 tmaxs(-1e24);
    tmins = tmins.fmin(otri.a);
    tmins = tmins.fmin(otri.b);
    tmins = tmins.fmin(otri.c);
    tmaxs = tmaxs.fmax(otri.a);
    tmaxs = tmaxs.fmax(otri.b);
    tmaxs = tmaxs.fmax(otri.c);

    LightmapTri tri;
    tri.palette = -1;
    tri.contents = otri.contents;
    tri.indices[0] = vertex;
    tri.indices[1] = vertex + 1;
    tri.indices[2] = vertex + 2;
    tri.mins = tmins;
    tri.maxs = tmaxs;

    _triangles.push_back(std::move(tri));

    vertex += 3;
    ++triangle;
  }

  lightbuilder_cat.info()
    << _vertices.size() << " vertices, " << _triangles.size() << " triangles\n";

  return true;
}

/**
 * Creates the set of GPU buffers that contain the data for all of the
 * vertices, triangles, and lights in the scene, as well as the triangle
 * K-D tree for ray tracing acceleration.  They are created as buffer textures
 * instead of SSBOs because 1) they are an older feature that more GPUs support
 * and 2) SSBOs are currently not supported on the shaderpipeline branch.
 * These buffers will be passed to the GPU to be accessed in shaders.
 *
 * The data in each buffer is packed as tightly as possible to minimize
 * the number of texels needed for each element.
 */
bool LightBuilder::
make_gpu_buffers() {
  lightbuilder_cat.info()
    << "Building GPU buffers\n";

  size_t total_buffer_size = 0u;

  // Vertex buffer texture.  Two four-channel texels per vertex.
  PT(Texture) vertices = new Texture("lm_vertices");
  vertices->setup_buffer_texture(_vertices.size() * 2, Texture::T_float,
                                 Texture::F_rgba32, GeomEnums::UH_static);
  vertices->set_keep_ram_image(false);
  PTA_uchar vertex_data;
  vertex_data.resize(sizeof(float) * _vertices.size() * 8);
  nassertr(vertices->get_expected_ram_image_size() == vertex_data.size(), false);
  float *vertex_datap = (float *)vertex_data.p();
  for (size_t i = 0; i < _vertices.size(); i++) {
    const LightmapVertex &vtx = _vertices[i];

    // Texel 0: x, y, z, u
    vertex_datap[i * 8] = vtx.pos[0];
    vertex_datap[i * 8 + 1] = vtx.pos[1];
    vertex_datap[i * 8 + 2] = vtx.pos[2];
    vertex_datap[i * 8 + 3] = vtx.uv[0];

    // Texel 1: nx, ny, nz, v
    vertex_datap[i * 8 + 4] = vtx.normal[0];
    vertex_datap[i * 8 + 5] = vtx.normal[1];
    vertex_datap[i * 8 + 6] = vtx.normal[2];
    vertex_datap[i * 8 + 7] = vtx.uv[1];
  }
  vertices->set_ram_image(vertex_data);
  _gpu_buffers["vertices"] = vertices;

  lightbuilder_cat.info()
    << "Vertex buffer is " << (float)vertex_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += vertex_data.size();

  // Triangle buffer texture.  Three four-channel texels per triangle.
  PT(Texture) triangles = new Texture("lm_triangles");
  triangles->setup_buffer_texture(_triangles.size() * 3, Texture::T_float,
                                  Texture::F_rgba32, GeomEnums::UH_static);
  triangles->set_keep_ram_image(false);
  PTA_uchar tri_data;
  tri_data.resize(sizeof(float) * _triangles.size() * 12);
  nassertr(triangles->get_expected_ram_image_size() == tri_data.size(), false);
  float *tri_datap = (float *)tri_data.p();
  for (size_t i = 0; i < _triangles.size(); i++) {
    const LightmapTri &tri = _triangles[i];

    // Texel 0: v1, v2, v3, palette index
    tri_datap[i * 12] = tri.indices[0];
    tri_datap[i * 12 + 1] = tri.indices[1];
    tri_datap[i * 12 + 2] = tri.indices[2];
    tri_datap[i * 12 + 3] = 0.0f;

    // Texel 1: minx, miny, minz, contents
    tri_datap[i * 12 + 4] = tri.mins[0];
    tri_datap[i * 12 + 5] = tri.mins[1];
    tri_datap[i * 12 + 6] = tri.mins[2];
    tri_datap[i * 12 + 7] = tri.contents;

    // Texel 2: maxx, maxy, maxz, unused
    tri_datap[i * 12 + 8] = tri.maxs[0];
    tri_datap[i * 12 + 9] = tri.maxs[1];
    tri_datap[i * 12 + 10] = tri.maxs[2];
    tri_datap[i * 12 + 11] = tri.palette;
  }
  triangles->set_ram_image(tri_data);
  _gpu_buffers["triangles"] = triangles;

  lightbuilder_cat.info()
    << "Triangle buffer is " << (float)tri_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += tri_data.size();

  // Light buffer texture.  Five four-channel texels per light.
  PT(Texture) lights = new Texture("lm_lights");
  lights->setup_buffer_texture(_lights.size() * 5, Texture::T_float,
                               Texture::F_rgba32, GeomEnums::UH_static);
  lights->set_keep_ram_image(false);
  PTA_uchar light_data;
  light_data.resize(sizeof(float) * _lights.size() * 20);
  nassertr(lights->get_expected_ram_image_size() == light_data.size(), false);
  float *light_datap = (float *)light_data.p();
  for (size_t i = 0; i < _lights.size(); i++) {
    const LightmapLight &light = _lights[i];

    LQuaternion quat;
    quat.set_hpr(light.hpr);
    LVector3 dir = quat.get_forward();

    float stopdot = std::cos(deg_2_rad((float)light.inner_cone));
    float stopdot2 = std::cos(deg_2_rad((float)light.outer_cone));
    float oodot = (stopdot > stopdot2) ? (1.0f / (stopdot - stopdot2)) : 0.0f;

    // Texel 0: type, constant, linear, quadratic
    light_datap[i * 20] = light.type;
    light_datap[i * 20 + 1] = light.constant;
    light_datap[i * 20 + 2] = light.linear;
    light_datap[i * 20 + 3] = light.quadratic;

    // Texel 1: r, g, b, bake direct
    light_datap[i * 20 + 4] = light.color[0];
    light_datap[i * 20 + 5] = light.color[1];
    light_datap[i * 20 + 6] = light.color[2];
    light_datap[i * 20 + 7] = light.bake_direct;

    // Texel 2: x, y, z, unused
    light_datap[i * 20 + 8] = light.pos[0];
    light_datap[i * 20 + 9] = light.pos[1];
    light_datap[i * 20 + 10] = light.pos[2];
    light_datap[i * 20 + 11] = 0.0f;

    // Texel 3: dirx, diry, dirz, unused
    light_datap[i * 20 + 12] = dir[0];
    light_datap[i * 20 + 13] = dir[1];
    light_datap[i * 20 + 14] = dir[2];
    light_datap[i * 20 + 15] = 0.0f;

    // Texel 4: exponent, stopdot, stopdot2, oodot
    light_datap[i * 20 + 16] = light.exponent;
    light_datap[i * 20 + 17] = stopdot;
    light_datap[i * 20 + 18] = stopdot2;
    light_datap[i * 20 + 19] = oodot;
  }
  lights->set_ram_image(light_data);
  _gpu_buffers["lights"] = lights;

  lightbuilder_cat.info()
    << "Light buffer is " << (float)light_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += light_data.size();

  // Buffer of ambient probe positions.  One three-channel texel per probe.
  PT(Texture) probes = new Texture("lm_probes");
  probes->setup_buffer_texture(_probes.size(), Texture::T_float,
                               Texture::F_rgb32, GeomEnums::UH_static);
  probes->set_keep_ram_image(false);
  PTA_uchar probe_data;
  probe_data.resize(sizeof(float) * _probes.size() * 3);
  nassertr(probes->get_expected_ram_image_size() == probe_data.size(), false);
  float *probe_datap = (float *)probe_data.p();
  for (size_t i = 0; i < _probes.size(); i++) {
    const LightmapAmbientProbe &probe = _probes[i];
    probe_datap[i * 3] = probe.pos[0];
    probe_datap[i * 3 + 1] = probe.pos[1];
    probe_datap[i * 3 + 2] = probe.pos[2];
  }
  probes->set_ram_image(probe_data);
  _gpu_buffers["probes"] = probes;

  lightbuilder_cat.info()
    << "Probe position buffer is " << (float)probe_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += probe_data.size();


  lightbuilder_cat.info()
    << "K-D tree buffer texture is " << _kd_node_count * 5 << " RGBA32 texels\n";

  // Count number of non-leaf nodes.
  int node_count = 0;
  int leaf_count = 0;
  KDNode *pnode;
  for (pnode = _kd_tree_head; pnode != nullptr; pnode = pnode->next) {
    if (pnode->is_leaf()) {
      pnode->index = ~leaf_count;
      ++leaf_count;
    } else {
      pnode->index = node_count + 1;
      ++node_count;
    }
  }

  PT(Texture) kd_tree = new Texture("kd_tree");
  kd_tree->setup_buffer_texture(node_count, Texture::T_float,
                                Texture::F_rgba32, GeomEnums::UH_static);
  kd_tree->set_compression(Texture::CM_off);
  kd_tree->set_keep_ram_image(false);
  PTA_uchar kd_tree_data;
  kd_tree_data.resize(sizeof(float) * node_count * 4);
  nassertr(kd_tree_data.size() == kd_tree->get_expected_ram_image_size(), false);
  float *kd_tree_datap = (float *)kd_tree_data.p();

  size_t node_num = 0;
  for (pnode = _kd_tree_head; pnode != nullptr; pnode = pnode->next) {
    const KDNode &node = *pnode;

    if (node.is_leaf()) {
      continue;
    }

    assert(node.index == (node_num + 1));

    // Texel 0: Children indices, splitting plane.
    kd_tree_datap[node_num * 4] = node.get_child_node_index(0);
    kd_tree_datap[node_num * 4 + 1] = node.get_child_node_index(1);
    kd_tree_datap[node_num * 4 + 2] = node.axis;
    kd_tree_datap[node_num * 4 + 3] = node.dist;

    ++node_num;
  }
  kd_tree->set_ram_image(kd_tree_data);
  _gpu_buffers["kd_tree"] = kd_tree;

  lightbuilder_cat.info()
    << "K-D tree buffer is " << (float)kd_tree_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += kd_tree_data.size();

  lightbuilder_cat.info()
    << "K-D leaves buffer texture is " << leaf_count * 4 << " RGBA32 texels\n";

  PT(Texture) kd_leaves = new Texture("kd_leaves");
  kd_leaves->setup_buffer_texture(leaf_count * 4, Texture::T_float,
                                  Texture::F_rgba32, GeomEnums::UH_static);
  kd_leaves->set_compression(Texture::CM_off);
  kd_leaves->set_keep_ram_image(false);
  PTA_uchar kd_leaves_data;
  kd_leaves_data.resize(sizeof(float) * leaf_count * 16);
  nassertr(kd_leaves_data.size() == kd_leaves->get_expected_ram_image_size(), false);
  float *kd_leaves_datap = (float *)kd_leaves_data.p();
  int leaf_num = 0;
  for (pnode = _kd_tree_head; pnode != nullptr; pnode = pnode->next) {
    const KDNode &node = *pnode;
    if (!node.is_leaf()) {
      continue;
    }

    assert(node.index == ~leaf_num);

    // Texel 1: mins, unused
    kd_leaves_datap[leaf_num * 16] = node.mins[0];
    kd_leaves_datap[leaf_num * 16 + 1] = node.mins[1];
    kd_leaves_datap[leaf_num * 16 + 2] = node.mins[2];
    kd_leaves_datap[leaf_num * 16 + 3] = 0.0f;

    // Texel 2: maxs, unused
    kd_leaves_datap[leaf_num * 16 + 4] = node.maxs[0];
    kd_leaves_datap[leaf_num * 16 + 5] = node.maxs[1];
    kd_leaves_datap[leaf_num * 16 + 6] = node.maxs[2];
    kd_leaves_datap[leaf_num * 16 + 7] = 0.0f;

    // Texel 3: left, right, front, back neighbors
    kd_leaves_datap[leaf_num * 16 + 8] = node.get_neighbor_node_index(0);
    kd_leaves_datap[leaf_num * 16 + 9] = node.get_neighbor_node_index(1);
    kd_leaves_datap[leaf_num * 16 + 10] = node.get_neighbor_node_index(2);
    kd_leaves_datap[leaf_num * 16 + 11] = node.get_neighbor_node_index(3);

    // Texel 4: bottom, top neighbors, first tri, num tris
    kd_leaves_datap[leaf_num * 16 + 12] = node.get_neighbor_node_index(4);
    kd_leaves_datap[leaf_num * 16 + 13] = node.get_neighbor_node_index(5);
    kd_leaves_datap[leaf_num * 16 + 14] = node.first_triangle;
    kd_leaves_datap[leaf_num * 16 + 15] = node.num_triangles;

    leaf_num++;
  }
  kd_leaves->set_ram_image(kd_leaves_data);
  _gpu_buffers["kd_leaves"] = kd_leaves;

  lightbuilder_cat.info()
    << "K-D leaves buffer is " << (float)kd_leaves_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += kd_leaves_data.size();

  free_kd_tree();

#if 1
  lightbuilder_cat.info()
    << "K-D triangle list buffer texture is " << _kd_tri_list.size() << " R32I texels\n";
  PT(Texture) kd_tri_list = new Texture("kd_tri_list");
  kd_tri_list->setup_buffer_texture(_kd_tri_list.size(), Texture::T_unsigned_int,
                                    Texture::F_r32i, GeomEnums::UH_static);
  kd_tri_list->set_compression(Texture::CM_off);
  kd_tri_list->set_keep_ram_image(false);
  PTA_uchar kd_tri_data;
  kd_tri_data.resize(sizeof(unsigned int) * _kd_tri_list.size());
  nassertr(kd_tri_data.size() == kd_tri_list->get_expected_ram_image_size(), false);
  memcpy(kd_tri_data.p(), _kd_tri_list.data(), sizeof(unsigned int) * _kd_tri_list.size());
  kd_tri_list->set_ram_image(kd_tri_data);
  _gpu_buffers["kd_tri_list"] = kd_tri_list;

  lightbuilder_cat.info()
    << "K-D triangle list buffer is " << (float)kd_tri_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += kd_tri_data.size();
#else

  PT(Texture) kd_tri_list = new Texture("kd_tri_list");
  kd_tri_list->setup_buffer_texture(_kd_tri_list.size() * 3, Texture::T_float,
                                    Texture::F_rgba32, GeomEnums::UH_static);
  kd_tri_list->set_compression(Texture::CM_off);
  kd_tri_list->set_keep_ram_image(false);
  PTA_uchar kd_tri_data;
  kd_tri_data.resize(sizeof(float) * _kd_tri_list.size() * 3 * 4);
  nassertr(kd_tri_data.size() == kd_tri_list->get_expected_ram_image_size(), false);

  size_t li = 0;
  for (unsigned int tri_index : _kd_tri_list) {
    const LightmapTri &tri = _triangles[tri_index];
    const LightmapVertex &va = _vertices[tri.indices[0]];
    const LightmapVertex &vb = _vertices[tri.indices[1]];
    const LightmapVertex &vc = _vertices[tri.indices[2]];

    kd_tri_data[li++] = va.pos[0];
    kd_tri_data[li++] = va.pos[1];
    kd_tri_data[li++] = va.pos[2];
    kd_tri_data[li++] = tri_index;

    kd_tri_data[li++] = vb.pos[0];
    kd_tri_data[li++] = vb.pos[1];
    kd_tri_data[li++] = vb.pos[2];
    kd_tri_data[li++] = tri.contents;

    kd_tri_data[li++] = vc.pos[0];
    kd_tri_data[li++] = vc.pos[1];
    kd_tri_data[li++] = vc.pos[2];
    kd_tri_data[li++] = tri.palette;
  }

  kd_tri_list->set_ram_image(kd_tri_data);
  _gpu_buffers["kd_tri_list"] = kd_tri_list;

  lightbuilder_cat.info()
    << "K-D triangle list buffer is " << (float)kd_tri_data.size() / 1000000.0f << " MB\n";
  total_buffer_size += kd_tri_data.size();

#endif

  lightbuilder_cat.info()
    << "Total size of all buffers: " << (float)total_buffer_size / 1000000.0f << " MB\n";

  _kd_tri_list.clear();

  return true;
}

static constexpr int kd_max_triangles = 10;
static constexpr int kd_max_depth = 28;

/**
 *
 */
bool LightBuilder::
build_kd_tree() {
  lightbuilder_cat.info()
    << "Building ray tracing acceleration structure...\n";

  // First, determine the scene AABB.  This is the union of all triangle
  // AABBs.
  _scene_mins.set(1e24, 1e24, 1e24);
  _scene_maxs.set(-1e24, -1e24, -1e24);
  for (size_t i = 0; i < _triangles.size(); i++) {
    const LightmapTri &tri = _triangles[i];
    _scene_mins[0] = std::min(_scene_mins[0], tri.mins[0]);
    _scene_mins[1] = std::min(_scene_mins[1], tri.mins[1]);
    _scene_mins[2] = std::min(_scene_mins[2], tri.mins[2]);
    _scene_maxs[0] = std::max(_scene_maxs[0], tri.maxs[0]);
    _scene_maxs[1] = std::max(_scene_maxs[1], tri.maxs[1]);
    _scene_maxs[2] = std::max(_scene_maxs[2], tri.maxs[2]);
  }

  lightbuilder_cat.info()
    << "Scene mins: " << _scene_mins << ", maxs: " << _scene_maxs << "\n";

  vector_int tmp;
  tmp.resize(_triangles.size());

  _kd_node_count = 0;
  _kd_tree_head = nullptr;
  _kd_tree_tail = nullptr;

  lightbuilder_cat.info()
    << "Building tree split candidates\n";

  KDSplits splits[3];
  for (unsigned char axis = 0; axis < 3; ++axis) {
    splits[axis].reserve(_triangles.size() * 2);
    for (size_t i = 0; i < _triangles.size(); ++i) {
      const LightmapTri &tri = _triangles[i];
      splits[axis].push_back({ (int)i, tri.mins[axis], axis, KDSplit::S_min });
      splits[axis].push_back({ (int)i, tri.maxs[axis], axis, KDSplit::S_max });
    }
    // Now sort splits along axis by increasing distance.
    // This also sorts the triangles.
    std::sort(splits[axis].begin(), splits[axis].end());
  }

  lightbuilder_cat.info()
    << "Constructing tree\n";
  r_build_kd_tree(splits, (int)_triangles.size(), _scene_mins, _scene_maxs, 0, 1e24, tmp);

  nassertr(_kd_tree_head != nullptr && _kd_tree_head->id == 0, false);

  lightbuilder_cat.info()
    << "Building leaf triangle list\n";
  for (KDNode *node = _kd_tree_head; node != nullptr; node = node->next) {
    if (node->is_leaf()) {
      node->first_triangle = (int)_kd_tri_list.size();
      node->num_triangles = (int)node->triangles.size();
      for (int itri : node->triangles) {
        _kd_tri_list.push_back(itri);
      }
    }
  }

  lightbuilder_cat.info()
    << "Building K-D tree ropes\n";
  if (_kd_tree_head != nullptr) {
    KDNode *rope[6];
    for (int i = 0; i < 6; ++i) {
      rope[i] = nullptr;
    }
    r_build_kd_ropes(_kd_tree_head, rope);
  }

  lightbuilder_cat.info()
    << "Done.\n";

  //if (_kd_tree_head != nullptr) {
  //  r_kd_node_output(_kd_tree_head, 0);
  //}

  return true;
}

// x y
// y z
// z x

/**
 *
 */
static float
box_half_surf_area(const LPoint3 &mins, const LPoint3 &maxs) {
  LVector3 delta = maxs - mins;
  return (delta[0] * delta[1] + delta[1] * delta[2] + delta[2] * delta[0]);
}

static constexpr float cost_traverse = 1.0f;
static constexpr float cost_intersect = 1.0f;

/**
 *
 */
void LightBuilder::
make_kd_leaf(KDNode *node, const KDSplits *splits, int num_tris) {
  LPoint3 center = (node->mins + node->maxs) * 0.5f;
  LVector3 half = (node->maxs - node->mins) * 0.5f;
  half += LVector3(0.01f);

  for (int i = 0; i < (int)splits[0].size(); ++i) {
    if (splits[0][i].split == KDSplit::S_min) {
      const LightmapTri &tri = _triangles[splits[0][i].triangle];
      if (tri_box_overlap(center, half, _vertices[tri.indices[0]].pos,
                          _vertices[tri.indices[1]].pos, _vertices[tri.indices[2]].pos)) {
        node->triangles.push_back(splits[0][i].triangle);
      }
    }
  }
}

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

#define CHILD_FRONT 1
#define CHILD_BACK 0

/**
 * Creates a new KDNode, adds it to the linked list, and returns the
 * new node.
 */
LightBuilder::KDNode *LightBuilder::
alloc_kd_node() {
  KDNode *node = new KDNode;
  add_kd_node(node);
  return node;
}

/**
 *
 */
void LightBuilder::
free_kd_tree() {
  KDNode *node = _kd_tree_head;
  while (node != nullptr) {
    KDNode *next = node->next;
    delete node;
    node = next;
  }

  _kd_tree_tail = nullptr;
  _kd_tree_head = nullptr;
}

/**
 * Appends the indicated KDNode to the tail of the linked list.
 */
void LightBuilder::
add_kd_node(KDNode *node) {
  nassertv(node->id < 0);

  node->id = _kd_node_count++;

  if (_kd_tree_head == nullptr) {
    // First node in list.
    _kd_tree_head = node;
    _kd_tree_tail = node;

  } else {
    // Adding new node to end of existing list.
    nassertv(_kd_tree_tail != nullptr);
    _kd_tree_tail->next = node;
    _kd_tree_tail = node;
  }
}

/**
 *
 */
LightBuilder::KDNode *LightBuilder::
r_build_kd_tree(const KDSplits *splits, int num_tris, const LPoint3 &mins, const LPoint3 &maxs,
                int depth, float parent_sah, vector_int &tmp) {

  KDNode *node = alloc_kd_node();
  node->mins = mins;
  node->maxs = maxs;

  if (num_tris < kd_max_triangles || depth >= kd_max_depth) {
    // Stop here.  Leaf node with triangles.
    make_kd_leaf(node, splits, num_tris);
    return node;
  }

  //float my_sah = triangles.size() * box_half_surf_area(mins, maxs);

  // Otherwise we need to split somewhere.
  // Create an interior node.
  vector_int front, back;
  unsigned char axis;
  int split_index;
  float sah = split_triangles(splits, num_tris, axis, split_index, mins, maxs);
  if (sah >= cost_intersect * num_tris) {
    // Stop here.  Leaf node with triangles.
    make_kd_leaf(node, splits, num_tris);
    return node;
  }
  float dist = splits[axis][split_index].dist;
  node->axis = axis;
  node->dist = dist;

  //
  int split_count = num_tris * 2;
  const KDSplits &asplits = splits[axis];
  int num_l = 0, num_r = 0;
  for (int i = 0; i < split_count; ++i) {
    tmp[asplits[i].triangle] = 0;
  }
  for (int i = 0; i < split_count; ++i) {
    if (i < split_index) {
      if (asplits[i].split == KDSplit::S_min) {
        tmp[asplits[i].triangle] |= 1;
        ++num_l;
      }

    } else if (i > split_index) {
      if (asplits[i].split == KDSplit::S_max) {
        tmp[asplits[i].triangle] |= 2;
        ++num_r;
      }
    }
  }

  KDSplits l_splits[3];
  KDSplits r_splits[3];

  for (unsigned char k = 0; k < 3; ++k) {
    l_splits[k].resize(num_l * 2);
    r_splits[k].resize(num_r * 2);
  }
  for (unsigned char k = 0; k < 3; ++k) {
    int l_offset = 0, r_offset = 0;
    for (int i = 0; i < split_count; ++i) {
      const KDSplit &old = splits[k][i];
      int tri = old.triangle;
      if (tmp[tri] & 0x01) {
        l_splits[k][l_offset++] = old;
      }
      if (tmp[tri] & 0x02) {
        r_splits[k][r_offset++] = old;
      }
    }
  }

  // Now create children.

  // Clip maxs of back node to split plane.
  LPoint3 cmins = mins;
  LPoint3 cmaxs = maxs;
  cmaxs[axis] = dist;
  node->children[CHILD_BACK] = r_build_kd_tree(l_splits, num_l, cmins, cmaxs, depth + 1, sah, tmp);

  // Clip mins of front node to split plane.
  cmins = mins;
  cmaxs = maxs;
  cmins[axis] = dist;
  node->children[CHILD_FRONT] = r_build_kd_tree(r_splits, num_r, cmins, cmaxs, depth + 1, sah, tmp);

  return node;
}

static float
lambda(int num_l, int num_r, float prob_l, float prob_r) {
  if ((num_l == 0 || num_r == 0) &&
      !(prob_l == 1 || prob_r == 1)) {
    return 0.8f;
  }
  return 1.0f;
}

/**
 *
 */
static float
calc_sah(int l, int r, unsigned char axis, float dist, const LPoint3 &mins, const LPoint3 &maxs) {
  float inv_surf_area = 1.0f / box_half_surf_area(mins, maxs);
  LVector3 delta = maxs - mins;

  delta[axis] = dist - mins[axis];
  float left_surf_area = (delta[0] * delta[1] + delta[1] * delta[2] + delta[2] * delta[0]);

  delta[axis] = maxs[axis] - dist;
  float right_surf_area = (delta[0] * delta[1] + delta[1] * delta[2] + delta[2] * delta[0]);

  return (l * left_surf_area + r * right_surf_area) * inv_surf_area;
}

/**
 *
 */
float LightBuilder::
split_triangles(const KDSplits *splits, int num_tris, unsigned char &axis, int &split,
                const LPoint3 &mins, const LPoint3 &maxs) {
  // Possible split positions are for each triangle:
  // Tri AABB min X / max X
  // Tri AABB min Y / max Y
  // Tri AABB min Z / max Z

  float min_sah = 1e24;

  for (unsigned char j = 0; j < 3; ++j) { // Each possible axis.

    int num_l = 0;
    int num_r = num_tris;
    int split_count = num_tris * 2;
    int i = 0;

    while (i < split_count) {
      const KDSplit &ksplit = splits[j][i];

      float cand_dist = ksplit.dist;

      // Reject split if node will be flat on an axis.
      float left_delta = cand_dist - mins[j];
      float right_delta = maxs[j] - cand_dist;

      if (left_delta <= 0.01f || right_delta <= 0.01f) {
        ++i;
        continue;
      }

      if (cand_dist <= mins[j]) {
        ++i;
        continue;
      }
      if (cand_dist >= maxs[j]) {
        break;
      }

      if (ksplit.split == KDSplit::S_max) {
        --num_r;
      }

      // Compute surface area heuristic here.
      float sah = calc_sah(num_l, num_r, j, cand_dist, mins, maxs);
      if (sah < min_sah) {
        min_sah = sah;
        axis = j;
        split = i;
      }

      if (ksplit.split == KDSplit::S_min) {
        ++num_l;
      }

      ++i;
    }
  }

  return min_sah;
}

/**
 *
 */
void LightBuilder::
r_kd_node_output(const KDNode *node, int indent_level) {
  indent(std::cout, indent_level) << "mins " << node->mins << "\n";
  indent(std::cout, indent_level) << "maxs " << node->maxs << "\n";
  if (!node->is_leaf()) {
    indent(std::cout, indent_level) << "split " << (int)node->axis << " : " << node->dist << "\n";
    indent(std::cout, indent_level) << "back child " << node->children[0] << "\n";
    if (node->children[0] != nullptr) {
      r_kd_node_output(node->children[0], indent_level + 2);
    }
    indent(std::cout, indent_level) << "front child " << node->children[1] << "\n";
    if (node->children[1] != nullptr) {
      r_kd_node_output(node->children[1], indent_level + 2);
    }

  } else {
    indent(std::cout, indent_level) << "tri list: ";
    for (int itri : node->triangles) {
      std::cout << itri << " ";
    }
    std::cout << "\n";
    indent(std::cout, indent_level) << "ropes: ";
    for (int i = 0; i < 6; ++i) {
      std::cout << node->neighbors[i] << " ";
    }
    std::cout << "\n";
  }
}

#define KD_EPSILON 0.0001f

/**
 *
 */
void LightBuilder::
optimize_ropes(KDNode *rope[6], const LPoint3 &mins, const LPoint3 &maxs) {
  for (int i = 0; i < 6; ++i) {

    KDNode *rope_node = rope[i];
    if (rope_node == nullptr) {
      continue;
    }

    while (!rope_node->is_leaf()) {
      if (i == KDNode::F_left || i == KDNode::F_right) {

        // Handle parallel split plane case.
        if (rope_node->axis == AXIS_X) {
          rope_node = (i == KDNode::F_left) ? rope_node->children[CHILD_FRONT] : rope_node->children[CHILD_BACK];

        } else if (rope_node->axis == AXIS_Y) {
          if (rope_node->dist < (mins[AXIS_Y] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_FRONT];

          } else if (rope_node->dist > (maxs[AXIS_Y] + KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_BACK];

          } else {
            break;
          }

        } else {
          // Z axis.
          if (rope_node->dist < (mins[AXIS_Z] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_FRONT];

          } else if (rope_node->dist > (maxs[AXIS_Z] + KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_BACK];

          } else {
            break;
          }
        }

      } else if (i == KDNode::F_back || i == KDNode::F_front) {

        // Handle parallel split plane case.
        if (rope_node->axis == AXIS_Y) {
          rope_node = (i == KDNode::F_back) ? rope_node->children[CHILD_FRONT] : rope_node->children[CHILD_BACK];

        } else if (rope_node->axis == AXIS_X) {
          if (rope_node->dist < (mins[AXIS_X] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_FRONT];

          } else if (rope_node->dist > (maxs[AXIS_X] + KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_BACK];

          } else {
            break;
          }

        } else {
          // Z axis.
          if (rope_node->dist < (mins[AXIS_Z] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_FRONT];

          } else if (rope_node->dist > (maxs[AXIS_Z] + KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_BACK];

          } else {
            break;
          }
        }

      } else {
        // Top and bottom.

        // Handle parallel split plane case.
        if (rope_node->axis == AXIS_Z) {
          rope_node = (i == KDNode::F_bottom) ? rope_node->children[CHILD_FRONT] : rope_node->children[CHILD_BACK];

        } else if (rope_node->axis == AXIS_X) {
          if (rope_node->dist < (mins[AXIS_X] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_FRONT];

          } else if (rope_node->dist > (maxs[AXIS_X] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_BACK];

          } else {
            break;
          }

        } else {
          // Y axis.
          if (rope_node->dist < (mins[AXIS_Y] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_FRONT];

          } else if (rope_node->dist > (maxs[AXIS_Y] - KD_EPSILON)) {
            rope_node = rope_node->children[CHILD_BACK];

          } else {
            break;
          }
        }

      }
    }

    rope[i] = rope_node;
  }
}

/**
 *
 */
void LightBuilder::
r_build_kd_ropes(KDNode *node, KDNode *rope[6]) {
  if (node->is_leaf()) {
    for (int i = 0; i < 6; ++i) {
      node->neighbors[i] = rope[i];
    }

  } else {
    //for (int i = 0; i < 6; ++i) {
    //  if (rope[i] >= 0) {
    //    optimize_rope(node, rope[i], i);
    //  }
    //}

    optimize_ropes(rope, node->mins, node->maxs);

    KDNode::Face sb, sf;

    if (node->axis == 0) {
      // X axis
      sb = KDNode::F_left;
      sf = KDNode::F_right;

    } else if (node->axis == 1) {
      // Y axis
      sb = KDNode::F_back;
      sf = KDNode::F_front;

    } else {
      // Z axis.
      sb = KDNode::F_bottom;
      sf = KDNode::F_top;
    }

    KDNode *rope_b[6];
    KDNode *rope_f[6];
    for (int i = 0; i < 6; ++i) {
      rope_b[i] = rope[i];
      rope_f[i] = rope[i];
    }

    rope_b[sf] = node->children[CHILD_FRONT];
    r_build_kd_ropes(node->children[CHILD_BACK], rope_b);

    rope_f[sb] = node->children[CHILD_BACK];
    r_build_kd_ropes(node->children[CHILD_FRONT], rope_f);
  }
}

/**
 * This is the first step of the actual lightmap computation on the GPU side.
 * Every Geom in the scene is rasterized into lightmap space textures for
 * use in future steps.  It records the albedo, world-space position, and
 * world-space normal at each luxel, which is required to do lighting.
 */
bool LightBuilder::
rasterize_geoms_into_lightmap_textures() {
  // Create an FBO with framebuffer attachments to render the Geoms with.

  lightbuilder_cat.info()
    << "Rasterizing Geoms into lightmap textures\n";

  WindowProperties winprops;
  winprops.clear();
  winprops.set_size(_lightmap_size);

  FrameBufferProperties fbprops;
  fbprops.clear();
  fbprops.set_aux_float(4);
  fbprops.set_rgba_bits(8, 8, 8, 8);
  fbprops.set_force_hardware(true);
  fbprops.set_srgb_color(false);
  //fbprops.set_multisamples(16);

  unsigned int flags = GraphicsPipe::BF_refuse_window;

  PT(GraphicsOutput) buffer = _graphics_engine->make_output(
    _graphics_pipe, "lm_raster", -1, fbprops, winprops,
    flags, _gsg, _host_output);
  nassertr(buffer != nullptr, false);
  buffer->set_clear_color(LColor(0, 0, 0, 0));

  // Bind framebuffer attachments.
  buffer->add_render_texture(_lm_textures["albedo"], GraphicsOutput::RTM_bind_or_copy,
                             GraphicsOutput::RTP_color);
  buffer->add_render_texture(_lm_textures["position"], GraphicsOutput::RTM_bind_or_copy,
                             GraphicsOutput::RTP_aux_float_0);
  buffer->add_render_texture(_lm_textures["normal"], GraphicsOutput::RTM_bind_or_copy,
                             GraphicsOutput::RTP_aux_float_1);
  buffer->add_render_texture(_lm_textures["unocclude"], GraphicsOutput::RTM_bind_or_copy,
                             GraphicsOutput::RTP_aux_float_2);
  buffer->add_render_texture(_lm_textures["emission"], GraphicsOutput::RTM_bind_or_copy,
                             GraphicsOutput::RTP_aux_float_3);

  pvector<NodePath> scenes;
  pvector<PT(DisplayRegion)> display_regions;

  CPT(RenderState) wireframe_state = RenderState::make(
    RenderModeAttrib::make(RenderModeAttrib::M_wireframe), 10
  );

  LVecBase2 uv_offsets[25] = {
    LVecBase2(-2, -2),
    LVecBase2(2, -2),
    LVecBase2(-2, 2),
    LVecBase2(2, 2),

    LVecBase2(-1, -2),
    LVecBase2(1, -2),
    LVecBase2(-2, -1),
    LVecBase2(2, -1),
    LVecBase2(-2, 1),
    LVecBase2(2, 1),
    LVecBase2(-1, 2),
    LVecBase2(1, 2),

    LVecBase2(-2, 0),
    LVecBase2(2, 0),
    LVecBase2(0, -2),
    LVecBase2(0, 2),

    LVecBase2(-1, -1),
    LVecBase2(1, -1),
    LVecBase2(-1, 0),
    LVecBase2(1, 0),
    LVecBase2(-1, 1),
    LVecBase2(1, 1),
    LVecBase2(0, -1),
    LVecBase2(0, 1),

    LVecBase2(0, 0)
  };

  PN_stdfloat x_half_size = (1.0f / (PN_stdfloat)_lightmap_size[0]);
  PN_stdfloat y_half_size = (1.0f / (PN_stdfloat)_lightmap_size[1]);

  //CPT(RenderState) uv_offset_states[25];
  //for (size_t i = 0; i < 25; i++) {
  //  CPT(RenderAttrib) sattr = ShaderAttrib::make();
  //  uv_offset_states[i] = RenderState::make(
  //    DCAST(ShaderAttrib, sattr)->set_shader_input(
  //      ShaderInput("u_uv_offset", LVecBase2(uv_offsets[i][0] * x_half_size,
  //                                           uv_offsets[i][1] * y_half_size))),
  //    10
  ///  );
  //}

  // Now create a display region for each lightmap palette that will render
  // the Geoms that are part of that palette.  Each display region will
  // render into the correct page of the array texture.
  for (size_t i = 0; i < _pages.size(); i++) {
    const LightmapPage &page = _pages[i];

    assert(page.index == i);

    NodePath root("root");

    // Don't do any front or back face culling.
    root.set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none), 10);
    // Don't allow Panda to do dualing transparency if a Geom enables it.
    root.set_transparency(TransparencyAttrib::M_none, 10);
    //root.set_antialias(AntialiasAttrib::M_multisample, 10);
    // If we write to or test against the depth buffer, Geoms rendered in one
    // page may be occluded by Geoms that were rendered in a different page.
    root.set_depth_write(false, 10);
    root.set_depth_test(false, 10);
    root.set_bin("unsorted", i, 10);
    root.set_attrib(ColorBlendAttrib::make_off(), 10);

    //root.set_render_mode_filled_wireframe()

    root.set_shader(Shader::load(Shader::SL_GLSL, "shaders/lm_raster.vert.glsl", "shaders/lm_raster.frag.glsl"), 10);
    apply_kd_uniforms(root, 10);
    root.set_shader_input("u_lm_palette_luxel_size", LVecBase2(1.0f / (float)_lightmap_size[0],
                                                               1.0f / (float)_lightmap_size[1]));

    scenes.push_back(root);

    PT(DisplayRegion) dr = buffer->make_display_region();
    dr->set_sort(i);
    // Instruct the display region to render into this lightmap page.
    dr->set_target_tex_page(i);
    dr->set_clear_color_active(true);
    dr->set_clear_depth_active(true);
    dr->set_clear_color(LColor(0, 0, 0, 0));
    dr->set_clear_active(GraphicsOutput::RTP_aux_float_0, true);
    dr->set_clear_active(GraphicsOutput::RTP_aux_float_1, true);
    dr->set_clear_active(GraphicsOutput::RTP_aux_float_2, true);
    dr->set_clear_active(GraphicsOutput::RTP_aux_float_3, true);
    dr->set_clear_value(GraphicsOutput::RTP_aux_float_0, LColor(0, 0, 0, 0));
    dr->set_clear_value(GraphicsOutput::RTP_aux_float_1, LColor(0, 0, 0, 0));
    dr->set_clear_value(GraphicsOutput::RTP_aux_float_2, LColor(0, 0, 0, 0));
    dr->set_clear_value(GraphicsOutput::RTP_aux_float_3, LColor(0, 0, 0, 1));

    PT(Camera) cam = new Camera("cam");
    PT(OrthographicLens) lens = new OrthographicLens;
    lens->set_film_size(-1000, 1000);
    cam->set_lens(lens);
    // Give the camera an omni cull volume so nothing gets view frustum culled.
    // We have to render everything.
    cam->set_cull_bounds(new OmniBoundingVolume);
    NodePath cam_np(cam);
    dr->set_camera(cam_np);
    cam_np.reparent_to(root);

    display_regions.push_back(dr);

    // Now add each Geom associated with this palette/page to a GeomNode
    // and parent it into the rasterizer scene.
    for (size_t j = 0; j < page.geoms.size(); j++) {
      const LightmapGeom &geom = _geoms[page.geoms[j]];

      assert(geom.palette == i);

      // Determine the base color/albedo texture of the Geom.
      PT(Texture) base_tex;
      bool has_base_color = false;
      LColor base_color(0.5, 0.5f, 0.5f, 1.0f);
      LVecBase3 emission_color(0, 0, 0);

      bool has_transparency = (geom.contents & C_transparent) != 0;
      bool has_emission = false;

      const MaterialAttrib *mattr;
      if (geom.state->get_attrib(mattr)) {
        // If we have a material attrib try to get the base color texture.
        Material *mat = mattr->get_material();
        if (mat != nullptr) {

          MaterialParamBase *base_color_param = mat->get_param("base_color");
          if (base_color_param != nullptr) {
            if (base_color_param->is_of_type(MaterialParamColor::get_class_type())) {
              // It's a solid color and not a texture.
              has_base_color = true;
              base_color = ((MaterialParamColor *)base_color_param)->get_value();
            } else if (base_color_param->is_of_type(MaterialParamTexture::get_class_type())) {
              // The base color comes from a texture.
              base_tex = ((MaterialParamTexture *)base_color_param)->get_value();
            }
          }

          // Check for emission.
          MaterialParamBase *selfillum_param = mat->get_param("selfillum");
          if (selfillum_param != nullptr && DCAST(MaterialParamBool, selfillum_param)->get_value()) {
            has_emission = true;
            MaterialParamBase *tint_param = mat->get_param("selfillumtint");
            if (tint_param != nullptr) {
              emission_color = DCAST(MaterialParamColor, tint_param)->get_value().get_xyz();
              emission_color[0] = std::pow(emission_color[0], 2.2f);
              emission_color[1] = std::pow(emission_color[1], 2.2f);
              emission_color[2] = std::pow(emission_color[2], 2.2f);
            }
          }
        }
      }

      if (base_tex == nullptr) {
        // Didn't get a base color texture yet.  Check for one on the
        // TextureAttrib now.
        const TextureAttrib *tattr;
        if (geom.state->get_attrib(tattr)) {
          base_tex = tattr->get_texture();
        }
      }

      if (base_tex == nullptr) {
        // If we didn't get a base texture, create a dummy 1x1 texture that is
        // cleared to the base color (or white if no base color was specified).
        base_tex = new Texture("dummy_base_color");
        base_tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_srgb);
        base_tex->set_clear_color(base_color);
        base_tex->clear_image();
      }

      geom.ni_geom->make_nonindexed(false);

      PT(GeomNode) geom_node = new GeomNode("lm_geom");
      geom_node->add_geom(geom.ni_geom, geom.state);
      //for (size_t k = 0; k < 25; k++) {
      //  geom_node->add_geom(geom.ni_geom, geom.state->compose(uv_offset_states[k]));
      //}

      geom_node->set_transform(geom.net_transform);
      NodePath geom_np = root.attach_new_node(geom_node);
      geom_np.set_shader_input("base_texture_sampler", base_tex, 10);
      geom_np.set_shader_input("emission_color", emission_color, 10);
      geom_np.set_shader_input("first_triangle_transparency_emission", LVecBase3i(geom.first_triangle, has_transparency, has_emission), 10);
      geom_np.set_shader_input("geom_uv_mins", geom.uv_mins, 10);
      geom_np.set_shader_input("geom_uv_maxs", geom.uv_maxs, 10);
    }
  }

  _gsg->set_conservative_raster(true);

  // Now render everything and block until it's done.
  _graphics_engine->render_frame();
  _graphics_engine->sync_frame();

  _gsg->set_conservative_raster(false);

  _gsg->finish();

  //_graphics_engine->extract_texture_data(_lm_textures["albedo"], _gsg);
  //_lm_textures["albedo"]->write("lm_albedo_#.png", 0, 0, true, false);
  //_graphics_engine->extract_texture_data(_lm_textures["emission"], _gsg);
  //_lm_textures["emission"]->write("lm_emission_#.png", 0, 0, true, false);
  //_graphics_engine->extract_texture_data(_lm_textures["position"], _gsg);
  //_lm_textures["position"]->write("lm_position_#.pfm", 0, 0, true, false);
  //_graphics_engine->extract_texture_data(_lm_textures["normal"], _gsg);
  //_lm_textures["normal"]->write("lm_normal_#.pfm", 0, 0, true, false);

  _graphics_engine->remove_window(buffer);

  return true;
}

/**
 * Rasterizes per-vertex albedo of all vertex lit geometry.
 */
bool LightBuilder::
rasterize_vertex_lit_geoms() {
  if (_num_vertex_lit_vertices == 0) {
    return true;
  }

  lightbuilder_cat.info()
    << "Rasterizing vertex lit per-vertex albedo\n";

  WindowProperties winprops;
  winprops.clear();
  winprops.set_size(_vertex_palette_width, _vertex_palette_height);

  FrameBufferProperties fbprops;
  fbprops.clear();
  fbprops.set_rgba_bits(8, 8, 8, 8);
  fbprops.set_force_hardware(true);
  fbprops.set_srgb_color(false);

  unsigned int flags = GraphicsPipe::BF_refuse_window;

  PT(GraphicsOutput) buffer = _graphics_engine->make_output(
    _graphics_pipe, "vtx_raster", -1, fbprops, winprops,
    flags, _gsg, _host_output);
  nassertr(buffer != nullptr, false);
  buffer->set_clear_color(LColor(0, 0, 0, 0));

  // Bind framebuffer attachments.
  buffer->add_render_texture(_lm_textures["vtx_albedo"], GraphicsOutput::RTM_bind_or_copy,
                             GraphicsOutput::RTP_color);

  NodePath root("root");

  // Don't do any front or back face culling.
  root.set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none), 10);
  // Don't allow Panda to do dualing transparency if a Geom enables it.
  root.set_transparency(TransparencyAttrib::M_none, 10);
  //root.set_antialias(AntialiasAttrib::M_multisample, 10);
  // If we write to or test against the depth buffer, Geoms rendered in one
  // page may be occluded by Geoms that were rendered in a different page.
  root.set_depth_write(false, 10);
  root.set_depth_test(false, 10);
  root.set_bin("unsorted", 10);
  root.set_attrib(ColorBlendAttrib::make_off(), 10);

  root.set_shader(Shader::load(Shader::SL_GLSL, "shaders/lm_vtx_raster.vert.glsl", "shaders/lm_vtx_raster.frag.glsl"), 10);
  root.set_shader_input("u_vtx_palette_size", LVecBase2i(_vertex_palette_width, _vertex_palette_height), 10);

  PT(DisplayRegion) dr = buffer->make_display_region();
  dr->set_clear_color_active(true);
  dr->set_clear_depth_active(true);
  dr->set_clear_color(LColor(0, 0, 0, 0));

  PT(Camera) cam = new Camera("cam");
  PT(OrthographicLens) lens = new OrthographicLens;
  lens->set_film_size(-1000, 1000);
  cam->set_lens(lens);
  // Give the camera an omni cull volume so nothing gets view frustum culled.
  // We have to render everything.
  cam->set_cull_bounds(new OmniBoundingVolume);
  NodePath cam_np(cam);
  dr->set_camera(cam_np);
  cam_np.reparent_to(root);

  // Render all vertex lit geoms as points at each vertex.
  for (const LightmapGeom &geom : _geoms) {
    if (geom.light_mode != LightmapGeom::LM_per_vertex) {
      continue;
    }

    PT(Geom) pts_geom = geom.ni_geom->make_points();
    pts_geom->make_nonindexed(false);

    PT(GeomNode) gn = new GeomNode("points");
    gn->add_geom(pts_geom, geom.state);
    gn->set_transform(geom.net_transform);

    LColor base_color(1, 1, 1, 1);

    PT(Texture) base_tex;
    if (geom.state->has_attrib(MaterialAttrib::get_class_slot())) {
      const MaterialAttrib *mattr;
      geom.state->get_attrib(mattr);
      Material *mat = mattr->get_material();
      if (mat != nullptr) {
        MaterialParamBase *tex_p = mat->get_param("base_color");
        if (tex_p != nullptr && tex_p->is_of_type(MaterialParamTexture::get_class_type())) {
          base_tex = DCAST(MaterialParamTexture, tex_p)->get_value();
        }
      }
    }
    if (base_tex == nullptr) {
      if (geom.state->has_attrib(TextureAttrib::get_class_slot())) {
        const TextureAttrib *texa;
        geom.state->get_attrib(texa);
        base_tex = texa->get_texture();
      }
    }

    if (base_tex == nullptr) {
      // If we didn't get a base texture, create a dummy 1x1 texture that is
      // cleared to the base color (or white if no base color was specified).
      base_tex = new Texture("dummy_base_color");
      base_tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_srgb);
      base_tex->set_clear_color(base_color);
      base_tex->clear_image();
    }

    NodePath gnnp = root.attach_new_node(gn);
    gnnp.set_shader_input("u_first_vertex", LVecBase2i(geom.first_vertex - _first_vertex_lit_vertex), 10);
    gnnp.set_shader_input("base_texture_sampler", base_tex, 10);
  }

  //_gsg->set_conservative_raster(true);

  _graphics_engine->render_frame();
  _graphics_engine->sync_frame();

  //_gsg->set_conservative_raster(false);

  _gsg->finish();

  _graphics_engine->remove_window(buffer);

  return true;
}

/**
 *
 */
bool LightBuilder::
compute_unocclude() {
  lightbuilder_cat.info()
    << "Computing unocclude...\n";

  NodePath np("state");

  np.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_unocclude.compute.glsl"));

  apply_kd_uniforms(np, 0);

  np.set_shader_input("position", _lm_textures["position"]);
  np.set_shader_input("unocclude", _lm_textures["unocclude"]);

  np.set_shader_input("u_bias_", LVecBase2(_bias));
  np.set_shader_input("u_region_ofs", LVecBase2i(0, 0));

  LVecBase3i group_size((_lightmap_size[0] - 1) / 8 + 1,
                        (_lightmap_size[1] - 1) / 8 + 1, 1);

  for (size_t i = 0; i < _pages.size(); i++) {
    np.set_shader_input("u_palette_size_page", LVecBase3i(_lightmap_size[0], _lightmap_size[1], i));

    // Run all pages simultaneously.
    _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
    _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
  }

  _gsg->finish();

  // Free up memory.
  free_texture(_lm_textures["unocclude"]);
  // We have to render a frame to actually free the textures.
  _graphics_engine->render_frame();

  lightbuilder_cat.info()
    << "Done.\n";
  return true;
}

/**
 * Computes the direct lighting for each luxel.
 */
bool LightBuilder::
compute_direct() {
  lightbuilder_cat.info()
    << "Computing direct light...\n";

  NodePath np("state");

  np.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_direct.compute.glsl"));

  apply_kd_uniforms(np, 0);

  np.set_shader_input("luxel_direct", _lm_textures["direct"]);
  np.set_shader_input("luxel_direct_dynamic", _lm_textures["direct_dynamic"]);
  np.set_shader_input("luxel_reflectivity", _lm_textures["reflectivity"]);
  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);
  np.set_shader_input("luxel_position", _lm_textures["position"]);
  np.set_shader_input("luxel_normal", _lm_textures["normal"]);
  np.set_shader_input("luxel_emission", _lm_textures["emission"]);

  np.set_shader_input("u_bias_sun_extent", LVecBase2(_bias, std::sin(deg_2_rad(_sun_angular_extent))));
  np.set_shader_input("u_region_ofs", LVecBase2i(0, 0));
  np.set_shader_input("u_to_cell_offset", _scene_mins);

  LVecBase3i group_size((_lightmap_size[0] - 1) / 8 + 1,
                        (_lightmap_size[1] - 1) / 8 + 1, 1);

  for (size_t i = 0; i < _pages.size(); i++) {
    np.set_shader_input("u_palette_size_page", LVecBase3i(_lightmap_size[0], _lightmap_size[1], i));

    // Run all pages simultaneously.
    _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
    _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
  }

  _gsg->finish();

  lightbuilder_cat.info()
    << "Done.\n";

  //_graphics_engine->extract_texture_data(_lm_textures["direct"], _gsg);
  //_lm_textures["direct"]->write("lm_direct_#.png", 0, 0, true, false);

  return true;
}

/**
 *
 */
bool LightBuilder::
compute_vtx_reflectivity() {
  if (_num_vertex_lit_vertices == 0) {
    return true;
  }

  lightbuilder_cat.info()
    << "Computing per-vertex reflectivity...\n";

  NodePath np("state");

  np.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_vtx_direct.compute.glsl"));

  apply_kd_uniforms(np, 0);

  np.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl"]);
  np.set_shader_input("vtx_albedo", _lm_textures["vtx_albedo"]);
  np.set_shader_input("vtx_light", _lm_textures["vtx_light"]);
  np.set_shader_input("vtx_light_dynamic", _lm_textures["vtx_light_dynamic"]);
  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);

  np.set_shader_input("u_vtx_palette_size_first_vtx_num_verts",
    LVecBase4i(_vertex_palette_width, _vertex_palette_height,
              _first_vertex_lit_vertex, _num_vertex_lit_vertices));
  np.set_shader_input("_u_bias", LVecBase2f(_bias));

  LVecBase3i group_size((_vertex_palette_width - 1) / 64 + 1,
                        _vertex_palette_height, 1);

  _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
  _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);

  _gsg->finish();

  //free_texture(_lm_textures["vtx_albedo"]);
  //_graphics_engine->render_frame();

  lightbuilder_cat.info()
    << "Done.\n";

  return true;
}

/**
 * Computes the indirect lighting for each luxel.
 */
bool LightBuilder::
compute_indirect() {
  if (_bounces == 0) {
    return true;
  }

  lightbuilder_cat.info()
    << "Computing indirect light...\n";

  NodePath np("state");

  np.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_indirect.compute.glsl"));

  apply_kd_uniforms(np, 0);

  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);
  np.set_shader_input("vtx_albedo", _lm_textures["vtx_albedo"]);
  np.set_shader_input("luxel_position", _lm_textures["position"]);
  np.set_shader_input("luxel_normal", _lm_textures["normal"]);
  np.set_shader_input("luxel_light", _lm_textures["direct"]);

  np.set_shader_input("u_vtx_lit_info", LVecBase2i(_first_vertex_lit_vertex, _vertex_palette_width));

  np.set_shader_input("u_bias_", LVecBase2(_bias));
  np.set_shader_input("u_region_ofs", LVecBase2i(0, 0));
  np.set_shader_input("u_sky_color", _sky_color.get_xyz());

  NodePath vnp("vstate");
  vnp.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_vtx_indirect.compute.glsl"));
  apply_kd_uniforms(vnp, 0);

  vnp.set_shader_input("luxel_albedo", _lm_textures["albedo"]);
  vnp.set_shader_input("vtx_albedo", _lm_textures["vtx_albedo"]);
  vnp.set_shader_input("vtx_light", _lm_textures["vtx_light"]);

  vnp.set_shader_input("u_sky_color", _sky_color.get_xyz());

  vnp.set_shader_input("u_vtx_palette_size_first_vtx_num_verts",
    LVecBase4i(_vertex_palette_width, _vertex_palette_height,
              _first_vertex_lit_vertex, _num_vertex_lit_vertices));
  vnp.set_shader_input("_u_bias", LVecBase2f(_bias));

  NodePath anp("astate");
  anp.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_probes.compute.glsl"));
  apply_kd_uniforms(anp, 0);
  // Probe positions.
  anp.set_shader_input("probes", _gpu_buffers["probes"]);
  // Probe output data.
  anp.set_shader_input("probe_output", _lm_textures["probes"]);
  anp.set_shader_input("u_bias_", LVecBase2(_bias));
  anp.set_shader_input("_u_probe_count", LVecBase2i((int)_probes.size()));
  anp.set_shader_input("u_sky_color", _sky_color.get_xyz());
  anp.set_shader_input("luxel_albedo", _lm_textures["albedo"]);
  anp.set_shader_input("u_vtx_lit_info", LVecBase2i(_first_vertex_lit_vertex, _vertex_palette_width));

  //int num_ray_iters = 16;
  int rays_per_iter = 16;//_rays_per_luxel / num_ray_iters;
  int num_ray_iters = (_rays_per_luxel - 1) / rays_per_iter + 1;

  PT(Texture) total_added = new Texture("lm-indirect-total-bounce-added");
  total_added->setup_1d_texture(3, Texture::T_unsigned_int, Texture::F_r32i);
  total_added->set_compression(Texture::CM_off);
  total_added->set_clear_color(LColor(0));
  total_added->clear_image();

  np.set_shader_input("feedback_total_add", total_added);
  vnp.set_shader_input("feedback_total_add", total_added);

  for (int b = 0; b < 100; b++) {
    lightbuilder_cat.info()
      << "Bounce " << b + 1 << "...\n";

    total_added->clear_image();

    // It works like this:
    // Bounce 0 gathers direct light * albedo.
    // Bounce 1 gathers bounce 0
    // Bounce 2 gathers bounce 1
    // ...and so on.

    Texture *out_tex;
    Texture *out_vtx_tex;

    if (b & 1) {
      // Reflected light read from here.
      np.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl_accum"]);
      vnp.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl_accum"]);
      anp.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl_accum"]);
      np.set_shader_input("luxel_reflectivity", _lm_textures["indirect_accum"]);
      vnp.set_shader_input("luxel_reflectivity", _lm_textures["indirect_accum"]);
      anp.set_shader_input("luxel_reflectivity", _lm_textures["indirect_accum"]);

      // Gathered light stored here.
      np.set_shader_input("luxel_gathered", _lm_textures["reflectivity"]);
      vnp.set_shader_input("vtx_gathered", _lm_textures["vtx_refl"]);

      // Gathered light on this bounce is stored in this texture, accumulated
      // over several ray iterations.  We need to be sure to clear out data
      // from the previous bounce.
      //_lm_textures["reflectivity"]->clear_image();
      //_lm_textures["vtx_refl"]->clear_image();

      out_tex = _lm_textures["reflectivity"];
      out_vtx_tex = _lm_textures["vtx_refl"];

    } else {
      // Reflected light read from here.
      np.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl"]);
      vnp.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl"]);
      anp.set_shader_input("vtx_reflectivity", _lm_textures["vtx_refl"]);
      np.set_shader_input("luxel_reflectivity", _lm_textures["reflectivity"]);
      vnp.set_shader_input("luxel_reflectivity", _lm_textures["reflectivity"]);
      anp.set_shader_input("luxel_reflectivity", _lm_textures["reflectivity"]);

      // Gathered light stored here.
      np.set_shader_input("luxel_gathered", _lm_textures["indirect_accum"]);
      vnp.set_shader_input("vtx_gathered", _lm_textures["vtx_refl_accum"]);

      // Gathered light on this bounce is stored in this texture, accumulated
      // over several ray iterations.  We need to be sure to clear out data
      // from the previous bounce.
      //_lm_textures["indirect_accum"]->clear_image();
      //_lm_textures["vtx_refl_accum"]->clear_image();

      out_tex = _lm_textures["indirect_accum"];
      out_vtx_tex = _lm_textures["vtx_refl_accum"];
    }

#if 1
    int ray_start = 0;

    double start = ClockObject::get_global_clock()->get_real_time();

    for (int i = 0; i < num_ray_iters; ++i) {
      lightbuilder_cat.info(false)
        << (int)(((float)(i + 1) / num_ray_iters) * 100.0f) << "%";
      if (i != (num_ray_iters - 1)) {
        lightbuilder_cat.info(false) << "\r";
      } else {
        lightbuilder_cat.info(false) << "\n";
      }

      // Lightmapped stuff.
      for (size_t j = 0; j < _pages.size(); ++j) {
        np.set_shader_input("u_palette_size_page_bounce", LVecBase4i(_lightmap_size[0], _lightmap_size[1], j, b));
        np.set_shader_input("u_ray_params", LVecBase3i(ray_start, std::min(ray_start + rays_per_iter, _rays_per_luxel), _rays_per_luxel));
        np.set_shader_input("u_region_ofs", LVecBase2i(0, 0));
        LVecBase3i group_size((_lightmap_size[0] - 1) / 8 + 1, (_lightmap_size[1] - 1) / 8 + 1, 1);
        _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
        _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
        _gsg->finish();
      }

      // Vertex lit stuff.
      if (_num_vertex_lit_vertices > 0) {
        LVecBase3i group_size((_vertex_palette_width - 1) / 64 + 1, _vertex_palette_height, 1);
        //LVecBase3i group_size((_vertex_palette_width - 1) / 8 + 1, (_vertex_palette_height - 1) / 8 + 1, 1);
        vnp.set_shader_input("u_ray_count_bounce", LVecBase4i(ray_start, std::min(ray_start + rays_per_iter, _rays_per_luxel), _rays_per_luxel, b));
        vnp.set_shader_input("u_region_ofs", LVecBase2i(0, 0));
        _gsg->set_state_and_transform(vnp.get_state(), TransformState::make_identity());
        _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
        _gsg->finish();
      }

      // Ambient probes.
      if (!_probes.empty()) {
        // Gather ambient probe lighting.
        LVecBase3i group_size((_probes.size() - 1) / 64 + 1, 1, 1);
        anp.set_shader_input("u_ray_params", LVecBase4i(ray_start, std::min(ray_start + rays_per_iter, _rays_per_luxel), _rays_per_luxel, b));
        _gsg->set_state_and_transform(anp.get_state(), TransformState::make_identity());
        _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
        _gsg->finish();
      }

      ray_start += rays_per_iter;
    }

    double end = ClockObject::get_global_clock()->get_real_time();

    lightbuilder_cat.info(false)
      << " [ " << (int)(end - start) << " seconds ]";
#endif

    //_gsg->finish();

#if 0
    _graphics_engine->extract_texture_data(out_tex, _gsg);
    std::ostringstream ss;
    ss << "gathered-bounce-" << b << "-lm.png";
    out_tex->write(ss.str());

    //_graphics_engine->extract_texture_data(out_vtx_tex, _gsg);
    //std::ostringstream ss2;
    //ss2 << "gathered-bounce-" << b << "-vtx.png";
    //out_vtx_tex->write(ss2.str());
#endif

    _graphics_engine->extract_texture_data(total_added, _gsg);
    CPTA_uchar added_ram_image = total_added->get_ram_image();
    const uint32_t *added_datap = (const uint32_t *)added_ram_image.p();
    float max_r = (float)added_datap[0] / 10000.0f;
    float max_g = (float)added_datap[1] / 10000.0f;
    float max_b = (float)added_datap[2] / 10000.0f;
    lightbuilder_cat.info(false)
      << " [ Added max RGB " << max_r << " " << max_g << " " << max_b << " ]\n";
    if (max_r <= 0.0001f && max_g <= 0.0001f && max_b <= 0.0001f) {
      // Stabilized.  We're done bouncing.
      break;
    }
  }

  if (!_probes.empty()) {
    // Retrieve probe data back onto CPU.
    _graphics_engine->extract_texture_data(_lm_textures["probes"], _gsg);
    CPTA_uchar probe_data = _lm_textures["probes"]->get_ram_image();
    const float *probe_datap = (const float *)probe_data.p();

    // Now output the data to a friendly format.
    for (size_t i = 0; i < _probes.size(); i++) {
      if (lightbuilder_cat.is_debug()) {
        lightbuilder_cat.debug()
          << "Probe " << i << ":\n";
      }
      LightmapAmbientProbe &probe = _probes[i];
      for (int j = 0; j < 9; j++) {
        probe.data[j][0] = probe_datap[i * 36 + j * 4];
        probe.data[j][1] = probe_datap[i * 36 + j * 4 + 1];
        probe.data[j][2] = probe_datap[i * 36 + j * 4 + 2];
        if (lightbuilder_cat.is_debug()) {
          lightbuilder_cat.debug(false)
            << "\t" << probe.data[j] << "\n";
        }
      }
    }
  }

  // Free up memory.
  free_texture(_lm_textures["indirect_accum"]);
  free_texture(_lm_textures["indirect"]);
  free_texture(_lm_textures["reflectivity"]);
  free_texture(_lm_textures["vtx_refl_accum"]);
  free_texture(_lm_textures["vtx_refl"]);
  free_texture(_lm_textures["position"]);
  free_texture(_lm_textures["normal"]);
  free_texture(_lm_textures["probes"]);
  free_texture(_lm_textures["albedo"]);
  free_texture(_lm_textures["vtx_albedo"]);
  // Freeing a texture is actually queued up and not actually
  // freed until we flush the queue by rendering a frame.
  _graphics_engine->render_frame();

  lightbuilder_cat.info()
    << "Done.\n";

  return true;
}

#define PrintVec(v) v[0] << " " << v[1] << " " << v[2]
#define VectorClamp(v, a, b) \
  { \
    v[0] = std::clamp(v[0], a, b); \
    v[1] = std::clamp(v[1], a, b); \
    v[2] = std::clamp(v[2], a, b); \
  }

/**
 * Uses Intel OpenImageDenoise to denoise the computed lightmaps.
 */
bool LightBuilder::
denoise_lightmaps() {
  lightbuilder_cat.info()
    << "Denoising lightmaps...\n";

  // Bring the needed textures into system RAM.
  _graphics_engine->extract_texture_data(_lm_textures["reflectivity"], _gsg);

#if 1

  oidn::DeviceRef device = oidn::newDevice();
  device.commit();

  // Get the ram image without an alpha channel.  OIDN wants a 3-channel image.
  CPTA_uchar color_data = _lm_textures["reflectivity"]->get_ram_image_as("RGB");
  CPTA_uchar alpha_data = _lm_textures["reflectivity"]->get_ram_image_as("A");
  float *color_datap = (float *)color_data.p();
  const float *alpha_datap = (const float *)alpha_data.p();

  size_t page_size = 3 * _lightmap_size[0] * _lightmap_size[1];

  // Normalize the L1 coefficients.
  for (int page = 0; page < (int)_pages.size(); ++page) {
    int pos = 0;
    for (int y = 0; y < _lightmap_size[1]; y++) {
      for (int x = 0; x < _lightmap_size[0]; x++) {
        float *l0 = &color_datap[page_size * page * 4 + pos];
        float *l1n1 = &color_datap[page_size * (page * 4 + 1) + pos];
        float *l1n0 = &color_datap[page_size * (page * 4 + 2) + pos];
        float *l1p1 = &color_datap[page_size * (page * 4 + 3) + pos];

        float l0_factor[3] = {
          1.0f / ((l0[0] / 0.282095f) * 0.488603f),
          1.0f / ((l0[1] / 0.282095f) * 0.488603f),
          1.0f / ((l0[2] / 0.282095f) * 0.488603f),
        };

        VectorMultiply(l1n1, l0_factor, l1n1);
        VectorMultiply(l1n0, l0_factor, l1n0);
        VectorMultiply(l1p1, l0_factor, l1p1);
        VectorClamp(l1n1, -1.0f, 1.0f);
        VectorClamp(l1n0, -1.0f, 1.0f);
        VectorClamp(l1p1, -1.0f, 1.0f);

        pos += 3;
      }
    }
  }

  page_size = sizeof(float) * 3 * _lightmap_size[0] * _lightmap_size[1];

  oidn::FilterRef filter = device.newFilter("RTLightmap");
  // Denoise each page.
  for (size_t i = 0; i < _pages.size(); i++) {

    for (size_t j = 0; j < 4; ++j) {
      void *page_ptr = (void *)(color_data.p() + page_size * (i * 4 + j));
      if (j == 0) {
        // The first page is the L0 constant term (0..inf).
        filter.set("directional", false);
      } else {
        // The remaining 3 are normalized L1 coefficients, we normalized them
        // to range from -1..1 above, which is what OIDN expects.
        filter.set("directional", true);
      }

      filter.setImage("color", page_ptr,
                    oidn::Format::Float3, _lightmap_size[0], _lightmap_size[1]);
      filter.setImage("output", page_ptr,
                      oidn::Format::Float3, _lightmap_size[0], _lightmap_size[1]);
      filter.commit();

      filter.execute();

      const char *error;
      if (device.getError(error) != oidn::Error::None) {
        lightbuilder_cat.error()
          << "OIDN error when denoising lightmap page " << i << ": "
          << std::string(error) << "\n";
        return false;
      }
    }
  }

  PTA_uchar new_data;
  new_data.resize(_lm_textures["reflectivity"]->get_expected_ram_image_size());
  float *new_datap = (float *)new_data.p();

  for (int i = 0; i < _lightmap_size[0] * _lightmap_size[1] * _pages.size() * 4; i++) {
    new_datap[i * 4] = color_datap[i * 3];
    new_datap[i * 4 + 1] = color_datap[i * 3 + 1];
    new_datap[i * 4 + 2] = color_datap[i * 3 + 2];
    new_datap[i * 4 + 3] = alpha_datap[i];
  }

  page_size = 4 * _lightmap_size[0] * _lightmap_size[1];

#if 1
  // Now, after denoising, scale and bias the L1 coefficients to
  // range from 0..1, so we can store the textures as RGB8 and compress
  // as DXT1.
  for (int page = 0; page < (int)_pages.size(); ++page) {
    int pos = 0;
    for (int y = 0; y < _lightmap_size[1]; y++) {
      for (int x = 0; x < _lightmap_size[0]; x++) {
        float *l1n1 = &new_datap[page_size * (page * 4 + 1) + pos];
        float *l1n0 = &new_datap[page_size * (page * 4 + 2) + pos];
        float *l1p1 = &new_datap[page_size * (page * 4 + 3) + pos];

        VectorScale(l1n1, 0.5f, l1n1);
        VectorAddVec(l1n1, 0.5f, l1n1);
        VectorScale(l1n0, 0.5f, l1n0);
        VectorAddVec(l1n0, 0.5f, l1n0);
        VectorScale(l1p1, 0.5f, l1p1);
        VectorAddVec(l1p1, 0.5f, l1p1);

        VectorClamp(l1n1, 0.0f, 1.0f);
        VectorClamp(l1n0, 0.0f, 1.0f);
        VectorClamp(l1p1, 0.0f, 1.0f);

        pos += 4;
      }
    }
  }
#endif

  // Now throw it back on the texture.
  _lm_textures["reflectivity"]->set_ram_image_as(new_data, "RGBA");
  _lm_textures["reflectivity"]->set_format(Texture::F_rgba32);

#endif

  lightbuilder_cat.info()
    << "Done.\n";

  return true;
}

/**
 * Expands lightmap borders to fix bleeding in the palette.
 */
bool LightBuilder::
dialate_lightmaps() {
  lightbuilder_cat.info()
    << "Dilating lightmaps...\n";

  NodePath np("state");

  np.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_dilate.compute.glsl"));

  np.set_shader_input("source_tex", _lm_textures["direct"]);
  // NOTE: reflectivity texture is re-used for the output of dilation.
  np.set_shader_input("dest_image", _lm_textures["reflectivity"]);

  np.set_shader_input("u_region_ofs", LVecBase2i(0, 0));

  LVecBase3i group_size((_lightmap_size[0] - 1) / 8 + 1,
                        (_lightmap_size[1] - 1) / 8 + 1, 1);

  for (size_t i = 0; i < _pages.size() * 4; i++) {
    np.set_shader_input("u_palette_size_page", LVecBase3i(_lightmap_size[0], _lightmap_size[1], i));

    // Run all pages simultaneously.
    _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
    _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
  }

  _gsg->finish();

  // Free up memory.
  free_texture(_lm_textures["direct"]);
  _graphics_engine->render_frame();

  lightbuilder_cat.info()
    << "Done.\n";

  return true;
}

/**
 * Applies the output lightmap to all Geoms in the scene.
 */
bool LightBuilder::
write_geoms() {
  lightbuilder_cat.info()
    << "Applying lightmaps to input Geoms\n";

  Texture *lm_tex = _lm_textures["reflectivity"];

  // Bring the lightmap texture into system RAM.
  // The reflectivity texture is re-used as the dilation output.
  //if (!_graphics_engine->extract_texture_data(lm_tex, _gsg)) {
  //  lightbuilder_cat.error()
  //    << "Couldn't bring lightmap texture into system RAM\n";
  //  return false;
  //}

  size_t page_size = lm_tex->get_ram_page_size();
  CPTA_uchar lm_ram_image = lm_tex->get_ram_image();

  //_lm_textures["reflectivity"]->write("lm_final_light_#.png", 0, 0, true, false);

  // Lightmaps are identified in the shader generator by a texture stage
  // named "lightmap".
  PT(TextureStage) stage = new TextureStage("lightmap");
  PT(TextureStage) stage_l1y = new TextureStage("lightmap_l1y");
  PT(TextureStage) stage_l1z = new TextureStage("lightmap_l1z");
  PT(TextureStage) stage_l1x = new TextureStage("lightmap_l1x");
  TextureStage *l1_stages[3] = { stage_l1y, stage_l1z, stage_l1x };
  PT(Texture) l1_textures[3];

  pvector<CPT(RenderState)> page_texture_states;
  // Extract each page from the lightmap array texture into individual textures.
  for (size_t i = 0; i < _pages.size(); i++) {
    const unsigned char *page_data = lm_ram_image.p() + page_size * i * 4;

    LightmapPage &page = _pages[i];

    // Chop dimensions to absolute minimum size needed by this page.
    // During the build process, each page is the same size (the max of all
    // pages) because of how the pages are stored in a single texture array.
    // However, since the output pages are separate textures, we can store
    // each page at a different size, saving on memory.
    LVecBase2i page_dim = page.dim;

    // The geoms in this page need their lightmap UV's offset again.
    float u_scale = (float)_lightmap_size[0] / (float)page_dim[0];
    float v_scale = (float)_lightmap_size[1] / (float)page_dim[1];

    std::ostringstream ss;
    ss << "lm_page_" << i << "_l0";
    PT(Texture) tex = new Texture(ss.str());
    tex->setup_2d_texture(page_dim[0], page_dim[1], Texture::T_half_float,
                          Texture::F_rgb16);
    tex->set_minfilter(SamplerState::FT_linear);
    tex->set_magfilter(SamplerState::FT_linear);
    tex->set_wrap_u(SamplerState::WM_clamp);
    tex->set_wrap_v(SamplerState::WM_clamp);
    tex->set_keep_ram_image(false);
    tex->set_compression(Texture::CM_off);

    size_t chopped_page_size = sizeof(unsigned short) * page_dim[0] * page_dim[1] * 3;
    PTA_uchar ram_image;
    ram_image.resize(chopped_page_size);
    unsigned char *ram_image_datap = ram_image.p();
    convert_rgba32_to_rgb16(page_data, page_size, _lightmap_size, page_dim, ram_image_datap);
    page_data += page_size;

    tex->set_ram_image(ram_image);

    compress_rgb16_to_bc6h(tex);

    lightbuilder_cat.info()
      << "Output lightmap page " << i << " L0:\n";
    tex->write(lightbuilder_cat.info(false), 0);

    chopped_page_size = sizeof(unsigned char) * page_dim[0] * page_dim[1] * 3;
    for (int j = 0; j < 3; ++j) {
      std::ostringstream l1ss;
      l1ss << "lm_page_" << i << "_l1_" << j;
      PT(Texture) tmp = new Texture(l1ss.str());
      tmp->setup_2d_texture(page_dim[0], page_dim[1], Texture::T_unsigned_byte, Texture::F_rgb8);
      tmp->set_minfilter(SamplerState::FT_linear);
      tmp->set_magfilter(SamplerState::FT_linear);
      tmp->set_wrap_u(SamplerState::WM_clamp);
      tmp->set_wrap_v(SamplerState::WM_clamp);
      tmp->set_keep_ram_image(false);
      tmp->set_compression(Texture::CM_off);
      PTA_uchar tmpram_image;
      tmpram_image.resize(chopped_page_size);
      ram_image_datap = tmpram_image.p();
      convert_rgba32_to_rgb8(page_data, page_size, _lightmap_size, page_dim, ram_image_datap);
      page_data += page_size;
      tmp->set_ram_image_as(tmpram_image, "RGB");
      if (tmp->compress_ram_image(Texture::CM_dxt1)) {
        tmp->set_compression(Texture::CM_dxt1);
        lightbuilder_cat.info()
          << "Successfully compressed L1 ram image " << j << "\n";
      } else {
        lightbuilder_cat.warning()
          << "Failed to compress ram image for L1 " << j << "\n";
      }
      l1_textures[j] = tmp;

      lightbuilder_cat.info()
        << "Output lightmap page " << i << " L1 " << j << ":\n";
      tmp->write(lightbuilder_cat.info(false), 0);
    }

    CPT(RenderAttrib) tattr = TextureAttrib::make();
    tattr = DCAST(TextureAttrib, tattr)->add_on_stage(stage, tex);
    for (int j = 0; j < 3; ++j) {
      tattr = DCAST(TextureAttrib, tattr)->add_on_stage(l1_stages[j], l1_textures[j]);
    }
    page_texture_states.push_back(RenderState::make(tattr));

    if (page_dim != _lightmap_size) {
      // Page size is smaller than largest page.  Re-offset the lightmap UVs
      // of geoms in this page to correspond to chopped page size.

      pmap<CPT(GeomVertexData), PT(GeomVertexData)> mod_vdatas;

      for (int igeom : page.geoms) {
        const LightmapGeom &lgeom = _geoms[igeom];

        // Grab the vertex indices referenced by all primitives of the Geom.
        // We will only offset these vertex UV's.
        BitArray referenced_vertices;
        for (size_t iprim = 0; iprim < lgeom.geom->get_num_primitives(); ++iprim) {
          const GeomPrimitive *prim = lgeom.geom->get_primitive(iprim);
          GeomPrimitivePipelineReader prim_reader(prim, Thread::get_current_thread());
          prim_reader.get_referenced_vertices(referenced_vertices);
        }

        PT(GeomVertexData) &mod_vdata = mod_vdatas[lgeom.geom->get_vertex_data()];
        if (mod_vdata == nullptr) {
          mod_vdata = lgeom.geom->modify_vertex_data();
        }

        GeomVertexRewriter luv_rewriter(mod_vdata, get_lightmap_uv_name());

        // Offset the referenced vertices.
        int index = referenced_vertices.get_lowest_on_bit();
        while (index >= 0) {
          luv_rewriter.set_row(index);
          const LVecBase2f &luv = luv_rewriter.get_data2f();
          luv_rewriter.set_data2f(luv[0] * u_scale, luv[1] * v_scale);

          referenced_vertices.clear_bit(index);
          index = referenced_vertices.get_lowest_on_bit();
        }
      }
    }
  }

  free_texture(_lm_textures["reflectivity"]);
  _graphics_engine->render_frame();

  for (size_t i = 0; i < _pages.size(); i++) {
    const LightmapPage &page = _pages[i];
    for (size_t j = 0; j < page.geoms.size(); j++) {
      LightmapGeom &geom = _geoms[page.geoms[j]];
      // Modify the geom and state.
      if (geom.source_geom_node == nullptr || geom.geom_index == -1) {
        continue;
      }

      // Apply modified Geom back onto the GeomNode.
      geom.source_geom_node->set_geom(geom.geom_index, geom.geom);

      CPT(RenderState) state = geom.source_geom_node->get_geom_state(geom.geom_index);
      // Apply the the modified RenderState containing the lightmap texture.
      state = state->compose(page_texture_states[i]);
      geom.source_geom_node->set_geom_state(geom.geom_index, state);
    }
  }

#if 1
  PT(GeomVertexArrayFormat) arr = new GeomVertexArrayFormat;
  arr->add_column(InternalName::make("vertex_lighting"), 4, GeomEnums::NT_uint8, GeomEnums::C_other);
  CPT(GeomVertexArrayFormat) vtx_light_format = GeomVertexArrayFormat::register_format(arr);

  pmap<CPT(GeomVertexData), PT(GeomVertexArrayData)> light_arrays;

  // Now write baked vertex-lit lighting for static props.
  _graphics_engine->extract_texture_data(_lm_textures["vtx_light"], _gsg);
  const float *vtx_light_data = (const float *)(_lm_textures["vtx_light"]->get_ram_image().p());
  for (size_t i = 0; i < _geoms.size(); ++i) {
    LightmapGeom &lgeom = _geoms[i];
    if (lgeom.light_mode != LightmapGeom::LM_per_vertex) {
      continue;
    }

    PT(GeomVertexArrayData) &light_array = light_arrays[lgeom.geom->get_vertex_data()];
    if (light_array == nullptr) {
      light_array = new GeomVertexArrayData(vtx_light_format, GeomEnums::UH_static);
      light_array->set_num_rows(lgeom.geom->get_vertex_data()->get_num_rows());
    }
    GeomVertexWriter lwriter(light_array);
    lwriter.set_column(InternalName::make("vertex_lighting"));

    for (size_t j = lgeom.first_vertex; j < (lgeom.first_vertex + lgeom.num_vertices); ++j) {
      LightmapVertex &lvert = _vertices[j];

      int palette_offset = j - _first_vertex_lit_vertex;
      float r = vtx_light_data[palette_offset * 4];
      float g = vtx_light_data[palette_offset * 4 + 1];
      float b = vtx_light_data[palette_offset * 4 + 2];

      ColorRGBExp32 rgbe(LVecBase3(b, g, r));

      // Write this to all vertex indices that share this LightmapVertex.
      for (auto it = lvert.orig_vertices.begin(); it != lvert.orig_vertices.end(); ++it) {
        lwriter.set_row(*it);
        lwriter.set_data4i(rgbe.get_r(), rgbe.get_g(), rgbe.get_b(), (int)rgbe.get_exponent() + 128);
      }
    }

    lgeom.vertex_light_array = light_array;
  }

#endif
  return true;
}

/**
 * Computes spherical harmonics ambient lighting probes for applying to
 * dynamic models.
 */
bool LightBuilder::
compute_probes() {
  lightbuilder_cat.info()
    << "Computing ambient probes...\n";

  NodePath np("state");

  np.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/lm_probes.compute.glsl"));

  apply_kd_uniforms(np, 0);

  // Probe positions.
  np.set_shader_input("probes", _gpu_buffers["probes"]);
  // Probe output data.
  np.set_shader_input("probe_output", _lm_textures["probes"]);
  np.set_shader_input("probe_flat_output", _lm_textures["probes_flat"]);

  // Use denoised+dialated indirect+direct lightmap.
  np.set_shader_input("luxel_light", _lm_textures["direct"]);
  // Also include the direct light from non-baked sources.
  np.set_shader_input("luxel_light_dynamic", _lm_textures["direct_dynamic"]);
  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);

  np.set_shader_input("u_bias_", LVecBase2(_bias));
  np.set_shader_input("_u_probe_count", LVecBase2i((int)_probes.size()));
  np.set_shader_input("u_sky_color", _sky_color.get_xyz());

  int ray_count = 2048;
  int max_rays = 64;
  int ray_iters = (ray_count - 1) / max_rays + 1;

  np.set_shader_input("u_ray_params", LVecBase3i(0, ray_count, ray_count));

  //_gsg->finish();

  //std::string blah;

  //for (int i = 0; i < ray_iters; i++) {
  //  int ray_from = i * max_rays;
  //  int ray_to = std::min((i + 1) * max_rays, ray_count);
  //  np.set_shader_input("u_ray_params", LVecBase3i(ray_from, ray_to, ray_count));

    LVecBase3i group_size((_probes.size() - 1) / 64 + 1, 1, 1);
  //for (int i = 0; i < 20; i++) {
    //_gsg->set_state_and_transform(RenderState::make_empty(), TransformState::make_identity());
    //_lm_textures["probes"]->clear_image();
    _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
    _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
    _gsg->finish();
    //std::cin >> blah;
  //}

  // Retrieve probe data back onto CPU.
  _graphics_engine->extract_texture_data(_lm_textures["probes"], _gsg);
  CPTA_uchar probe_data = _lm_textures["probes"]->get_ram_image();
  const float *probe_datap = (const float *)probe_data.p();

  // Now output the data to a friendly format.
  for (size_t i = 0; i < _probes.size(); i++) {
    if (lightbuilder_cat.is_debug()) {
      lightbuilder_cat.debug()
        << "Probe " << i << ":\n";
    }
    LightmapAmbientProbe &probe = _probes[i];
    for (int j = 0; j < 9; j++) {
      probe.data[j][0] = probe_datap[i * 36 + j * 4];
      probe.data[j][1] = probe_datap[i * 36 + j * 4 + 1];
      probe.data[j][2] = probe_datap[i * 36 + j * 4 + 2];
      if (lightbuilder_cat.is_debug()) {
        lightbuilder_cat.debug(false)
          << "\t" << probe.data[j] << "\n";
      }
    }
  }

  //_graphics_engine->extract_texture_data(_lm_textures["probes_flat"], _gsg);
  //CPTA_uchar probe_flat_data = _lm_textures["probes_flat"]->get_ram_image();
  //const float *probe_flat_datap = (const float *)probe_flat_data.p();

  //LVecBase3 color;
  //for (size_t i = 0; i < _probes.size(); i++) {
  //  color[0] = probe_flat_datap[i * 4];
  //  color[1] = probe_flat_datap[i * 4 + 1];
  //  color[2] = probe_flat_datap[i * 4 + 2];
  //  lightbuilder_cat.info()
  //    << "Probe " << i << " flat color: " << color << "\n";
  //}

  free_texture(_lm_textures["probes"]);
  free_texture(_lm_textures["albedo"]);
  _graphics_engine->render_frame();

  lightbuilder_cat.info()
    << "Done.\n";

  return true;
}

/**
 * Does the lightmap solve.  Returns true on success or false if something
 * went wrong.
 */
bool LightBuilder::
solve() {
  /**
   * Here's what we need to compute a lightmap for each Geom.
   *
   * A "luxel" is a lightmap texel, in the lightmap UV set.
   *
   * In lightmap UV space:
   * - Luxel world position
   * - Luxel surface normal
   * - Luxel albedo (reflectivity)
   * - Luxel emission (emissive surfaces)
   *
   * When we have this information, we can compute a light value for each luxel.
   * - Luxel direct lighting
   * - Luxel indirect lighting
   */

  // First sort all LightmapGeoms by light mode so the corresponding
  // LightmapTris are also sorted that way.
  std::sort(_geoms.begin(), _geoms.end(), [](const LightmapGeom &a, const LightmapGeom &b) {
    return a.light_mode < b.light_mode;
  });

  // A bug in ShaderModuleSpirV is messing up ray tracing, so force
  // the shaders to compile to GLSL.
  load_prc_file_data("lightmap", "gl-support-spirv 0");
  load_prc_file_data("lightmap", "gl-coordinate-system default");
  load_prc_file_data("lightmap", "gl-enable-memory-barriers 0");
  load_prc_file_data("lightmap", "threading-model");

  if (!initialize_pipe()) {
    lightbuilder_cat.error()
      << "Failed to initialize graphics pipe for lightmap building\n";
    return false;
  }

  if (!make_palette()) {
    lightbuilder_cat.error()
      << "Failed to generate lightmap palettes\n";
    return false;
  }

  if (!offset_geom_lightmap_uvs()) {
    lightbuilder_cat.error()
      << "Failed to offset Geom lightmap UVs in palettes\n";
    return false;
  }

  if (!collect_vertices_and_triangles()) {
    lightbuilder_cat.error()
      << "Failed to collect scene vertices and triangles\n";
    return false;
  }

  if (!build_kd_tree()) {
    lightbuilder_cat.error()
      << "Failed to build K-D tree\n";
    return false;
  }

  if (!make_textures()) {
    lightbuilder_cat.error()
      << "Failed to create lightmap textures\n";
    return false;
  }

  if (!make_gpu_buffers()) {
    lightbuilder_cat.error()
      << "Failed to create GPU data structures\n";
    return false;
  }

  if (!rasterize_geoms_into_lightmap_textures()) {
    lightbuilder_cat.error()
      << "Failed to rasterize geoms into lightmap textures\n";
    return false;
  }

  if (!rasterize_vertex_lit_geoms()) {
    lightbuilder_cat.error()
      << "Falied to rasterize vertex lit geoms\n";
    return false;
  }

  // After rasterizing, get rid of the non-indexed Geom copies.
  for (LightmapGeom &lgeom : _geoms) {
    lgeom.ni_geom = nullptr;
    lgeom.ni_vdata = nullptr;
  }

  //_graphics_engine->extract_texture_data(_lm_textures["vtx_albedo"], _gsg);
  //_lm_textures["vtx_albedo"]->write("lm_vtx_albedo.png");

  if (!compute_unocclude()) {
    lightbuilder_cat.info()
      << "Failed to compute luxel unocclusion\n";
    return false;
  }

  //_graphics_engine->extract_texture_data(_lm_textures["position"], _gsg);
  //_lm_textures["position"]->write("lm_position_#.png", 0, 0, true, false);

  if (!compute_direct()) {
    lightbuilder_cat.error()
      << "Failed to compute luxel direct lighting\n";
    return false;
  }

  if (!compute_vtx_reflectivity()) {
    lightbuilder_cat.error()
      << "Failed to compute per-vertex reflectivity\n";
    return false;
  }

  //_graphics_engine->extract_texture_data(_lm_textures["vtx_refl"], _gsg);
  //_lm_textures["vtx_refl"]->write("lm_vtx_refl.png");

  if (!compute_indirect()) {
    lightbuilder_cat.error()
      << "Failed to compute luxel indirect lighting\n";
    return false;
  }

  //if (!compute_probes()) {
  //  lightbuilder_cat.error()
  //    << "Failed to compute ambient probes\n";
 // }

  if (!dialate_lightmaps()) {
    lightbuilder_cat.error()
      << "Failed to dialate lightmaps\n";
    return false;
  }

  //_graphics_engine->extract_texture_data(_lm_textures["reflectivity"], _gsg);

  if (!denoise_lightmaps()) {
    lightbuilder_cat.error()
      << "Failed to denoise lightmaps\n";
    return false;
  }

  if (!write_geoms()) {
    lightbuilder_cat.error()
      << "Failed to write lightmaps to input Geoms\n";
    return false;
  }

  // Now release all of our resources.
  for (auto it = _lm_textures.begin(); it != _lm_textures.end(); ++it) {
    Texture *tex = (*it).second;
    if (tex != nullptr) {
      free_texture(tex);
    } else {
      lightbuilder_cat.warning()
        << "Null texture: " << (*it).first << "\n";
    }

  }
  for (auto it = _gpu_buffers.begin(); it != _gpu_buffers.end(); ++it) {
    Texture *buf = (*it).second;
    if (buf != nullptr) {
      free_texture(buf);
    } else {
      lightbuilder_cat.warning()
        << "Null buffer: " << (*it).first << "\n";
    }

  }
  _graphics_engine->render_frame();
  _graphics_engine->remove_window(_host_output);
  _graphics_engine->render_frame();
  _host_output = nullptr;
  _gsg = nullptr;
  _graphics_pipe = nullptr;

  return true;
}

/**
 * Returns the name of the lightmap texture coordinate set.
 *
 * The lightmapper expects geometry lightmap UV's to be in a texcoord set
 * with this name.  From Maya or Blender, give the model's lightmap UV set
 * this name.
 */
const InternalName *LightBuilder::
get_lightmap_uv_name() {
  if (_lightmap_uv_name == nullptr) {
    _lightmap_uv_name = InternalName::get_texcoord_name("lightmap");
  }
  return _lightmap_uv_name;
}

/**
 *
 */
void LightBuilder::
free_texture(Texture *tex) {
  tex->clear_image();
  //tex->release(_gsg->get_prepared_objects());
}

/**
 * Converts the given RAM image, assumed to be in RGBA32 format, to RGB16
 * half-float.  The given image is expected to be a single page of a single
 * mipmap level.
 */
void LightBuilder::
convert_rgba32_to_rgb16(const unsigned char *image, size_t image_size,
                        const LVecBase2i &orig_size, const LVecBase2i &new_size,
                        unsigned char *out) {

  int y_diff = orig_size[1] - new_size[1];
  int x_diff = orig_size[0] - new_size[0];

  // New size should be same or smaller.
  assert(y_diff >= 0 && x_diff >= 0);

  const float *fp32_data = (const float *)image;
  unsigned short *fp16_data = (unsigned short *)out;

  for (int y = 0; y < orig_size[1] - y_diff; ++y) {
    for (int x = 0; x < orig_size[0] - x_diff; ++x) {

      int orig_pos = y * orig_size[0] * 4;
      orig_pos += x * 4;

      // Convert to half float and store in new image.
      *fp16_data++ = fp16_ieee_from_fp32_value(fp32_data[orig_pos]);
      *fp16_data++ = fp16_ieee_from_fp32_value(fp32_data[orig_pos + 1]);
      *fp16_data++ = fp16_ieee_from_fp32_value(fp32_data[orig_pos + 2]);
    }
  }
}

/**
 * Converts the given RAM image, assumed to be in RGBA32 format, to RGB8
 * unsigned byte.  The given image is expected to be a single page of a single
 * mipmap level.
 */
void LightBuilder::
convert_rgba32_to_rgb8(const unsigned char *image, size_t image_size,
                        const LVecBase2i &orig_size, const LVecBase2i &new_size,
                        unsigned char *out) {

  int y_diff = orig_size[1] - new_size[1];
  int x_diff = orig_size[0] - new_size[0];

  // New size should be same or smaller.
  assert(y_diff >= 0 && x_diff >= 0);

  const float *fp32_data = (const float *)image;
  unsigned char *u8_data = out;

  for (int y = 0; y < orig_size[1] - y_diff; ++y) {
    for (int x = 0; x < orig_size[0] - x_diff; ++x) {

      int orig_pos = y * orig_size[0] * 4;
      orig_pos += x * 4;

      // Convert to unsigned byte and store in new image.
      *u8_data++ = (unsigned char)std::clamp((int)lroundf(fp32_data[orig_pos] * 255.0f), 0, 255);
      *u8_data++ = (unsigned char)std::clamp((int)lroundf(fp32_data[orig_pos + 1] * 255.0f), 0, 255);
      *u8_data++ = (unsigned char)std::clamp((int)lroundf(fp32_data[orig_pos + 2] * 255.0f), 0, 255);
    }
  }
}

/**
 *
 */
bool LightBuilder::
compress_rgb16_to_bc6h(Texture *tex) {

  SamplerState::FilterType orig_minfilter = tex->get_minfilter();
  SamplerState::FilterType orig_magfilter = tex->get_magfilter();

  // We don't want to do any interpolation of the texture colors
  // when sampling for compression.
  tex->set_minfilter(SamplerState::FT_nearest);
  tex->set_magfilter(SamplerState::FT_nearest);
  tex->set_wrap_u(SamplerState::WM_clamp);
  tex->set_wrap_v(SamplerState::WM_clamp);

  int width = tex->get_x_size();
  int height = tex->get_y_size();

  int block_width = (width + 3) >> 2;
  int block_height = (height + 3) >> 2;

  PT(Texture) dst_tex = new Texture("bc6h-dst");
  dst_tex->setup_2d_texture(block_width, block_height, Texture::T_unsigned_int, Texture::F_rgba32i);
  dst_tex->set_compression(Texture::CM_off);
  dst_tex->set_minfilter(SamplerState::FT_nearest);
  dst_tex->set_magfilter(SamplerState::FT_nearest);
  dst_tex->set_wrap_u(SamplerState::WM_clamp);
  dst_tex->set_wrap_v(SamplerState::WM_clamp);
  dst_tex->clear_image();

  NodePath tmp("tmp");
  tmp.set_shader(Shader::load_compute(Shader::SL_GLSL, "shaders/compress_bc6h.compute.glsl"));
  tmp.set_shader_input("srcTexture", tex);
  tmp.set_shader_input("dstTexture", dst_tex, false, true);
  tmp.set_shader_input("p_textureSizeRcp", LVecBase2(1.0f / width, 1.0f / height));

  lightbuilder_cat.info()
    << "Compressing " << tex->get_name() << " to BC6H..\n";

  _gsg->set_state_and_transform(tmp.get_state(), TransformState::make_identity());
  _gsg->dispatch_compute(align_to_next_multiple(width, 32) / 32,
                         align_to_next_multiple(height, 32) / 32, 1, true);

  lightbuilder_cat.info()
    << "Done.\n";

  // Store original filter settings.
  tex->set_minfilter(orig_minfilter);
  tex->set_magfilter(orig_magfilter);

  if (!_gsg->extract_texture_data(dst_tex)) {
    lightbuilder_cat.warning()
      << "Failed to extract compressed texture data\n";
    return false;
  }

  tex->set_ram_image(dst_tex->get_ram_image(), Texture::CM_bptc);
  tex->set_compression(Texture::CM_bptc);

  return true;
}

/**
 *
 */
void LightBuilder::
apply_kd_uniforms(NodePath np, int override) {
  np.set_shader_input("vertices", _gpu_buffers["vertices"], override);
  np.set_shader_input("triangles", _gpu_buffers["triangles"], override);
  np.set_shader_input("lights", _gpu_buffers["lights"], override);
  np.set_shader_input("kd_nodes", _gpu_buffers["kd_tree"], override);
  np.set_shader_input("kd_leaves", _gpu_buffers["kd_leaves"], override);
  np.set_shader_input("kd_triangles", _gpu_buffers["kd_tri_list"], override);
  np.set_shader_input("scene_mins", _scene_mins, override);
  np.set_shader_input("scene_maxs", _scene_maxs, override);
}
