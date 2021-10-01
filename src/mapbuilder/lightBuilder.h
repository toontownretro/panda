/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightBuilder.h
 * @author lachbr
 * @date 2021-09-18
 */

#ifndef LIGHTBUILDER_H
#define LIGHTBUILDER_H

#include "pandabase.h"
#include "geomNode.h"
#include "geom.h"
#include "texture.h"
#include "pnmImage.h"
#include "light.h"
#include "graphicsStateGuardianBase.h"
#include "graphicsPipe.h"
#include "pointerTo.h"
#include "pmap.h"
#include "imagePacker.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(lightbuilder, EXPCL_PANDA_MAPBUILDER, EXPTP_PANDA_MAPBUILDER);

class GraphicsEngine;
class GraphicsOutput;

/**
 * GPU-accelerated lightmap solver.
 *
 * Requirements:
 * - Fast.  Use OpenGL and GLSL shaders to accelerate process.  CPU just does set up
 *   work and render calls.
 * - Direct lighting, with shadows, from any light type.
 * - Texture alpha shadows.  Albedo texture with alpha (such as a chainlink fence) should
 *   cast proper shadows.
 * - Indirect lighting, potentially in a separate lightmap (for doing dynamic direct lighting).
 *   Reflectivity of a surface should be determined from albedo color.
 * - Should be able to work on any geometry.  Should be able to throw any scene graph/geom node
 *   at the solver and should be able to compute lightmap coords and lightmaps without any
 *   special geometry set up requirements.  However, if the geometry comes with lightmap UVs
 *   it should utilize those.
 * - Compute light/ambient probes for lighting dynamic models at run-time.
 */
class EXPCL_PANDA_MAPBUILDER LightBuilder {
PUBLISHED:
  enum LightType {
    LT_directional,
    LT_point,
    LT_spot,
  };

  LightBuilder();

  void add_subgraph(NodePath root, const LVecBase2i &lightmap_size);
  void add_geom_node(GeomNode *geom_node, const LVecBase2i &lightmap_size);
  void add_geom(const Geom *geom, const RenderState *state, const TransformState *transform,
                const LVecBase2i &lightmap_size, GeomNode *geom_node = nullptr, int geom_index = -1,
                uint32_t contents = 0);

  //void add_light(NodePath light);

  bool solve();

  INLINE void set_grid_size(int grid_size);
  INLINE int get_grid_size() const;

  INLINE void set_ray_bias(PN_stdfloat bias);
  INLINE PN_stdfloat get_ray_bias() const;

  INLINE void set_num_bounces(int count);
  INLINE int get_num_bounces() const;

  INLINE void set_num_rays_per_luxel(int count);
  INLINE int get_num_rays_per_luxel() const;

  INLINE void set_ray_region_size(int size);
  INLINE int get_ray_region_size() const;

  INLINE void set_num_rays_per_region(int count);
  INLINE int get_num_rays_per_region() const;

  INLINE void set_sky_color(const LColor &color);
  INLINE LColor get_sky_color() const;

  static const InternalName *get_lightmap_uv_name();

private:
  bool initialize_pipe();
  bool make_palette();
  bool make_textures();
  bool offset_geom_lightmap_uvs();
  bool collect_vertices_and_triangles();
  bool make_gpu_buffers();
  bool rasterize_geoms_into_lightmap_textures();
  bool compute_unocclude();
  bool compute_direct();
  bool compute_indirect();
  bool denoise_lightmaps();
  bool dialate_lightmaps();
  bool write_geoms();

public:
  /**
   * A single vertex in the scene.  Comes from a geom.
   */
  struct LightmapVertex {
    // All in world coordinates.
    LPoint3 pos;
    LVector3 normal;
    LVecBase2 uv;
  };
  typedef pvector<LightmapVertex> Vertices;
  Vertices _vertices;

  /**
   * A single triangle in the scene.  Comes from a geom.
   */
  struct LightmapTri {
    // Indices into LightmapVertex array.
    int indices[3];

    // Index of lightmap palette that contains the triangle.
    int palette;

    // AABB of triangle.
    LPoint3 mins;
    LPoint3 maxs;

    // This bitmask is used to determine what kind of triangle it
    // represents.  Used to identify sky triangles.
    uint32_t contents = 0;
  };
  typedef pvector<LightmapTri> Triangles;
  Triangles _triangles;

  struct TriSort {
    int cell_index;
    int tri_index;
  };
  typedef pvector<TriSort> TriSorts;

