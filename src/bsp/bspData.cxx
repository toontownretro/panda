/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspData.cxx
 * @author lachbr
 * @date 2020-12-30
 */

#include "bspData.h"
#include "bsp.h"
#include "lzmaDecoder.h"
#include "bspFlags.h"
#include "mathutil_misc.h"
#include "virtualFileSystem.h"

TypeHandle BSPData::_type_handle;

/**
 * Returns the nth vertex of the given face.
 */
DVertex *BSPData::
get_face_vertex(DFace *face, int n) {
  int surf_edge = dsurfedges[face->first_edge + n];
  int point;
  if (surf_edge < 0) {
    point = dedges[-surf_edge].v[1];
  } else {
    point = dedges[surf_edge].v[0];
  }

  return &dvertexes[point];
}

/**
 * Fetches a lightmap sample from the given light data.
 */
const ColorRGBExp32 *BSPData::
sample_light_data(const pvector<uint8_t> &data, const DFace *face, int ofs,
                  int luxel, int style, int bump) const {
  int luxels = (face->lightmap_size[0] + 1) * (face->lightmap_size[1] + 1);
  const TexInfo *tinfo = &texinfo[face->texinfo];
  int bump_count = (tinfo->flags & SURF_BUMPLIGHT) ? (NUM_BUMP_VECTS + 1) : 1;
  return (const ColorRGBExp32 *)&data[ofs + ((style * bump_count + bump) * luxels) + luxel * sizeof(ColorRGBExp32)];
}

/**
 * Fetches a lightmap sample from the current light data.
 */
const ColorRGBExp32 *BSPData::
sample_lightmap(const DFace *face, int luxel, int style, int bump) const {
  return sample_light_data(*pdlightdata, face, face->lightofs, luxel, style, bump);
}

/**
 * Returns the string from the string table with the indicated ID.
 */
std::string BSPData::
get_string(int id) const {
  return std::string(&tex_data_string_data[tex_data_string_table[id]]);
}

/**
 * Returns the index of the given string in the string table.  If the string is
 * not in the table, it is added to the table and the new index is returned.
 */
int BSPData::
add_or_find_string(const std::string &str) {
  size_t i;
  for (i = 0; i < tex_data_string_table.size(); i++) {
    if (get_string(i) == str) {
      return i;
    }
  }

  // Not found; add it to the table.
  size_t out_offset = tex_data_string_data.size();
  tex_data_string_data.insert(tex_data_string_data.end(), str.begin(), str.end());
  size_t out_index = tex_data_string_table.size();
  tex_data_string_table.push_back(out_offset);

  return out_index;
}

/**
 * Reads in the BSP file from the indicated datagram.
 */
bool BSPData::
read_datagram(DatagramIterator &dgi) {
  // First read the header.
  if (!read_header(dgi)) {
    bsp_cat.error()
      << "Failed to read BSP header.\n";
    return false;
  }

  // Now read in the actual guts of each lump.
  if (!read_lumps(dgi.get_datagram())) {
    bsp_cat.error()
      << "Failed to read BSP lumps.\n";
    return false;
  }

  _valid = true;

  return true;
}

/**
 * Reads in the BSP header from the datagram.  The BSP header contains the
 * magic number, file version, and lump entries.
 */
bool BSPData::
read_header(DatagramIterator &dgi) {
  _header = dgi.get_int32();

  if (_header == _source_bsp_header) {
    _is_source = true;

  } else if (_header == _bsp_header) {
    _is_source = false;

  } else {
    bsp_cat.error()
      << "Not a valid PBSP or VBSP file.  Header: " << _header << "\n";
    return false;
  }

  _version = dgi.get_int32();

  if (_is_source) {
    if (_version < _source_bsp_version_min || _version > _source_bsp_version) {
      bsp_cat.error()
        << "This VBSP file is version " << _version << ", but I can only read VBSP versions "
        << _source_bsp_version_min << " through " << _source_bsp_version << ".\n";
      return false;
    }
  } else {
    if (_version < _bsp_version_min || _version > _bsp_version) {
      bsp_cat.error()
        << "This PBSP file is version " << _version << ", but I can only read PBSP versions "
        << _source_bsp_version_min << " through " << _source_bsp_version << ".\n";
      return false;
    }
  }

  // Now read in the lump entries.
  _lumps.resize(BSPEnums::HEADER_LUMPS);
  for (int i = 0; i < BSPEnums::HEADER_LUMPS; i++) {
    read_lump_entry(dgi, i);
  }

  _map_revision = dgi.get_int32();

  return true;
}

