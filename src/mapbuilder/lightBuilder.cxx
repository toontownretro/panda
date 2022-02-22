/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightBuilder.cxx
 * @author lachbr
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

#ifndef CPPPARSER
#include "OpenImageDenoise/oidn.hpp"
#endif

NotifyCategoryDef(lightbuilder, "");

CPT(InternalName) LightBuilder::_lightmap_uv_name = nullptr;

/**
 *
 */
LightBuilder::
LightBuilder() :
  _lightmap_size(-1),
  _grid_size(128),
  _bias(0.005f),
  _bounces(5), // 5
  _rays_per_luxel(1024),
  _ray_region_size(512),
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
    mapbuilder_cat.info()
      << "Skipping empty Geom\n";
    return;
  }

  if (geom->get_primitive_type() != Geom::PT_polygons) {
    // Only polygons get lightmapped.
    mapbuilder_cat.info()
      << "Skipping non-polygon Geom\n";
    return;
  }

  // Can't lightmap it if it doesn't have the lightmap UV set.
  if (!geom->get_vertex_data()->has_column(get_lightmap_uv_name())) {
    mapbuilder_cat.info()
      << "Skipping Geom without lightmap UV set\n";
    return;
  }

  PT(Geom) the_geom = geom->make_copy();
  // Decompose tristrips/trifans into triangles if there are any on the Geom.
  the_geom->decompose_in_place();

  LightmapGeom lgeom;
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

  // Pack each Geom into a palette.
  for (size_t i = 0; i < _geoms.size(); i++) {
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
      }
    }

    if (!added) {
      // If the geom was not added into any palette above, that means that we
      // don't have any palettes yet, or the Geom couldn't fit into any
      // existing palette.  Try again on a fresh palette.
      LightmapPage page;
      page.index = _pages.size();
      page.packer.reset(0, 8192, 8192, 4);
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
    LVecBase2i size = _pages[i].packer.get_minimum_dimensions();
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

    PN_stdfloat u_scale = lmgeom.lightmap_size[0] / (PN_stdfloat)_lightmap_size[0];
    PN_stdfloat u_offset = lmgeom.palette_offset[0] / (PN_stdfloat)_lightmap_size[0];
    PN_stdfloat v_scale = lmgeom.lightmap_size[1] / (PN_stdfloat)_lightmap_size[1];
    PN_stdfloat v_offset = lmgeom.palette_offset[1] / (PN_stdfloat)_lightmap_size[1];

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
          LVecBase2 new_uv(uv[0] * u_scale + u_offset,
                           uv[1] * v_scale + v_offset);
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

  SamplerState sampler;
  sampler.set_minfilter(SamplerState::FT_linear);
  sampler.set_magfilter(SamplerState::FT_linear);
  sampler.set_min_lod(0);
  sampler.set_max_lod(0);
  sampler.set_anisotropic_degree(0);

  // Color of direct lighting reaching a luxel.
  PT(Texture) direct = new Texture("lm_direct");
  direct->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                 Texture::T_float, Texture::F_rgba32);
  direct->set_clear_color(LColor(0, 0, 0, 0));
  direct->set_default_sampler(sampler);
  direct->set_compression(Texture::CM_off);
  direct->clear_image();
  _lm_textures["direct"] = direct;

  PT(Texture) direct_dynamic = new Texture("lm_direct_dynamic");
  direct_dynamic->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
                                         Texture::T_float, Texture::F_rgba32);
  direct_dynamic->set_clear_color(LColor(0, 0, 0, 0));
  direct_dynamic->set_default_sampler(sampler);
  direct_dynamic->set_compression(Texture::CM_off);
  direct_dynamic->clear_image();
  _lm_textures["direct_dynamic"] = direct;

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
  reflectivity->setup_2d_texture_array(_lightmap_size[0], _lightmap_size[1], _pages.size(),
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

  return true;
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

      // Make a non-indexed copy of the Geom for collecting vertices and
      // triangles and rasterizing into lightmap textures.  In the rasterizer
      // shader we have to determine the triangle index from the vertex index,
      // which is not possible with indexed triangles.
      geom.ni_geom = geom.geom->make_copy();
      geom.ni_geom->make_nonindexed(false);

      // Making the Geom non-indexed will create a new GeomVertexData that
      // *only* contains the vertices referenced by the primitives of the Geom,
      // so we know that every vertex in the resulting vdata is actually used
      // by the Geom.  We can also guarantee that every 3 consecutive rows in
      // the vdata corresponds to a triangle.
      geom.ni_vdata = geom.ni_geom->get_vertex_data();

      // Since we're dealing with non-indexed triangles, the number of rows
      // should be a multiple of three.
      nassertr((geom.ni_vdata->get_num_rows() % 3) == 0, false);

      GeomVertexReader vreader(geom.ni_vdata, InternalName::get_vertex());
      GeomVertexReader nreader(geom.ni_vdata, InternalName::get_normal());
      GeomVertexReader uvreader(geom.ni_vdata, get_lightmap_uv_name());

      LMatrix4 mat = geom.net_transform->get_mat();

      while (!vreader.is_at_end()) {
        LPoint3 pos = vreader.get_data3f();
        LVector3 normal = nreader.get_data3f();
        LVecBase2 uv = uvreader.get_data2f();

        // Transform position and normal into world coordinates.
        pos = mat.xform_point(pos);
        normal = mat.xform_vec(normal).normalized();

        LightmapVertex v;
        v.pos = pos;
        v.normal = normal;
        v.uv = uv;

        _vertices.push_back(std::move(v));
      }

      geom.first_triangle = triangle;

      // Now collect the triangles.
      for (int k = 0; k < geom.num_triangles; k++) {
        LightmapTri tri;
        // Since they're non-indexed we know the triangle vertices are laid
        // out consecutively like this.
        tri.indices[0] = vertex + (k * 3);
        tri.indices[1] = vertex + (k * 3) + 1;
        tri.indices[2] = vertex + (k * 3) + 2;

        // Carry over some information from the Geom.
        tri.palette = geom.palette;
        tri.contents = geom.contents;

        // Compute triangle bounds.
        tri.mins.set(1e24, 1e24, 1e24);
        tri.maxs.set(-1e24, -1e24, -1e24);
        for (int l = 0; l < 3; l++) {
          // Expand the bounds by a tiny bit to avoid precision errors.
          tri.mins[0] = std::min(tri.mins[0], _vertices[tri.indices[l]].pos[0] - 0.001f);
          tri.mins[1] = std::min(tri.mins[1], _vertices[tri.indices[l]].pos[1] - 0.001f);
          tri.mins[2] = std::min(tri.mins[2], _vertices[tri.indices[l]].pos[2] - 0.001f);
          tri.maxs[0] = std::max(tri.maxs[0], _vertices[tri.indices[l]].pos[0] + 0.001f);
          tri.maxs[1] = std::max(tri.maxs[1], _vertices[tri.indices[l]].pos[1] + 0.001f);
          tri.maxs[2] = std::max(tri.maxs[2], _vertices[tri.indices[l]].pos[2] + 0.001f);
        }

        _triangles.push_back(std::move(tri));
      }

      // Increment vertex and triangle offsets for next Geom.
      vertex += geom.ni_vdata->get_num_rows();
      triangle += geom.num_triangles;
    }
  }

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
    tri_datap[i * 12 + 3] = tri.palette;

    // Texel 1: minx, miny, minz, contents
    tri_datap[i * 12 + 4] = tri.mins[0];
    tri_datap[i * 12 + 5] = tri.mins[1];
    tri_datap[i * 12 + 6] = tri.mins[2];
    tri_datap[i * 12 + 7] = tri.contents;

    // Texel 2: maxx, maxy, maxz, unused
    tri_datap[i * 12 + 8] = tri.maxs[0];
    tri_datap[i * 12 + 9] = tri.maxs[1];
    tri_datap[i * 12 + 10] = tri.maxs[2];
    tri_datap[i * 12 + 11] = 0.0f;
  }
  triangles->set_ram_image(tri_data);
  _gpu_buffers["triangles"] = triangles;

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

  // Now build the triangle acceleration stucture for ray tracing.  Currently
  // it is a uniform grid of cells.  Each cell contains a list of triangles
  // that overlap with the cell.  Would like to eventually do a K-D tree using
  // the stackless traversal algorithm.

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
  // Expand a tiny bit.
  _scene_mins -= LVector3(0.05);
  _scene_maxs += LVector3(0.05);

  lightbuilder_cat.info()
    << "Scene mins: " << _scene_mins << ", maxs: " << _scene_maxs << "\n";

  // Now determine all of the cell-triangle pairings.  This does an octree
  // traversal so we don't have to test every triangle against every cell.
  // The result of this is a huge vector of cell-triangle pairings.
  TriSorts tri_sort;
  for (size_t i = 0; i < _triangles.size(); i++) {
    const LightmapTri &tri = _triangles[i];
    LPoint3 points[3] = {
      _vertices[tri.indices[0]].pos,
      _vertices[tri.indices[1]].pos,
      _vertices[tri.indices[2]].pos
    };
    add_tri_to_grid(_grid_size, LVecBase3i(0), _scene_mins, _scene_maxs, points,
                    i, tri_sort);
  }

  // Now sort all of the pairings by increasing cell index.
  std::sort(tri_sort.begin(), tri_sort.end(),
    [](const TriSort &a, const TriSort &b) -> bool {
      return a.cell_index < b.cell_index;
    }
  );

  lightbuilder_cat.info()
    << tri_sort.size() << " cell-triangle pairings\n";
  if (lightbuilder_cat.is_debug()) {
    for (size_t i = 0; i < tri_sort.size(); i++) {
      lightbuilder_cat.debug()
        << "Cell " << tri_sort[i].cell_index << ", tri " << tri_sort[i].tri_index << "\n";
    }
  }

  // This is simply an array of indices into the LightmapTri array.
  PTA_uchar tri_indices;
  tri_indices.resize(sizeof(uint32_t) * tri_sort.size());
  uint32_t *tri_indicesp = (uint32_t *)tri_indices.p();

  // A 3-D texture, one two-channel texel per grid cell.  The first
  // channel specifies a start index into the triangle index array,
  // and the second channel specifies a triangle count.  This identifies
  // the set of triangles associated with a cell.
  PTA_uchar grid_indices;
  grid_indices.resize(sizeof(uint32_t) * (_grid_size * _grid_size * _grid_size * 2));
  memset(grid_indices.p(), 0, grid_indices.size());
  uint32_t *grid_indicesp = (uint32_t *)grid_indices.p();

  uint32_t last_cell = 0xFFFFFFFF;
  for (size_t i = 0; i < tri_sort.size(); i++) {
    uint32_t cell = tri_sort[i].cell_index;
    if (cell != last_cell) {
      // Cell changed, update pointer to indices.
      grid_indicesp[cell * 2 + 1] = i;
    }
    tri_indicesp[i] = tri_sort[i].tri_index;
    grid_indicesp[cell * 2]++;
    last_cell = cell;
  }

  PT(Texture) tri_cells = new Texture("lm_tri_cells");
  tri_cells->setup_buffer_texture(tri_sort.size(), Texture::T_unsigned_int,
                                  Texture::F_r32i, GeomEnums::UH_static);
  tri_cells->set_compression(Texture::CM_off);
  tri_cells->set_keep_ram_image(false);
  nassertr(tri_cells->get_expected_ram_image_size() == tri_indices.size(), false);
  tri_cells->set_ram_image(tri_indices);
  _gpu_buffers["triangle_cells"] = tri_cells;

  PT(Texture) grid = new Texture("lm_grid");
  grid->setup_3d_texture(_grid_size, _grid_size, _grid_size, Texture::T_unsigned_int,
                         Texture::F_rg32i);
  grid->set_compression(Texture::CM_off);
  grid->set_keep_ram_image(false);
  nassertr(grid->get_expected_ram_image_size() == grid_indices.size(), false);
  grid->set_ram_image(grid_indices);
  grid->set_minfilter(SamplerState::FT_linear);
  grid->set_magfilter(SamplerState::FT_linear);
  _gpu_buffers["grid"] = grid;

  return true;
}

