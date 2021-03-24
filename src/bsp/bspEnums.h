/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspEnums.h
 * @author lachbr
 * @date 2020-12-31
 */

#ifndef BSPENUMS_H
#define BSPENUMS_H

#include "config_bsp.h"

// level feature flags
#define LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_NONHDR 0x00000001	// was processed by vrad with -staticproplighting, no hdr data
#define LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_HDR    0x00000002   // was processed by vrad with -staticproplighting, in hdr
#define LVLFLAGS_LIGHTMAP_ALPHA                    0x00000004   // indicates that lightmap alpha data is interleved in the lighting lump
#define LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_3      0x00000008   // was processed by vrad with -staticproplighting3
#define LVLFLAGS_LIGHTMAP_ALPHA_3                  0x00000010   // indicates that 3 sets of lightmap alpha data are interleved in the lighting lump
#define LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_3_NO_SUN  0x00000020// indicates that vertexlitgeneric static prop lighting does not contain direct sunlight in lighting vertex stream
#define LVLFLAGS_LIGHTSTYLES_WITH_CSM			   0x00000040   // indicates that lightstyles now compatible with CSMs

#define	MAXLIGHTMAPS	4

// NOTE: Only 7-bits stored on disk!!!
#define LEAF_FLAGS_SKY			0x01		// This leaf has 3D sky in its PVS
#define LEAF_FLAGS_RADIAL		0x02		// This leaf culled away some portals due to radial vis
#define LEAF_FLAGS_SKY2D		0x04		// This leaf has 2D sky in its PVS
#define LEAF_FLAGS_CONTAINS_DETAILOBJECTS 0x08				// this leaf has at least one detail object in it (set by loader).

// upper design bounds
#define MIN_MAP_DISP_POWER		2	// Minimum and maximum power a displacement can be.
#define MAX_MAP_DISP_POWER		4

// Max # of neighboring displacement touching a displacement's corner.
#define MAX_DISP_CORNER_NEIGHBORS	4

#define NUM_DISP_POWER_VERTS(power)	( ((1 << (power)) + 1) * ((1 << (power)) + 1) )
#define NUM_DISP_POWER_TRIS(power)	( (1 << (power)) * (1 << (power)) * 2 )

#define MAX_MAP_DISP_VERTS				( MAX_MAP_DISPINFO * ((1<<MAX_MAP_DISP_POWER)+1) * ((1<<MAX_MAP_DISP_POWER)+1) )
#define MAX_MAP_DISP_TRIS				( (1 << MAX_MAP_DISP_POWER) * (1 << MAX_MAP_DISP_POWER) * 2 )
#define MAX_DISPVERTS					NUM_DISP_POWER_VERTS( MAX_MAP_DISP_POWER )
#define MAX_DISPTRIS					NUM_DISP_POWER_TRIS( MAX_MAP_DISP_POWER )

#define DISPTRI_TAG_SURFACE			(1<<0)
#define DISPTRI_TAG_WALKABLE		(1<<1)
#define DISPTRI_TAG_BUILDABLE		(1<<2)
#define DISPTRI_FLAG_SURFPROP1		(1<<3)
#define DISPTRI_FLAG_SURFPROP2		(1<<4)
#define DISPTRI_FLAG_SURFPROP3		(1<<5)
#define DISPTRI_FLAG_SURFPROP4		(1<<6)

#define MAX_DISP_MULTIBLEND_CHANNELS		4

#define DISP_MULTIBLEND_PROP_THRESHOLD	2.0f

#define DISP_INFO_FLAG_HAS_MULTIBLEND	0x40000000
#define DISP_INFO_FLAG_MAGIC			0x80000000

// Flags for dworldlight_t::flags
#define DWL_FLAGS_INAMBIENTCUBE		0x0001	// This says that the light was put into the per-leaf ambient cubes.
#define DWL_FLAGS_CASTENTITYSHADOWS	0x0002	// This says that the light will cast shadows from entities

#define OVERLAY_BSP_FACE_COUNT	64
#define OVERLAY_NUM_RENDER_ORDERS		(1<<OVERLAY_RENDER_ORDER_NUM_BITS)
#define OVERLAY_RENDER_ORDER_NUM_BITS	2
#define OVERLAY_RENDER_ORDER_MASK		0xC000	// top 2 bits set

#define WATEROVERLAY_BSP_FACE_COUNT				256
#define WATEROVERLAY_RENDER_ORDER_NUM_BITS		2
#define WATEROVERLAY_NUM_RENDER_ORDERS			(1<<WATEROVERLAY_RENDER_ORDER_NUM_BITS)
#define WATEROVERLAY_RENDER_ORDER_MASK			0xC000	// top 2 bits set

#define	DVIS_PVS	0
#define	DVIS_PAS	1

#define	ANGLE_UP	-1
#define	ANGLE_DOWN	-2