/**
 * Reads in a single lump entry from the datagram.
 */
void BSPData::
read_lump_entry(DatagramIterator &dgi, int n) {
  Lump &lump = _lumps[n];

  lump.file_offset = dgi.get_int32();
  lump.file_length = dgi.get_int32();
  lump.version = dgi.get_int32();
  lump.uncompressed_size = dgi.get_int32();

  bsp_cat.debug()
    << "Lump uncompressed size: " << lump.uncompressed_size << "\n";
}

/**
 * Reads in the actual guts of each lump from the datagram.
 *
 * Returns true if all lumps were successfully read in, false otherwise.
 */
bool BSPData::
read_lumps(const Datagram &dg) {
  // LUMP_ENTITIES

  if (!copy_lump(LUMP_PLANES, dplanes, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_TEXDATA, dtexdata, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_VERTEXES, dvertexes, dg)) {
    return false;
  }

  if (!copy_lump(F_uint8, LUMP_VISIBILITY, dvisdata, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_NODES, dnodes, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_TEXINFO, texinfo, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_FACES, dfaces, dg)) {
    return false;
  }

  if (!copy_lump(F_uint8, LUMP_LIGHTING, dlightdata_ldr, dg)) {
    return false;
  }

  if (has_lump(LUMP_OCCLUSION)) {
    if (!copy_occlusion_lump(dg)) {
      return false;
    }
  }


  if (!copy_lump(LUMP_LEAFS, dleafs, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_FACEIDS, dfaceids, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_EDGES, dedges, dg)) {
    return false;
  }

  if (!copy_lump(F_int32, LUMP_SURFEDGES, dsurfedges, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_MODELS, dmodels, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_WORLDLIGHTS, dworldlights_ldr, dg)) {
    return false;
  }

  if (!copy_lump(F_uint16, LUMP_LEAFFACES, dleaffaces, dg)) {
    return false;
  }

  if (!copy_lump(F_uint16, LUMP_LEAFBRUSHES, dleafbrushes, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_BRUSHES, dbrushes, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_BRUSHSIDES, dbrushsides, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_AREAS, dareas, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_AREAPORTALS, dareaportals, dg)) {
    return false;
  }

  if (!copy_lump(F_uint16, LUMP_FACEBRUSHES, dfacebrushes, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_FACEBRUSHLIST, dfacebrushlists, dg)) {
    return false;
  }

  // LUMP_UNUSED1
  // LUMP_UNUSED2

  if (!copy_lump(LUMP_DISPINFO, dispinfo, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_ORIGINALFACES, dorigfaces, dg)) {
    return false;
  }

  if (!copy_lump(F_uint8, LUMP_PHYSDISP, phys_disp, dg)) {
    return false;
  }

  if (!copy_lump(F_uint8, LUMP_PHYSCOLLIDE, phys_collide, dg)) {
    return false;
  }

  if (!copy_lump(F_vector, LUMP_VERTNORMALS, vertnormals, dg)) {
    return false;
  }

  if (!copy_lump(F_uint16, LUMP_VERTNORMALINDICES, vertnormalindices, dg)) {
    return false;
  }

  // LUMP_DISP_LIGHTMAPS_ALPHAS - This appears to be deprecated.

  if (!copy_lump(LUMP_DISP_VERTS, dispverts, dg)) {
    return false;
  }

  // LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS - This appears to be deprecated.

  // LUMP_GAME_LUMP

  if (!copy_lump(LUMP_LEAFWATERDATA, dleafwaterdata, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_PRIMITIVES, primitives, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_PRIMVERTS, primverts, dg)) {
    return false;
  }

  if (!copy_lump(F_uint16, LUMP_PRIMINDICES, primindices, dg)) {
    return false;
  }

  if (has_lump(LUMP_PAKFILE)) {
    if (!copy_pak_lump(dg)) {
      return false;
    }
  }

  if (!copy_lump(F_vector, LUMP_CLIPPORTALVERTS, clip_portal_verts, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_CUBEMAPS, cubemap_samples, dg)) {
    return false;
  }

  if (!copy_lump(F_int8, LUMP_TEXDATA_STRING_DATA, tex_data_string_data, dg)) {
    return false;
  }

  if (!copy_lump(F_int32, LUMP_TEXDATA_STRING_TABLE, tex_data_string_table, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_OVERLAYS, overlays, dg)) {
    return false;
  }

  if (!copy_lump(F_uint16, LUMP_LEAFMINDISTTOWATER, leaf_min_dist_to_water, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_FACE_MACRO_TEXTURE_INFO, face_macro_texture_infos, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_DISP_TRIS, disptris, dg)) {
    return false;
  }

  // LUMP_PROP_BLOB
  //if (!copy_lump())

  if (!copy_lump(LUMP_WATEROVERLAYS, water_overlays, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_LEAF_AMBIENT_INDEX_HDR, leafambientindex_hdr, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_LEAF_AMBIENT_INDEX, leafambientindex_ldr, dg)) {
    return false;
  }

  if (!copy_lump(F_uint8, LUMP_LIGHTING_HDR, dlightdata_hdr, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_WORLDLIGHTS_HDR, dworldlights_hdr, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_LEAF_AMBIENT_LIGHTING_HDR, leafambientlighting_hdr, dg)) {
    return false;
  }

  if (!copy_lump(LUMP_LEAF_AMBIENT_LIGHTING, leafambientlighting_ldr, dg)) {
    return false;
  }

  // LUMP_XZIPPAKFILE

  if (!copy_lump(LUMP_FACES_HDR, dfaces_hdr, dg)) {
    return false;
  }

  //if (!copy_lump(LUMP_MAP_FLAGS, level_fl))

  if (!copy_lump(LUMP_OVERLAY_FADES, overlay_fades, dg)) {
    return false;
  }

  // LUMP_PHYSLEVEL

  //if (!copy_lump(LUMP_DISP_MULTIBLEND, disp_))

  //
  // Pick the correct HDR/non-HDR lighting lumps based on what we have.
  //

  if (has_lump(LUMP_LIGHTING_HDR)) {
    pdlightdata = &dlightdata_hdr;
  } else {
    pdlightdata = &dlightdata_ldr;
  }

  if (has_lump(LUMP_LEAF_AMBIENT_INDEX_HDR)) {
    pleafambientindex = &leafambientindex_hdr;
  } else {
    pleafambientindex = &leafambientindex_ldr;
  }

  if (has_lump(LUMP_LEAF_AMBIENT_LIGHTING_HDR)) {
    pleafambientlighting = &leafambientlighting_hdr;
  } else {
    pleafambientlighting = &leafambientlighting_ldr;
  }

  return true;
}