  /**
   * A geom in the scene that should have a lightmap computed for it.
   * Requires the vertex data to contain a texcoord column named "lightmap".
   * There should be NO overlapping polygons in the lightmap UV set!!!
   */
  struct LightmapGeom {
    // The GeomNode that the Geom came from...
    PT(GeomNode) source_geom_node;
    // and the Geom's index into the GeomNode.
    int geom_index = -1;

    PT(Geom) geom;
    CPT(GeomVertexData) vdata;
    CPT(RenderState) state;
    CPT(TransformState) net_transform;

    // The lightmap size of this individual geom.
    // Will the size of the lightmap for this geom in the palette.
    LVecBase2i lightmap_size;

    // Texel offset into the lightmap palette of where the lightmap
    // for this geom begins.
    LVecBase2i palette_offset;
    // The index of the lightmap palette that contains the lightmap
    // for this geom.
    int palette = -1;

    // Base index into triangle buffer for this geom's set of triangles.
    int first_triangle = -1;
    int num_triangles;

    // Non-indexed copy of the geom.  Original indexed geom is preserved.
    PT(Geom) ni_geom;
    CPT(GeomVertexData) ni_vdata;

    // See LightmapTri for explanation of this.
    uint32_t contents;
  };
  typedef pvector<LightmapGeom> LightmapGeoms;
  LightmapGeoms _geoms;

  /**
   * Represents a single lightmap palette.
   */
  struct LightmapPage {
    // Indices of LightmapGeoms that are contained within this
    // palette.
    vector_int geoms;

    // The object used to pack the individual geom lightmaps into
    // the palette.
    ImagePacker packer;

    // The page's palette index.
    int index = 0;
  };
  typedef pvector<LightmapPage> Pages;
  Pages _pages;

  /**
   * A single light in the scene.
   */
  struct LightmapLight {
    LightBuilder::LightType type;

    LColor color;

    LPoint3 pos;

    // Direction of the light expressed in Euler angles.  Means nothing for
    // point lights.
    LVecBase3 hpr;

    // Attenuation parameters.
    PN_stdfloat constant;
    PN_stdfloat linear;
    PN_stdfloat quadratic;

    // Spotlight-specific parameters.
    PN_stdfloat inner_cone; // in degrees
    PN_stdfloat outer_cone; // in degrees
    PN_stdfloat exponent;

    // If false, only bakes indirect lighting for the light, and direct
    // lighting should be done dynamically at runtime.  Indirect is always
    // baked.  If you need a fully dynamic light, don't add it to the
    // lightmapper.
    bool bake_direct = true;
  };
  typedef pvector<LightmapLight> Lights;
  Lights _lights;

  // The width and height of the lightmap palette.
  LVecBase2i _lightmap_size;

  // This is the set of textures that will be written to during the
  // lightmapping process.  Each is a 2D array texture with a width and
  // height of the lightmap palette, and a depth of the number of
  // lightmap palettes.
  typedef pmap<std::string, PT(Texture)> Textures;
  Textures _lm_textures;

  // And this is the set of buffer textures that contain the vertices,
  // triangles, and lights in the scene.
  Textures _gpu_buffers;

  LPoint3 _scene_mins, _scene_maxs;

  // The number of cells along one dimension of the triangle grid.
  // FIXME: replace with a K-D tree.
  int _grid_size;

  // Bias in world coordinates for ray casting.
  PN_stdfloat _bias;

  // Color of the sky.  Could potentially use an environment map for this in
  // the future, or an option to use one or the other.
  LColor _sky_color;

  // Light bouncing parameters:
  int _bounces; // Number of bounce passes.
  int _rays_per_luxel; // Total number of rays to cast per luxel.
  int _ray_region_size; // Maximum size along one dimension of a region of the
                        // palette to cast rays for.
  int _rays_per_region; // Maximum number of rays to cast in each region.

  //typedef pvector<

  // The GSG we are using to issue render calls.
  PT(GraphicsPipe) _graphics_pipe;
  GraphicsEngine *_graphics_engine;
  // We need a glx/wglGraphicsBuffer to
  GraphicsOutput *_host_output;
  GraphicsStateGuardian *_gsg;

private:
  static CPT(InternalName) _lightmap_uv_name;

private:
  void add_tri_to_grid(int size, const LVecBase3i &offset, const LPoint3 &mins,
                       const LPoint3 &maxs, LPoint3 points[3],
                       int tri_index, TriSorts &triangles);
};

#include "lightBuilder.I"

#endif // LIGHTBUILDER_H
