/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspData.h
 * @author brian
 * @date 2020-12-30
 */

#ifndef BSPDATA_H
#define BSPDATA_H

#include "config_bsp.h"
#include "typedReferenceCount.h"
#include "datagramIterator.h"
#include "datagram.h"
#include "bspLumps.h"
#include "bspEnums.h"
#include "lzmaDecoder.h"
#include "zipArchive.h"
#include "pta_ushort.h"
#include "bspClusterVisibility.h"
#include "vector_int.h"

/**
 * The root object in the BSP file.
 */
class EXPCL_PANDA_BSP BSPData : public TypedReferenceCount, public BSPEnums {
PUBLISHED:
  /**
   * A single lump entry.  Simply references the offset into the file of where
   * the lump begins.
   */
  class Lump {
  PUBLISHED:
    int file_offset = 0;
    int file_length = 0;
    int version = 0;
    int uncompressed_size = 0;
  };

  enum Field {
    F_uint8,
    F_uint16,
    F_uint32,
    F_uint64,

    F_int8,
    F_int16,
    F_int32,
    F_int64,

    F_float32,
    F_float64,

    F_vector,
  };

  INLINE BSPData();

  INLINE int get_header() const;
  MAKE_PROPERTY(header, get_header);
  INLINE std::string get_header_string() const;
  MAKE_PROPERTY(header_string, get_header_string);

  INLINE int get_version() const;
  MAKE_PROPERTY(version, get_version);

  INLINE bool is_source() const;
  MAKE_PROPERTY(source, is_source);

  INLINE size_t get_num_lumps() const;
  INLINE const Lump &get_lump(size_t n) const;
  MAKE_SEQ_PROPERTY(lumps, get_num_lumps, get_lump);

  INLINE int get_map_revision() const;
  MAKE_PROPERTY(map_revision, get_map_revision);

  INLINE bool is_valid() const;
  MAKE_PROPERTY(valid, is_valid);

  INLINE int get_lump_version(int lump) const;
  INLINE bool has_lump(int lump) const;

  DVertex *get_face_vertex(DFace *face, int n);
  const ColorRGBExp32 *sample_light_data(const pvector<uint8_t> &lightdata, const DFace *face,
                                   int ofs, int luxel, int style, int bump) const;
  const ColorRGBExp32 *sample_lightmap(const DFace *face, int luxel, int style,
                                 int bump) const;

  std::string get_string(int id) const;
  int add_or_find_string(const std::string &str);

  int get_leaf_containing_point(const LPoint3 &point, int head_node = 0) const;

PUBLISHED:
  bool read_datagram(DatagramIterator &dgi);
  bool read_header(DatagramIterator &dgi);
  void read_lump_entry(DatagramIterator &dgi, int n);
  bool read_lumps(const Datagram &dg);

  void write_datagram(Datagram &dg);

private:
  template<class T>
  INLINE void extract_data(T &out, Field field, DatagramIterator &dgi);

  template <class T>
  INLINE bool copy_lump(int lump, pvector<T> &dest, const Datagram &dg, int force_version = -1);

  template <class T>
  INLINE bool copy_lump(Field field, int lump, pvector<T> &dest, const Datagram &dg);

  bool uncompress_lump(int lump, const Datagram &dg, Datagram &out_dg);

  bool copy_occlusion_lump(const Datagram &dg);

  bool copy_pak_lump(const Datagram &dg);

public:
  // Here's the lump structures.

  pvector<DModel> dmodels;

  pvector<uint8_t> dvisdata;
  DVis *dvis;

  // Decompressed per-cluster visibility data.  One entry for each cluster
  // index.
  pvector<BSPClusterVisibility> cluster_vis;

  pvector<uint8_t> dlightdata_hdr;
  pvector<uint8_t> dlightdata_ldr;
  pvector<uint8_t> *pdlightdata;
  pvector<char> dentdata;

  pvector<DLeaf> dleafs;

  pvector<DLeafAmbientLighting> leafambientlighting_ldr;
  pvector<DLeafAmbientLighting> leafambientlighting_hdr;
  pvector<DLeafAmbientLighting> *pleafambientlighting;
  pvector<DLeafAmbientIndex> leafambientindex_ldr;
  pvector<DLeafAmbientIndex> leafambientindex_hdr;
  pvector<DLeafAmbientIndex> *pleafambientindex;
  pvector<unsigned short> leaf_min_dist_to_water;

  pvector<DPlane> dplanes;

  pvector<DVertex> dvertexes;

  pvector<unsigned short> vertnormalindices;

  pvector<LVector3f> vertnormals;

  pvector<DNode> dnodes;

  pvector<TexInfo> texinfo;

  pvector<DTexData> dtexdata;

  // Displacement map info
  pvector<DDispInfo> dispinfo;
  pvector<DispVert> dispverts;
  pvector<DispTri> disptris;
  // LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS
  pvector<unsigned char> disp_lightmap_sample_positions;

  pvector<DFace> dorigfaces;

  pvector<DPrimitive> primitives;
  pvector<DPrimVert> primverts;
  pvector<unsigned short> primindices;

  pvector<DFace> dfaces;

  pvector<DFaceID> dfaceids;

  pvector<uint16_t> dfacebrushes;
  pvector<DFaceBrushList> dfacebrushlists;

  pvector<DFace> dfaces_hdr;

  pvector<DEdge> dedges;

  pvector<unsigned short> dleaffaces;
  pvector<unsigned short> dleafbrushes;

  vector_int dsurfedges;

  pvector<DArea> dareas;
  pvector<DAreaPortal> dareaportals;

  pvector<DBrush> dbrushes;
  pvector<DBrushSide> dbrushsides;

  pvector<DWorldlight> dworldlights_ldr;
  pvector<DWorldlight> dworldlights_hdr;
  pvector<DWorldlight> *dworldlights;

  pvector<LVector3f> clip_portal_verts;

  pvector<DCubeMapSample> cubemap_samples;

  pvector<DOverlay> overlays;
  pvector<DOverlayFade> overlay_fades;

  pvector<DWaterOverlay> water_overlays;

  pvector<char> tex_data_string_data;
  vector_int tex_data_string_table;

  pvector<DLeafWaterData> dleafwaterdata;

  pvector<FaceMacroTextureInfo> face_macro_texture_infos;

  pvector<DOccluderData> occluder_data;
  pvector<DOccluderPolyData> occluder_poly_data;
  vector_int occluder_vertex_indices;

  uint32_t level_flags;

  pvector<uint8_t> phys_collide;
  pvector<uint8_t> phys_disp;

  PT(ZipArchive) _pak_file;

private:
  int _header;

  int _version;
  bool _is_source;

  typedef pvector<Lump> Lumps;
  Lumps _lumps;

  int _map_revision;

  bool _valid;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BSPData",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "bspData.I"
#include "bspData.T"

#endif // BSPDATA_H