/**
 * Writes the BSPData to the indicated datagram, suitable for writing to disk.
 */
void BSPData::
write_datagram(Datagram &dg) {
}

/**
 * Uncompresses the indicated lump and fills in the output datagram with the
 * uncompressed data.
 *
 * Returns true on success, or false if the lump was not compressed or couldn't
 * be uncompressed.
 */
bool BSPData::
uncompress_lump(int lump, const Datagram &dg, Datagram &out_dg) {
  if (_lumps[lump].uncompressed_size) {
    // Lump is LZMA compressed.  Need to uncompress it into a temp buffer and
    // read from that.
    unsigned char *input = (unsigned char *)dg.get_data() + _lumps[lump].file_offset;
    CLZMA lzma;
    if (lzma.IsCompressed(input)) {
      if (bsp_cat.is_debug()) {
        bsp_cat.debug()
          << "Uncompressing compressed lump " << lump << "\n";
      }
      unsigned char *output = new unsigned char[lzma.GetActualSize(input)];
      if (!lzma.Uncompress(input, output)) {
        bsp_cat.error()
          << "Failed to uncompress lump " << lump << "\n";
        delete[] output;
        return false;
      }

      out_dg.assign(output, lzma.GetActualSize(input));

      delete[] output;
    }

    return true;
  }

  return false;
}