// This is expected to be a four-CC code ('lump')
typedef int GameLumpId;

class EXPCL_PANDA_BSP BSPEnums {
PUBLISHED:
  enum Lump {
    LUMP_ENTITIES					= 0,	// *
    LUMP_PLANES						= 1,	// *
    LUMP_TEXDATA					= 2,	// *
    LUMP_VERTEXES					= 3,	// *
    LUMP_VISIBILITY					= 4,	// *
    LUMP_NODES						= 5,	// *
    LUMP_TEXINFO					= 6,	// *
    LUMP_FACES						= 7,	// *
    LUMP_LIGHTING					= 8,	// *
    LUMP_OCCLUSION					= 9,
    LUMP_LEAFS						= 10,	// *
    LUMP_FACEIDS					= 11,
    LUMP_EDGES						= 12,	// *
    LUMP_SURFEDGES					= 13,	// *
    LUMP_MODELS						= 14,	// *
    LUMP_WORLDLIGHTS				= 15,	//
    LUMP_LEAFFACES					= 16,	// *
    LUMP_LEAFBRUSHES				= 17,	// *
    LUMP_BRUSHES					= 18,	// *
    LUMP_BRUSHSIDES					= 19,	// *
    LUMP_AREAS						= 20,	// *
    LUMP_AREAPORTALS				= 21,	// *
    LUMP_FACEBRUSHES				= 22,	// * Index of the brush each face came from
    LUMP_FACEBRUSHLIST				= 23,	// *
    LUMP_UNUSED1					= 24,	// *
    LUMP_UNUSED2					= 25,	// *

    LUMP_DISPINFO					= 26,
    LUMP_ORIGINALFACES				= 27,
    LUMP_PHYSDISP					= 28,
    LUMP_PHYSCOLLIDE				= 29,
    LUMP_VERTNORMALS				= 30,
    LUMP_VERTNORMALINDICES			= 31,
    LUMP_DISP_LIGHTMAP_ALPHAS		= 32,
    LUMP_DISP_VERTS					= 33,		// CDispVerts
    LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS = 34,	// For each displacement
                          //     For each lightmap sample
                          //         byte for index
                          //         if 255, then index = next byte + 255
                          //         3 bytes for barycentric coordinates
    // The game lump is a method of adding game-specific lumps
    // FIXME: Eventually, all lumps could use the game lump system
    LUMP_GAME_LUMP					= 35,
    LUMP_LEAFWATERDATA				= 36,
    LUMP_PRIMITIVES					= 37,
    LUMP_PRIMVERTS					= 38,
    LUMP_PRIMINDICES				= 39,
    // A pak file can be embedded in a .bsp now, and the file system will search the pak
    //  file first for any referenced names, before deferring to the game directory
    //  file system/pak files and finally the base directory file system/pak files.
    LUMP_PAKFILE					= 40,
    LUMP_CLIPPORTALVERTS			= 41,
    // A map can have a number of cubemap entities in it which cause cubemap renders
    // to be taken after running vrad.
    LUMP_CUBEMAPS					= 42,
    LUMP_TEXDATA_STRING_DATA		= 43,
    LUMP_TEXDATA_STRING_TABLE		= 44,
    LUMP_OVERLAYS					= 45,
    LUMP_LEAFMINDISTTOWATER			= 46,
    LUMP_FACE_MACRO_TEXTURE_INFO	= 47,
    LUMP_DISP_TRIS					= 48,
    LUMP_PROP_BLOB					= 49,	// static prop triangle & string data
    LUMP_WATEROVERLAYS              = 50,
    LUMP_LEAF_AMBIENT_INDEX_HDR		= 51,	// index of LUMP_LEAF_AMBIENT_LIGHTING_HDR
    LUMP_LEAF_AMBIENT_INDEX         = 52,	// index of LUMP_LEAF_AMBIENT_LIGHTING

    // optional lumps for HDR
    LUMP_LIGHTING_HDR				= 53,
    LUMP_WORLDLIGHTS_HDR			= 54,
    LUMP_LEAF_AMBIENT_LIGHTING_HDR	= 55,	// NOTE: this data overrides part of the data stored in LUMP_LEAFS.
    LUMP_LEAF_AMBIENT_LIGHTING		= 56,	// NOTE: this data overrides part of the data stored in LUMP_LEAFS.

    LUMP_XZIPPAKFILE				= 57,   // deprecated. xbox 1: xzip version of pak file
    LUMP_FACES_HDR					= 58,	// HDR maps may have different face data.
    LUMP_MAP_FLAGS                  = 59,   // extended level-wide flags. not present in all levels
    LUMP_OVERLAY_FADES				= 60,	// Fade distances for overlays
    LUMP_OVERLAY_SYSTEM_LEVELS		= 61,	// System level settings (min/max CPU & GPU to render this overlay)
    LUMP_PHYSLEVEL                  = 62,
    LUMP_DISP_MULTIBLEND			= 63,	// Displacement multiblend info

