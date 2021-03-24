/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspLumps.h
 * @author lachbr
 * @date 2020-12-31
 */

/**
 * This file contains the data structures for each lump in the BSP format.
 *
 * Each lump is a struct that contains the data members and methods for
 * reading/writing to/from a datagram, as well as a method that returns the
 * number of bytes in the structure.
 *
 * We individually read and write each data member of a lump to/from the
 * datagram to support versioned lumps and different byte-orders.
 *
 * We need a method that returns the size because the BSP format does not
 * directly specify how many entries there are for a particular lump; it must
 * be deduced from the size of the lump.  Most lumps just return sizeof(Lump)
 * for the size, but there are special cases, particularly when a lump has
 * multiple versions, to handle backwards compatibility.
 */

#ifndef BSPLUMPS_H
#define BSPLUMPS_H

#include "config_bsp.h"
#include "bspEnums.h"
#include "colorRGBExp32.h"
#include "luse.h"

struct DFlagsLump {
  uint32_t level_flags; // LVLFLAGS_xxx

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DGameLump {
  GameLumpId id;
  unsigned short flags;
  unsigned short version;
  int fileofs;
  int filelen;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DModel {
  LVector3f mins;
  LVector3f maxs;
  // for sounds or lights
  LVector3f origin;
  int head_node;
  // submodels just draw faces without walking the bsp tree
  int first_face;
  int num_faces;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DPhysModel {
  int model_index;
  int data_size;
  int keydata_size;
  int solid_count;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// contains the binary blob for each displacement surface's virtual hull
struct DPhysDisp {
  unsigned short num_displacements;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DVertex {
  LVector3f point;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// planes (x&~1) and (x&~1)+1 are always opposites
struct DPlane {
  LVector3f normal;
  float dist;
  int type; // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate

  INLINE DPlane();

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DNode {
  int plane_num;
  int children[2]; // negative numbers are -(leafs+1), not nodes
  short mins[3]; // for frustom culling
  short maxs[3];
  unsigned short first_face;
  unsigned short num_faces; // counting both sides
  // If all leaves below this node are in the same area, then
  // this is the area index. If not, this is -1.
  short area;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct TexInfo {
  float texture_vecs[2][4]; // [s/t][xyz offset]
  float lightmap_vecs[2][4]; // [s/t][xyz offset]
  int flags; // miptex flags + overrides
  int texdata; // Pointer to texture name, size, etc.

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DTexData {
  LVector3f reflectivity;
  // index into g_StringTable for the texture name
  int name_string_table_id;
  // source image
  int width;
  int height;
  int view_width;
  int view_height;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

/**
 * Occluders are simply polygons.
 */
struct DOccluderData {
  int flags;
  int first_poly; // index into doccluderpolys
  int poly_count;
  LVector3f mins;
  LVector3f maxs;

  // Version 1 only
  int area;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DOccluderPolyData {
  int first_vertex_index;
  int vertex_count;
  int plane_num;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// NOTE: see the section above titled "displacement neighbor rules".
struct DispSubNeighbor {
  INLINE bool is_valid() const { return neighbor != 0xFFFF; }
  INLINE void set_invalid() { neighbor = 0xFFFF; }

  unsigned short neighbor; // This indexes into DispInfos.
                            // 0xFFFF if there is no neighbor here.
  // (CCW) rotation of the neighbor wrt this displacement.
  unsigned char neighbor_orientation;

  // These use the NeighborSpan type.
  unsigned char span; // Where the neighbor fits onto this side of our displacement.
  unsigned char neighbor_span; // Where we fit onto our neighbor.

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DispNeighbor {
  INLINE void set_invalid() {
    sub_neighbors[0].set_invalid();
    sub_neighbors[1].set_invalid();
  }

  // Returns false if there isn't anything touching this edge.
  INLINE bool is_invalid() const {
    return sub_neighbors[0].is_valid() || sub_neighbors[1].is_valid();
  }

  // Note: if there is a neighbor that fills the whole side (CORNER_TO_CORNER),
  //       then it will always be in CDispNeighbor::m_Neighbors[0]
  DispSubNeighbor sub_neighbors[2];

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DispCornerNeighbors {
  void set_invalid() { num_neighbors = 0; }

  unsigned short neighbors[MAX_DISP_CORNER_NEIGHBORS];
  unsigned char num_neighbors;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DispVert {
  LVector3f vector;
  float dist;
  float alpha;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DispTri {
  unsigned short tags;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DispMultiBlend {
  LVector4f multi_blend;
  LVector4f alpha_blend;
  LVector3f multi_blend_colors[MAX_DISP_MULTIBLEND_CHANNELS];

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DDispInfo {
  INLINE int get_num_verts() const { return NUM_DISP_POWER_VERTS(power); }
  INLINE int get_num_tris() const { return NUM_DISP_POWER_TRIS(power); }

  LVector3f start_position;
  int disp_vert_start;
  int disp_tri_start;

  int power;
  int min_tess;
  float smoothing_angle;
  int contents;

  unsigned short map_face;

  int lightmap_alpha_start;
  int lightmap_sample_position_start;

  DispNeighbor edge_neighbors[4];
  DispCornerNeighbors corner_neighbors[4];

  enum unnamed { ALLOWEDVERTS_SIZE = PAD_NUMBER( MAX_DISPVERTS, 32 ) / 32 };
  unsigned int allowed_verts[ALLOWEDVERTS_SIZE];

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
struct DEdge {
  unsigned short v[2]; // vertex numbers

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DPrimitive {
  unsigned char type;
  unsigned short first_index;
  unsigned short index_count;
  unsigned short first_vert;
  unsigned short vert_count;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DPrimVert {
  LVector3f pos;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DFace {
  unsigned short plane_num;
  byte side; // faces opposite to the node's plane direction
  byte on_node; // 1 of on node, 0 if in leaf

  int first_edge; // we must support > 64k edges
  short num_edges;
  short texinfo;

  short dispinfo;
  short surface_fog_volume_id;

  // lighting info
  byte styles[MAXLIGHTMAPS];
  int lightofs; // start of [numstyles*surfsize] samples
  float area;

  int lightmap_mins[2];
  int lightmap_size[2];

  int orig_face; // reference the original face this face was derived from

  INLINE unsigned short get_num_prims() const;
  INLINE void set_num_prims(unsigned short count);
  INLINE bool get_dynamic_shadows_enabled() const;
  INLINE void set_dynamic_shadows_enabled(bool enabled);

  // Non-polygon primitives (strips and lists)
private:
  unsigned short num_prims;

public:
  unsigned short first_prim_id;
  unsigned int smoothing_groups;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DFaceID {
  unsigned short hammer_face_id;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DFaceBrushList {
  // number of brushes that contributed a side to this face
  unsigned short face_brush_count;
  // first brush. NOTE: if m_nFaceBrushCount is 1, this is a brush index!
  unsigned short face_brush_start;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// version 1
struct DLeaf {
  int contents; // OR of all brushes (not needed?)

  short cluster;

  union {
    short area_flags;
    struct {
      short area : 9;
      short flags : 7; // Per leaf flags.
    };
  };

  short mins[3];
  short maxs[3];

  unsigned short first_leaf_face;
  unsigned short num_leaf_faces;

  unsigned short first_leaf_brush;
  unsigned short num_leaf_brushes;

  short leaf_water_data_id;

  // NOTE: removed this for version 1 and moved into separate lump "LUMP_LEAF_AMBIENT_LIGHTING" or "LUMP_LEAF_AMBIENT_LIGHTING_HDR"
  // Precaculated light info for entities.
	CompressedLightCube ambient_lighting;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// each leaf contains N samples of the ambient lighting
// each sample contains a cube of ambient light projected on to each axis
// and a sampling position encoded as a 0.8 fraction (mins=0,maxs=255) of the leaf's bounding box
struct DLeafAmbientLighting {
  CompressedLightCube cube;
  byte x; // fixed point fraction of leaf bounds
	byte y; // fixed point fraction of leaf bounds
	byte z; // fixed point fraction of leaf bounds
	byte pad; // unused

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DLeafAmbientIndex {
  unsigned short ambient_sample_count;
  unsigned short first_ambient_sample;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DBrushSide {
  unsigned short plane_num; // facing out of the leaf
  short texinfo;
  short dispinfo; // displacement info (BSPVERSION 7)
  byte bevel; // is the side a bevel plane? (BSPVERSION 7)
  byte thin; // is a thin side?

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DBrush {
  int first_side;
  int num_sides;
  int contents;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// the visibility lump consists of a header with a count, then
// byte offsets for the PVS and PHS of each cluster, then the raw
// compressed bit vectors
struct DVis {
  int num_clusters;
  int bitofs[8][2];

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

// each area has a list of portals that lead into other areas
// when portals are closed, other areas may not be visible or
// hearable even if the vis info says that it should be
struct DAreaPortal {
  unsigned short portal_key;
  unsigned short other_area; // The area this portal looks into.
  unsigned short first_clip_portal_vert; // Portal geometry.
  unsigned short num_clip_portal_verts;
  int plane_num;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DArea {
  int num_area_portals;
  int first_area_portal;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DLeafWaterData {
  float surface_z;
  float min_z;
  short surface_texinfo_id;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct FaceMacroTextureInfo {
  // This looks up into g_TexDataStringTable, which looks up into g_TexDataStringData.
	// 0xFFFF if the face has no macro texture.
  unsigned short macro_texture_name_id;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DWorldlight {
  LVector3f origin;
  LVector3f intensity;
  LVector3f normal; // for surfaces and spotlights

  // Version 1 only
  LVector3f shadow_cast_offset; // gets added to the light origin when this light is used as a shadow caster (only if DWL_FLAGS_CASTENTITYSHADOWS flag is set)

  int cluster;
  BSPEnums::EmitType type;
  int style;
  float stopdot; // start of penumbra for emit_spotlight
  float stopdot2; // end of penumbra for emit_spotlight
  float exponent;
  float radius; // cutoff distance
  // falloff for emit_spotlight + emit_point:
  // 1 / (constant_attn + linear_attn * dist + quadratic_attn * dist^2)
  float constant_attn;
	float linear_attn;
	float quadratic_attn;
	int flags;			// Uses a combination of the DWL_FLAGS_ defines.
	int texinfo;		//
	int owner;			// entity that this light it relative to

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DCubeMapSample {
  int origin[3]; // position of light snapped to the nearest integer
  // 0 - default
  // otherwise, 1<<(size-1)
  unsigned char size;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DOverlay {
  int id;
  short texinfo;

  INLINE void set_face_count(unsigned short count);
  INLINE unsigned short get_face_count() const;

  INLINE void set_render_order(unsigned short order);
  INLINE unsigned short get_render_order() const;

private:
  unsigned short _face_count_render_order;

public:
  int faces[OVERLAY_BSP_FACE_COUNT];
  float u[2];
  float v[2];
  LVector3f uv_points[4];
  LVector3f origin;
  LVector3f basis_normal;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DOverlayFade {
  float fade_dist_min_sq;
  float fade_dist_max_sq;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DOverlaySystemLevel {
  unsigned char min_cpu_level;
  unsigned char max_cpu_level;
  unsigned char min_gpu_level;
  unsigned char max_gpu_level;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct DWaterOverlay {
  int id;
  short texinfo;

  INLINE void set_face_count(unsigned short count);
  INLINE unsigned short get_face_count() const;

  INLINE void set_render_order(unsigned short order);
  INLINE unsigned short get_render_order() const;

private:
  unsigned short _face_count_render_order;

public:
  int faces[WATEROVERLAY_BSP_FACE_COUNT];
  float u[2];
  float v[2];
  LVector3f uv_points[4];
  LVector3f origin;
  LVector3f basis_normal;

  INLINE static size_t get_size(int version);
  void read_datagram(DatagramIterator &dgi, int version);
  void write_datagram(Datagram &dg, int version) const;
};

struct EPair {
  EPair *next;
  char *key;
  char *value;
};

class Portal;

struct Entity {
  INLINE Entity();

  LVector3f origin;
  int first_brush;
  int num_brushes;
  EPair *epairs;

  // only valid for func_areaportals
  int area_portal_num;
  int portal_areas[2];
  Portal *portals_leading_into_areas[2];
};

#include "bspLumps.I"

#endif // BSPLUMPS_H