/**
 * Copies the occluder lump from the datagram.
 *
 * We need a special handler for this because for some reason, Valve decided
 * to combine three lumps into one.
 *
 * Returns true if the lump was successfully copied, false otherwise.
 */
bool BSPData::
copy_occlusion_lump(const Datagram &dg) {
  bsp_cat.debug()
    << "Reading occlusion lump\n";

  int length = _lumps[LUMP_OCCLUSION].file_length;
  int offset = _lumps[LUMP_OCCLUSION].file_offset;

  const Datagram *the_dg = &dg;

  Datagram decoded_dg;

  if (_lumps[LUMP_OCCLUSION].uncompressed_size) {
    if (!uncompress_lump(LUMP_OCCLUSION, dg, decoded_dg)) {
      return false;
    }

    the_dg = &decoded_dg;
    offset = 0;
    length = _lumps[LUMP_OCCLUSION].uncompressed_size;
  }

  DatagramIterator dgi(*the_dg, offset);

  // First get the number of DOccluderDatas.
  int count = dgi.get_int32();
  if (count > 0) {
    // Read them in.
    occluder_data.resize(count);
    for (int i = 0; i < count; i++) {
      occluder_data[i].read_datagram(dgi, LUMP_OCCLUSION_VERSION);
    }
  }

  // Now read in the DOccluderPolyDatas.
  count = dgi.get_int32();
  if (count > 0) {
    occluder_poly_data.resize(count);
    for (int i = 0; i < count; i++) {
      occluder_poly_data[i].read_datagram(dgi, LUMP_OCCLUSION_VERSION);
    }
  }

  // Finally, the vertex indices.
  count = dgi.get_int32();
  if (count > 0) {
    occluder_vertex_indices.resize(count);
    for (int i = 0; i < count; i++) {
      occluder_vertex_indices[i] = dgi.get_int32();
    }
  }

  return true;
}

/**
 * Copies the pak file lump from the indicated datagram into a ZipArchive
 * object.
 *
 * Returns true on success, or false if there was an error.
 */
bool BSPData::
copy_pak_lump(const Datagram &dg) {
  bsp_cat.debug()
    << "Reading pak file lump\n";

  int length = _lumps[LUMP_PAKFILE].file_length;
  int offset = _lumps[LUMP_PAKFILE].file_offset;

  const Datagram *the_dg = &dg;

  Datagram decoded_dg;

  if (_lumps[LUMP_PAKFILE].uncompressed_size) {
    if (!uncompress_lump(LUMP_PAKFILE, dg, decoded_dg)) {
      return false;
    }

    the_dg = &decoded_dg;
    offset = 0;
    length = _lumps[LUMP_PAKFILE].uncompressed_size;
  }

  DatagramIterator dgi(*the_dg, offset);

  std::istringstream *ss = new std::istringstream(
    std::string((const char *)the_dg->get_data() + offset, length));
  IStreamWrapper *wrapper = new IStreamWrapper(ss, true);

  _pak_file = new ZipArchive;
  if (!_pak_file->open_read(wrapper, true)) {
    bsp_cat.error()
      << "Couldn't open the pak file lump\n";
    _pak_file = nullptr;
    //return false;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->write_file("pakfile_lump.zip", (const unsigned char *)the_dg->get_data() + offset, length, false);

  return true;
}
