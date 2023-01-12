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
#include "pvector.h"

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
  void add_vertex_geom(const Geom *geom, const RenderState *state, const TransformState *transform,
                       int model_index = -1, int geom_index = -1, uint32_t contents = 0);

  //void add_light(NodePath light);

  bool solve();

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

  INLINE void set_sun_angular_extent(PN_stdfloat angle);
  INLINE PN_stdfloat get_sun_angular_extent() const;

  static const InternalName *get_lightmap_uv_name();

private:
  bool initialize_pipe();
  bool make_palette();
  bool make_textures();
  bool offset_geom_lightmap_uvs();
  bool collect_vertices_and_triangles();
  bool build_kd_tree();
  bool make_gpu_buffers();
  bool rasterize_geoms_into_lightmap_textures();
  bool rasterize_vertex_lit_geoms();
  bool compute_unocclude();
  bool compute_direct();
  bool compute_vtx_reflectivity();
  bool compute_indirect();
  bool denoise_lightmaps();
  bool dialate_lightmaps();
  bool write_geoms();
  bool compute_probes();

  void free_texture(Texture *tex);

public:
  enum Contents {
    C_none = 0,
    C_sky = 0b1,
    C_transparent = 0b10,

    // Object shouldn't block rays for direct lighting, meaning it won't
    // cast direct light shadows.
    C_dont_block_light = 0b100,
    // Object shouldn't block rays for indirect lighting, meaning it won't
    // reflect light.
    C_dont_reflect_light = 0b1000,
  };

  /**
   * A single vertex in the scene.  Comes from a geom.
   */
  struct LightmapVertex {
    // All in world coordinates.
    LPoint3 pos;
    LVector3 normal;
    LVecBase2 uv;
    // Index into the source Geom's vertex data.
    int orig_vertex;

    pset<int> orig_vertices;
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
    // -1 is no lighting (occluder only), -2 is per-vertex lighting.
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
  int _first_vertex_lit_vertex;
  int _num_vertex_lit_vertices;
  int _first_vertex_lit_tri;
  int _num_vertex_lit_tris;
  int _first_vertex_lit_geom;
  int _num_vertex_lit_geoms;
  int _vertex_palette_width;
  int _vertex_palette_height;

  struct OccluderTri {
    LPoint3 a, b, c;
    uint32_t contents = 0;
  };
  typedef pvector<OccluderTri> OccluderTris;
  OccluderTris _occluder_tris;

  /**
   * A geom in the scene that should have a lightmap computed for it.
   * Requires the vertex data to contain a texcoord column named "lightmap".
   * There should be NO overlapping polygons in the lightmap UV set!!!
   */
  struct LightmapGeom {

    enum LightMode {
      LM_lightmap,
      LM_per_vertex,
    };

    LightMode light_mode;

    // The GeomNode that the Geom came from...
    PT(GeomNode) source_geom_node;
    // and the Geom's index into the GeomNode.
    int geom_index = -1;
    int model_index = -1;

    // If true, this indicates that the Geom comes from an instanced model in
    // the scene, such as a static prop.  The lightmap and UV offsets for the
    // Geom should not be applied to the source geom, but be stored in an
    // index that can be looked up and applied to the model when it's loaded.
    // This way, the prop can have different lightmaps applied to it when used
    // in different levels or multiple times in the same level.
    bool proxy;

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

    LVecBase2 uv_mins, uv_maxs;

    // Base index into triangle buffer for this geom's set of triangles.
    int first_triangle = -1;
    int num_triangles;

    int first_vertex = -1;
    int num_vertices;

    // Non-indexed copy of the geom.  Original indexed geom is preserved.
    PT(Geom) ni_geom;
    CPT(GeomVertexData) ni_vdata;

    // See LightmapTri for explanation of this.
    uint32_t contents;

    PT(GeomVertexArrayData) vertex_light_array;
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
    LVecBase2i dim;

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

  /**
   * A single ambient lighting probe.  Uses spherical harmonics.
   */
  struct LightmapAmbientProbe {
    LPoint3 pos;

    // Output spherical harmonics ambient lighting after computation.
    LVecBase3 data[9];
  };
  typedef pvector<LightmapAmbientProbe> AmbientProbes;
  AmbientProbes _probes;

  struct KDSplit {
    enum Split {
      S_min,
      S_max,
      S_COUNT,
    };
    enum Axis {
      A_x,
      A_y,
      A_z,
      A_COUNT,
    };
    int triangle;
    float dist;
    unsigned char axis;
    Split split;

    bool operator < (const KDSplit &other) const {
      if (dist != other.dist) {
        return dist < other.dist;
      }
      return split < other.split;
    }
  };
  typedef pvector<KDSplit> KDSplits;

  class KDNode : public MemoryBase {
  public:
    enum Face {
      F_left = 1,
      F_right = 0,
      F_back = 3,
      F_front = 2,
      F_bottom = 5,
      F_top = 4,
    };

    static constexpr int nil = -1;

    KDNode() :
      mins(0.0f),
      maxs(0.0f),
      axis(0),
      dist(0.0f),
      first_triangle(0),
      num_triangles(0)
    {
      children[0] = nullptr;
      children[1] = nullptr;
      next = nullptr;
      id = -1;
      index = 0;
      for (int i = 0; i < 6; ++i) {
        neighbors[i] = nullptr;
      }
    }

    // Child nodes, in front and behind splitting plane respectively.
    KDNode *children[2];

    // Bounding box.
    LPoint3 mins, maxs;

    // Defines the splitting plane.
    unsigned char axis;
    float dist;

    // KDNode indices neighboring each face of the node box volume.
    KDNode *neighbors[6];

    // List of LightmapTri indices at leaf.
    vector_int triangles;
    int first_triangle, num_triangles;

    // Linked list.
    KDNode *next;
    int id;

    int index;

    INLINE int get_child_node_index(int child) const {
      if (children[child] != nullptr) {
        return children[child]->index;
      }
      return 0;
    }

    INLINE int get_neighbor_node_index(int neighbor) const {
      if (neighbors[neighbor] != nullptr) {
        return neighbors[neighbor]->index;
      }
      return 0;
    }

    INLINE bool is_leaf() const { return children[0] == nullptr && children[1] == nullptr; }
  };
  int _kd_node_count;
  KDNode *_kd_tree_head;
  KDNode *_kd_tree_tail;

  // Each leaf has a section in this list indentifying the triangles within the
  // leaf.
  pvector<unsigned int> _kd_tri_list;

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

  // Bias in world coordinates for ray casting.
  PN_stdfloat _bias;

  // Color of the sky.  Could potentially use an environment map for this in
  // the future, or an option to use one or the other.
  LColor _sky_color;
  PN_stdfloat _sun_angular_extent;

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
  void make_geom_vertices_and_triangles(LightmapGeom &lgeom, int &triangle, int &vertex);

  KDNode *alloc_kd_node();
  void free_kd_tree();
  KDNode *r_build_kd_tree(const KDSplits *splits, int num_tris, const LPoint3 &mins, const LPoint3 &maxs, int depth, float parent_sah, vector_int &tmp);
  float split_triangles(const KDSplits *splits, int num_tris, unsigned char &axis, int &split,
                        const LPoint3 &mins, const LPoint3 &maxs);
  void r_kd_node_output(const KDNode *node, int indent_level = 0);
  void r_build_kd_ropes(KDNode *node, KDNode *rope[6]);
  void make_kd_leaf(KDNode *node, const KDSplits *splits, int num_tris);
  void optimize_ropes(KDNode *rope[6], const LPoint3 &mins, const LPoint3 &maxs);
  void add_kd_node(KDNode *node);

  void convert_rgba32_to_rgb16(const unsigned char *image, size_t image_size,
    const LVecBase2i &orig_size, const LVecBase2i &new_size, unsigned char *out);
  void convert_rgba32_to_rgb8(const unsigned char *image, size_t image_size,
    const LVecBase2i &orig_size, const LVecBase2i &new_size, unsigned char *out);
  bool compress_rgb16_to_bc6h(Texture *tex);

  void apply_kd_uniforms(NodePath np, int override);
};

#include "lightBuilder.I"

#endif // LIGHTBUILDER_H