    HEADER_LUMPS = 64,
  };

  // Lumps that have versions are listed here
  enum {
    LUMP_LIGHTING_VERSION          = 1,
    LUMP_FACES_VERSION             = 1,
    LUMP_OCCLUSION_VERSION         = 2,
    LUMP_LEAFS_VERSION			   = 1,
    LUMP_LEAF_AMBIENT_LIGHTING_VERSION = 1,
    LUMP_WORLDLIGHTS_VERSION	   = 1
  };

  // ------------------------------------------------------------------------------------------------ //
  // Displacement neighbor rules
  // ------------------------------------------------------------------------------------------------ //
  //
  // Each displacement is considered to be in its own space:
  //
  //               NEIGHBOREDGE_TOP
  //
  //                   1 --- 2
  //                   |     |
  // NEIGHBOREDGE_LEFT |     | NEIGHBOREDGE_RIGHT
  //                   |     |
  //                   0 --- 3
  //
  //   			NEIGHBOREDGE_BOTTOM
  //
  //
  // Edge edge of a displacement can have up to two neighbors. If it only has one neighbor
  // and the neighbor fills the edge, then SubNeighbor 0 uses CORNER_TO_CORNER (and SubNeighbor 1
  // is undefined).
  //
  // CORNER_TO_MIDPOINT means that it spans [bottom edge,midpoint] or [left edge,midpoint] depending
  // on which edge you're on.
  //
  // MIDPOINT_TO_CORNER means that it spans [midpoint,top edge] or [midpoint,right edge] depending
  // on which edge you're on.
  //
  // Here's an illustration (where C2M=CORNER_TO_MIDPOINT and M2C=MIDPOINT_TO_CORNER
  //
  //
  //				 C2M			  M2C
  //
  //       1 --------------> x --------------> 2
  //
  //       ^                                   ^
  //       |                                   |
  //       |                                   |
  //  M2C  |                                   |	M2C
  //       |                                   |
  //       |                                   |
  //
  //       x                 x                 x
  //
  //       ^                                   ^
  //       |                                   |
  //       |                                   |
  //  C2M  |                                   |	C2M
  //       |                                   |
  //       |                                   |
  //
  //       0 --------------> x --------------> 3
  //
  //               C2M			  M2C
  //
  //
  // The CHILDNODE_ defines can be used to refer to a node's child nodes (this is for when you're
  // recursing into the node tree inside a displacement):
  //
  // ---------
  // |   |   |
  // | 1 | 0 |
  // |   |   |
  // |---x---|
  // |   |   |
  // | 2 | 3 |
  // |   |   |
  // ---------
  //
  // ------------------------------------------------------------------------------------------------ //

  // These can be used to index g_ChildNodeIndexMul.
  enum {
    CHILDNODE_UPPER_RIGHT=0,
    CHILDNODE_UPPER_LEFT=1,
    CHILDNODE_LOWER_LEFT=2,
    CHILDNODE_LOWER_RIGHT=3
  };

  // Corner indices. Used to index m_CornerNeighbors.
  enum {
    CORNER_LOWER_LEFT=0,
    CORNER_UPPER_LEFT=1,
    CORNER_UPPER_RIGHT=2,
    CORNER_LOWER_RIGHT=3
  };

  // These edge indices must match the edge indices of the CCoreDispSurface.
  enum {
    NEIGHBOREDGE_LEFT=0,
    NEIGHBOREDGE_TOP=1,
    NEIGHBOREDGE_RIGHT=2,
    NEIGHBOREDGE_BOTTOM=3
  };

  // These denote where one dispinfo fits on another.
  // Note: tables are generated based on these indices so make sure to update
  //       them if these indices are changed.
  typedef enum {
    CORNER_TO_CORNER=0,
    CORNER_TO_MIDPOINT=1,
    MIDPOINT_TO_CORNER=2
  } NeighborSpan;

  // These define relative orientations of displacement neighbors.
  typedef enum {
    ORIENTATION_CCW_0=0,
    ORIENTATION_CCW_90=1,
    ORIENTATION_CCW_180=2,
    ORIENTATION_CCW_270=3
  } NeighborOrientation;

  enum OccluderFlags {
    OF_inactive = 0x1,
  };

  enum DPrimitiveType {
    PRIM_TRILIST = 0,
    PRIM_TRISTRIP = 1,
  };

  // lights that were used to illuminate the world
  enum EmitType {
    emit_surface,		// 90 degree spotlight
    emit_point,			// simple point light source
    emit_spotlight,		// spotlight with penumbra
    emit_skylight,		// directional light with no falloff (surface must trace to SKY texture)
    emit_quakelight,	// linear falloff, non-lambertian
    emit_skyambient,	// spherical light source with no falloff (surface must trace to SKY texture)
  };
};

#endif // BSPENUMS_H