/**
 * Places a triangle in all of the grid cells that it intersects with.
 */
void LightBuilder::
add_tri_to_grid(int size, const LVecBase3i &offset, const LPoint3 &mins,
                const LPoint3 &maxs, LPoint3 points[3],
                int tri_index, TriSorts &triangles) {
  int half_size = size / 2;

  // Divide into eight octants.  Test triangle against each octant AABB.
  // If the triangle overlaps an octant, add it to the corresponding cell
  // if we're at the lowest level, otherwise keep dividing.
  for (int i = 0; i < 8; i++) {
    LPoint3 this_mins = mins;
    LPoint3 this_maxs = maxs;
    LVecBase3i this_ofs = offset;

    LVector3 hsize = this_maxs - this_mins;
    hsize *= 0.5f;

    if ((i & 4) != 0) {
      this_mins[0] += hsize[0];
      this_ofs[0] += half_size;
    }
    if ((i & 2) != 0) {
      this_mins[1] += hsize[1];
      this_ofs[1] += half_size;
    }
    if ((i & 1) != 0) {
      this_mins[2] += hsize[2];
      this_ofs[2] += half_size;
    }

    this_maxs = this_mins + hsize;

    LVector3 qsize = hsize * 0.5f;

    if (!tri_box_overlap(this_mins + qsize, qsize, points[0], points[1], points[2])) {
      // Doesn't overlap with the octant.
      continue;
    }

    if (half_size == 1) {
      // The triangle overlaps with the octant and we reached a cell.
      // Add it to the cell.
      int cell_index = this_ofs[0] + (this_ofs[1] * _grid_size) + (this_ofs[2] * _grid_size * _grid_size);
      TriSort ts;
      ts.cell_index = cell_index;
      ts.tri_index = tri_index;
      triangles.push_back(std::move(ts));

    } else {
      // Keep descending until we hit a cell.
      add_tri_to_grid(half_size, this_ofs, this_mins, this_maxs, points, tri_index, triangles);
    }
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

  CPT(RenderState) uv_offset_states[25];
  for (size_t i = 0; i < 25; i++) {
    CPT(RenderAttrib) sattr = ShaderAttrib::make();
    uv_offset_states[i] = RenderState::make(
      DCAST(ShaderAttrib, sattr)->set_shader_input(
        ShaderInput("u_uv_offset", LVecBase2(uv_offsets[i][0] * (1.0f / (PN_stdfloat)_lightmap_size[0]),
                                             uv_offsets[i][1] * (1.0f / (PN_stdfloat)_lightmap_size[1])))),
      10
    );
  }

  // Now create a display region for each lightmap palette that will render
  // the Geoms that are part of that palette.  Each display region will
  // render into the correct page of the array texture.
  for (size_t i = 0; i < _pages.size(); i++) {
    const LightmapPage &page = _pages[i];

    NodePath root("root");

    // Don't do any front or back face culling.
    root.set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none), 10);
    // Don't allow Panda to do dualing transparency if a Geom enables it.
    root.set_transparency(TransparencyAttrib::M_none, 10);
    //root.set_antialias(AntialiasAttrib::M_multisample, 10);
    // If we write to or test against the depth buffer, Geoms rendered in one
    // page may be occluded by Geoms that were rendered in a different page.
    root.set_depth_write(true, 10);
    root.set_depth_test(true, 10);
    root.set_bin("unsorted", 10);
    root.set_attrib(ColorBlendAttrib::make_off(), 10);

    //root.set_render_mode_filled_wireframe()

    root.set_shader(Shader::load(Shader::SL_GLSL, "shaders/lm_raster.vert.glsl", "shaders/lm_raster.frag.glsl"), 10);
    root.set_shader_input("vertices", _gpu_buffers["vertices"], 10);
    root.set_shader_input("triangles", _gpu_buffers["triangles"], 10);
    root.set_shader_input("lights", _gpu_buffers["lights"], 10);
    root.set_shader_input("triangle_cells", _gpu_buffers["triangle_cells"], 10);
    root.set_shader_input("grid", _gpu_buffers["grid"], 10);

    scenes.push_back(root);

    PT(DisplayRegion) dr = buffer->make_display_region();
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

      // Determine the base color/albedo texture of the Geom.
      PT(Texture) base_tex;
      bool has_base_color = false;
      LColor base_color(1);
      LVecBase3 emission_color(0, 0, 0);

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
          MaterialParamBase *selfillum_param = mat->get_param("self_illum");
          if (selfillum_param != nullptr && DCAST(MaterialParamBool, selfillum_param)->get_value()) {
            PN_stdfloat emission_factor = 255.0f;
            MaterialParamBase *emission_param = mat->get_param("emission");
            if (emission_param != nullptr && emission_param->is_of_type(MaterialParamFloat::get_class_type())) {
              emission_factor = DCAST(MaterialParamFloat, emission_param)->get_value();
            }
            emission_color.set(emission_factor / 255.0f, emission_factor / 255.0f, emission_factor / 255.0f);
            MaterialParamBase *tint_param = mat->get_param("self_illum_tint");
            if (tint_param != nullptr) {
              emission_color = DCAST(MaterialParamColor, tint_param)->get_value().get_xyz();
              emission_color[0] = std::pow(emission_color[0], 2.2f);
              emission_color[1] = std::pow(emission_color[1], 2.2f);
              emission_color[2] = std::pow(emission_color[2], 2.2f);
              emission_color *= emission_factor / 255.0f;
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

      PT(GeomNode) geom_node = new GeomNode("lm_geom");

      for (size_t k = 0; k < 25; k++) {
        geom_node->add_geom(geom.ni_geom, geom.state->compose(uv_offset_states[k]));
      }

      geom_node->set_transform(geom.net_transform);
      NodePath geom_np = root.attach_new_node(geom_node);
      geom_np.set_shader_input("base_texture_sampler", base_tex, 10);
      geom_np.set_shader_input("emission_color", emission_color, 10);
      geom_np.set_shader_input("first_triangle", LVecBase2i(geom.first_triangle), 10);
    }
  }

  // Now render everything and block until it's done.
  _graphics_engine->render_frame();
  _graphics_engine->sync_frame();

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

  np.set_shader_input("vertices", _gpu_buffers["vertices"]);
  np.set_shader_input("triangles", _gpu_buffers["triangles"]);
  np.set_shader_input("lights", _gpu_buffers["lights"]);
  np.set_shader_input("triangle_cells", _gpu_buffers["triangle_cells"]);
  np.set_shader_input("grid", _gpu_buffers["grid"]);

  np.set_shader_input("position", _lm_textures["position"]);
  np.set_shader_input("unocclude", _lm_textures["unocclude"]);

  np.set_shader_input("u_bias_", LVecBase2(_bias));
  np.set_shader_input("u_region_ofs_grid_size", LVecBase3i(0, 0, _grid_size));
  np.set_shader_input("u_to_cell_offset", _scene_mins);

  LVector3 size = _scene_maxs - _scene_mins;
  np.set_shader_input("u_to_cell_size", LVecBase3((1.0f / size[0]) * (float)_grid_size,
                                                  (1.0f / size[1]) * (float)_grid_size,
                                                  (1.0f / size[2]) * (float)_grid_size));

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
  _lm_textures["unocclude"]->clear_image();

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

  np.set_shader_input("vertices", _gpu_buffers["vertices"]);
  np.set_shader_input("triangles", _gpu_buffers["triangles"]);
  np.set_shader_input("lights", _gpu_buffers["lights"]);
  np.set_shader_input("triangle_cells", _gpu_buffers["triangle_cells"]);
  np.set_shader_input("grid", _gpu_buffers["grid"]);

  np.set_shader_input("luxel_direct", _lm_textures["direct"]);
  np.set_shader_input("luxel_direct_dynamic", _lm_textures["direct_dynamic"]);
  np.set_shader_input("luxel_reflectivity", _lm_textures["reflectivity"]);
  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);
  np.set_shader_input("luxel_position", _lm_textures["position"]);
  np.set_shader_input("luxel_normal", _lm_textures["normal"]);
  np.set_shader_input("luxel_emission", _lm_textures["emission"]);

  np.set_shader_input("u_bias_sun_extent", LVecBase2(_bias, std::sin(deg_2_rad(_sun_angular_extent))));
  np.set_shader_input("u_region_ofs_grid_size", LVecBase3i(0, 0, _grid_size));
  np.set_shader_input("u_to_cell_offset", _scene_mins);

  LVector3 size = _scene_maxs - _scene_mins;
  np.set_shader_input("u_to_cell_size", LVecBase3((1.0f / size[0]) * (float)_grid_size,
                                                  (1.0f / size[1]) * (float)_grid_size,
                                                  (1.0f / size[2]) * (float)_grid_size));

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

  np.set_shader_input("vertices", _gpu_buffers["vertices"]);
  np.set_shader_input("triangles", _gpu_buffers["triangles"]);
  np.set_shader_input("lights", _gpu_buffers["lights"]);
  np.set_shader_input("triangle_cells", _gpu_buffers["triangle_cells"]);
  np.set_shader_input("grid", _gpu_buffers["grid"]);

  np.set_shader_input("luxel_indirect", _lm_textures["indirect"]);
  np.set_shader_input("luxel_indirect_accum", _lm_textures["indirect_accum"]);
  np.set_shader_input("luxel_reflectivity", _lm_textures["reflectivity"]);
  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);
  np.set_shader_input("luxel_position", _lm_textures["position"]);
  np.set_shader_input("luxel_normal", _lm_textures["normal"]);
  np.set_shader_input("luxel_light", _lm_textures["direct"]);

  np.set_shader_input("u_bias_", LVecBase2(_bias));
  np.set_shader_input("u_region_ofs_grid_size", LVecBase3i(0, 0, _grid_size));
  np.set_shader_input("u_to_cell_offset", _scene_mins);
  np.set_shader_input("u_sky_color", _sky_color.get_xyz());

  LVector3 size = _scene_maxs - _scene_mins;
  np.set_shader_input("u_to_cell_size", LVecBase3((1.0f / size[0]) * (float)_grid_size,
                                                  (1.0f / size[1]) * (float)_grid_size,
                                                  (1.0f / size[2]) * (float)_grid_size));

  int x_regions = (_lightmap_size[0] - 1) / _ray_region_size + 1;
  int y_regions = (_lightmap_size[1] - 1) / _ray_region_size + 1;
  int ray_iters = (_rays_per_luxel - 1) / _rays_per_region + 1;

  int total_regions = _pages.size() * x_regions * y_regions;

  for (int b = 0; b < _bounces; b++) {
    lightbuilder_cat.info()
      << "Bounce " << b + 1 << "...\n";
    for (size_t i = 0; i < _pages.size(); i++) {
      const LightmapPage &page = _pages[i];

      np.set_shader_input("u_palette_size_page_bounce", LVecBase4i(_lightmap_size[0], _lightmap_size[1], i, b));

      for (int j = 0; j < x_regions; j++) {
        for (int k = 0; k < y_regions; k++) {
          int x = j * _ray_region_size;
          int y = k * _ray_region_size;
          int w = std::min((j + 1) * _ray_region_size, _lightmap_size[0]) - x;
          int h = std::min((k + 1) * _ray_region_size, _lightmap_size[1]) - y;

          np.set_shader_input("u_region_ofs_grid_size", LVecBase3i(x, y, _grid_size));

          LVecBase3i group_size((w - 1) / 8 + 1, (h - 1) / 8 + 1, 1);

          for (int l = 0; l < ray_iters; l++) {
            int ray_from = l * _rays_per_region;
            int ray_to = std::min((l + 1) * _rays_per_region, _rays_per_luxel);

            np.set_shader_input("u_ray_params", LVecBase3i(ray_from, ray_to, _rays_per_luxel));

            _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
            _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
            _gsg->finish();
          }
        }
      }
    }
  }

  // Free up memory.
  _lm_textures["indirect_accum"]->clear_image();
  _lm_textures["indirect"]->clear_image();
  //_lm_textures["albedo"]->clear_image();
  _lm_textures["position"]->clear_image();
  _lm_textures["normal"]->clear_image();

  lightbuilder_cat.info()
    << "Done.\n";

  return true;
}

/**
 * Uses Intel OpenImageDenoise to denoise the computed lightmaps.
 */
bool LightBuilder::
denoise_lightmaps() {
  lightbuilder_cat.info()
    << "Denoising lightmaps...\n";

  oidn::DeviceRef device = oidn::newDevice();
  device.commit();

  // Bring the needed textures into system RAM.
  _graphics_engine->extract_texture_data(_lm_textures["direct"], _gsg);

  // Get the ram image without an alpha channel.  OIDN wants a 3-channel image.
  CPTA_uchar color_data = _lm_textures["direct"]->get_ram_image_as("RGB");
  CPTA_uchar alpha_data = _lm_textures["direct"]->get_ram_image_as("A");
  size_t page_size = sizeof(float) * 3 * _lightmap_size[0] * _lightmap_size[1];

  oidn::FilterRef filter = device.newFilter("RTLightmap");
  // Denoise each page.
  for (size_t i = 0; i < _pages.size(); i++) {
    void *page_ptr = (void *)(color_data.p() + i * page_size);

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

  PTA_uchar new_data;
  new_data.resize(_lm_textures["direct"]->get_expected_ram_image_size());
  float *new_datap = (float *)new_data.p();
  const float *color_datap = (const float *)color_data.p();
  const float *alpha_datap = (const float *)alpha_data.p();

  for (int i = 0; i < _lightmap_size[0] * _lightmap_size[1] * _pages.size(); i++) {
    new_datap[i * 4] = color_datap[i * 3];
    new_datap[i * 4 + 1] = color_datap[i * 3 + 1];
    new_datap[i * 4 + 2] = color_datap[i * 3 + 2];
    new_datap[i * 4 + 3] = alpha_datap[i];
  }

  // Now throw it back on the texture.
  _lm_textures["direct"]->set_ram_image_as(new_data, "RGBA");
  _lm_textures["direct"]->set_format(Texture::F_rgba32);

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

  np.set_shader_input("u_region_ofs_grid_size", LVecBase3i(0, 0, _grid_size));

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
  _lm_textures["direct"]->clear_image();

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

  // Bring the lightmap texture into system RAM.
  // The reflectivity texture is re-used as the dilation output.
  if (!_graphics_engine->extract_texture_data(_lm_textures["reflectivity"], _gsg)) {
    lightbuilder_cat.error()
      << "Couldn't bring lightmap texture into system RAM\n";
    return false;
  }

  //_lm_textures["reflectivity"]->write("lm_final_light_#.png", 0, 0, true, false);

  // Lightmaps are identified in the shader generator by a texture stage
  // named "lightmap".
  PT(TextureStage) stage = new TextureStage("lightmap");

  pvector<CPT(RenderState)> page_texture_states;
  // Extract each page from the lightmap array texture into individual textures.
  for (size_t i = 0; i < _pages.size(); i++) {
    // Store the page in a floating-point pfm image, then load it up into a
    // new 2-D texture.
    PfmFile pfm_page;
    if (!_lm_textures["reflectivity"]->store(pfm_page, i, 0)) {
      lightbuilder_cat.error()
        << "Couldn't store lightmap page " << i << " into a PfmFile\n";
      return false;
    }

    std::ostringstream ss;
    ss << "lm_direct_page_" << i;
    PT(Texture) tex = new Texture(ss.str());
    if (!tex->load(pfm_page)) {
      lightbuilder_cat.error()
        << "Couldn't load lightmap page " << i << " from PfmFile\n";
      return false;
    }
    tex->set_minfilter(SamplerState::FT_linear);
    tex->set_magfilter(SamplerState::FT_linear);
    tex->set_format(Texture::F_rgba32);

    CPT(RenderAttrib) tattr = TextureAttrib::make();
    tattr = DCAST(TextureAttrib, tattr)->add_on_stage(stage, tex);
    page_texture_states.push_back(RenderState::make(tattr));
  }

  _lm_textures["reflectivity"]->clear_image();

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

  np.set_shader_input("vertices", _gpu_buffers["vertices"]);
  np.set_shader_input("triangles", _gpu_buffers["triangles"]);
  np.set_shader_input("lights", _gpu_buffers["lights"]);
  np.set_shader_input("triangle_cells", _gpu_buffers["triangle_cells"]);
  np.set_shader_input("grid", _gpu_buffers["grid"]);

  // Probe positions.
  np.set_shader_input("probes", _gpu_buffers["probes"]);
  // Probe output data.
  np.set_shader_input("probe_output", _lm_textures["probes"]);

  // Use denoised+dialated indirect+direct lightmap.
  np.set_shader_input("luxel_light", _lm_textures["direct"]);
  // Also include the direct light from non-baked sources.
  np.set_shader_input("luxel_light_dynamic", _lm_textures["direct_dynamic"]);
  np.set_shader_input("luxel_albedo", _lm_textures["albedo"]);

  np.set_shader_input("u_bias_", LVecBase2(_bias, 0));
  np.set_shader_input("u_grid_size_probe_count", LVecBase2i(_grid_size, _probes.size()));
  np.set_shader_input("u_to_cell_offset", _scene_mins);
  np.set_shader_input("u_sky_color", _sky_color.get_xyz());

  LVector3 size = _scene_maxs - _scene_mins;
  np.set_shader_input("u_to_cell_size", LVecBase3((1.0f / size[0]) * (float)_grid_size,
                                                  (1.0f / size[1]) * (float)_grid_size,
                                                  (1.0f / size[2]) * (float)_grid_size));

  int ray_count = 2048;
  int max_rays = 64;
  int ray_iters = (ray_count - 1) / max_rays + 1;

  np.set_shader_input("u_ray_params", LVecBase3i(0, ray_count, ray_count));

  //for (int i = 0; i < ray_iters; i++) {
  //  int ray_from = i * max_rays;
  //  int ray_to = std::min((i + 1) * max_rays, ray_count);
  //  np.set_shader_input("u_ray_params", LVecBase3i(ray_from, ray_to, ray_count));

    LVecBase3i group_size((_probes.size() - 1) / 64 + 1, 1, 1);
    _gsg->set_state_and_transform(np.get_state(), TransformState::make_identity());
    _gsg->dispatch_compute(group_size[0], group_size[1], group_size[2]);
    _gsg->finish();
  //}

  // Retrieve probe data back onto CPU.
  _graphics_engine->extract_texture_data(_lm_textures["probes"], _gsg);
  CPTA_uchar probe_data = _lm_textures["probes"]->get_ram_image();
  const float *probe_datap = (const float *)probe_data.p();

  // Now output the data to a friendly format.
  for (size_t i = 0; i < _probes.size(); i++) {
    LightmapAmbientProbe &probe = _probes[i];
    for (int j = 0; j < 9; j++) {
      probe.data[j][0] = probe_datap[i * 36 + j * 4];
      probe.data[j][1] = probe_datap[i * 36 + j * 4 + 1];
      probe.data[j][2] = probe_datap[i * 36 + j * 4 + 2];
    }
  }

  _lm_textures["probes"]->clear_image();
  _lm_textures["albedo"]->clear_image();
  //_lm_textures["reflectivity"]->clear_image();

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

  // A bug in ShaderModuleSpirV is messing up ray tracing, so force
  // the shaders to compile to GLSL.
  load_prc_file_data("lightmap", "gl-support-spirv 0");
  load_prc_file_data("lightmap", "gl-coordinate-system default");
  load_prc_file_data("lightmap", "gl-enable-memory-barriers 1");

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

  if (!make_textures()) {
    lightbuilder_cat.error()
      << "Failed to create lightmap textures\n";
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

  //_graphics_engine->extract_texture_data(_lm_textures["albedo"], _gsg);
  //_lm_textures["albedo"]->write("lm_albedo_#.png", 0, 0, true, false);

  //_graphics_engine->extract_texture_data(_lm_textures["position"], _gsg);
  //_lm_textures["position"]->write("lm_position_#.png", 0, 0, true, false);

  //_graphics_engine->extract_texture_data(_lm_textures["normal"], _gsg);
  //_lm_textures["normal"]->write("lm_normal_#.png", 0, 0, true, false);

  //if (!compute_unocclude()) {
  //  lightbuilder_cat.info()
  //    << "Failed to compute luxel unocclusion\n";
  //  return false;
  //}

  //_graphics_engine->extract_texture_data(_lm_textures["position"], _gsg);
  //_lm_textures["position"]->write("lm_position_#.png", 0, 0, true, false);

  if (!compute_direct()) {
    lightbuilder_cat.error()
      << "Failed to compute luxel direct lighting\n";
    return false;
  }

  if (!compute_indirect()) {
    lightbuilder_cat.error()
      << "Failed to compute luxel indirect lighting\n";
    return false;
  }

  if (!compute_probes()) {
    lightbuilder_cat.error()
      << "Failed to compute ambient probes\n";
  }

  if (!denoise_lightmaps()) {
    lightbuilder_cat.error()
      << "Failed to denoise lightmaps\n";
    return false;
  }

  if (!dialate_lightmaps()) {
    lightbuilder_cat.error()
      << "Failed to dialate lightmaps\n";
    return false;
  }

  if (!write_geoms()) {
    lightbuilder_cat.error()
      << "Failed to write lightmaps to input Geoms\n";
    return false;
  }

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
