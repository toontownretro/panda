/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspLumps.cxx
 * @author brian
 * @date 2021-01-01
 */

#include "bspLumps.h"

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DFlagsLump::
read_datagram(DatagramIterator &dgi, int version) {
  level_flags = dgi.get_uint32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DFlagsLump::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint32(level_flags);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DGameLump::
read_datagram(DatagramIterator &dgi, int version) {
  id = (GameLumpId)dgi.get_int32();
  flags = dgi.get_uint16();
  version = dgi.get_uint16();
  fileofs = dgi.get_int32();
  filelen = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DGameLump::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(id);
  dg.add_uint16(flags);
  dg.add_uint16(version);
  dg.add_int32(fileofs);
  dg.add_int32(filelen);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DModel::
read_datagram(DatagramIterator &dgi, int version) {
  mins.read_datagram_fixed(dgi);
  maxs.read_datagram_fixed(dgi);
  origin.read_datagram_fixed(dgi);
  head_node = dgi.get_int32();
  first_face = dgi.get_int32();
  num_faces = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DModel::
write_datagram(Datagram &dg, int version) const {
  mins.write_datagram_fixed(dg);
  maxs.write_datagram_fixed(dg);
  origin.write_datagram_fixed(dg);
  dg.add_int32(head_node);
  dg.add_int32(first_face);
  dg.add_int32(num_faces);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DPhysModel::
read_datagram(DatagramIterator &dgi, int version) {
  model_index = dgi.get_int32();
  data_size = dgi.get_int32();
  keydata_size = dgi.get_int32();
  solid_count = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DPhysModel::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(model_index);
  dg.add_int32(data_size);
  dg.add_int32(keydata_size);
  dg.add_int32(solid_count);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DPhysDisp::
read_datagram(DatagramIterator &dgi, int version) {
  num_displacements = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DPhysDisp::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(num_displacements);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DVertex::
read_datagram(DatagramIterator &dgi, int version) {
  point.read_datagram_fixed(dgi);
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DVertex::
write_datagram(Datagram &dg, int version) const {
  point.write_datagram_fixed(dg);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DPlane::
read_datagram(DatagramIterator &dgi, int version) {
  normal.read_datagram_fixed(dgi);
  dist = dgi.get_float32();
  type = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DPlane::
write_datagram(Datagram &dg, int version) const {
  normal.write_datagram_fixed(dg);
  dg.add_float32(dist);
  dg.add_int32(type);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DNode::
read_datagram(DatagramIterator &dgi, int version) {
  plane_num = dgi.get_int32();

  children[0] = dgi.get_int32();
  children[1] = dgi.get_int32();

  mins[0] = dgi.get_int16();
  mins[1] = dgi.get_int16();
  mins[2] = dgi.get_int16();

  maxs[0] = dgi.get_int16();
  maxs[1] = dgi.get_int16();
  maxs[2] = dgi.get_int16();

  first_face = dgi.get_uint16();
  num_faces = dgi.get_uint16();

  area = dgi.get_int16();

  padding = dgi.get_int16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DNode::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(plane_num);

  dg.add_int32(children[0]);
  dg.add_int32(children[1]);

  dg.add_int16(mins[0]);
  dg.add_int16(mins[1]);
  dg.add_int16(mins[2]);

  dg.add_int16(maxs[0]);
  dg.add_int16(maxs[1]);
  dg.add_int16(maxs[2]);

  dg.add_uint16(first_face);
  dg.add_uint16(num_faces);

  dg.add_int16(area);

  dg.add_int16(0); // padding
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void TexInfo::
read_datagram(DatagramIterator &dgi, int version) {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      texture_vecs[i][j] = dgi.get_float32();
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      lightmap_vecs[i][j] = dgi.get_float32();
    }
  }

  flags = dgi.get_int32();
  texdata = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void TexInfo::
write_datagram(Datagram &dg, int version) const {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      dg.add_float32(texture_vecs[i][j]);
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      dg.add_float32(lightmap_vecs[i][j]);
    }
  }

  dg.add_int32(flags);
  dg.add_int32(texdata);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DTexData::
read_datagram(DatagramIterator &dgi, int version) {
  reflectivity.read_datagram_fixed(dgi);
  name_string_table_id = dgi.get_int32();
  width = dgi.get_int32();
  height = dgi.get_int32();
  view_width = dgi.get_int32();
  view_height = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DTexData::
write_datagram(Datagram &dg, int version) const {
  reflectivity.write_datagram_fixed(dg);
  dg.add_int32(name_string_table_id);
  dg.add_int32(width);
  dg.add_int32(height);
  dg.add_int32(view_width);
  dg.add_int32(view_height);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DOccluderData::
read_datagram(DatagramIterator &dgi, int version) {
  flags = dgi.get_int32();
  first_poly = dgi.get_int32();
  poly_count = dgi.get_int32();
  mins.read_datagram_fixed(dgi);
  maxs.read_datagram_fixed(dgi);

  if (version >= 1) {
    area = dgi.get_int32();
  }
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DOccluderData::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(flags);
  dg.add_int32(first_poly);
  dg.add_int32(poly_count);
  mins.write_datagram_fixed(dg);
  maxs.write_datagram_fixed(dg);

  if (version >= 1) {
    dg.add_int32(area);
  }
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DOccluderPolyData::
read_datagram(DatagramIterator &dgi, int version) {
  first_vertex_index = dgi.get_int32();
  vertex_count = dgi.get_int32();
  plane_num = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DOccluderPolyData::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(first_vertex_index);
  dg.add_int32(vertex_count);
  dg.add_int32(plane_num);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DispSubNeighbor::
read_datagram(DatagramIterator &dgi, int version) {
  neighbor = dgi.get_uint16();
  neighbor_orientation = dgi.get_uint8();
  span = dgi.get_uint8();
  neighbor_span = dgi.get_uint8();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DispSubNeighbor::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(neighbor);
  dg.add_uint8(neighbor_orientation);
  dg.add_uint8(span);
  dg.add_uint8(neighbor_span);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DispNeighbor::
read_datagram(DatagramIterator &dgi, int version) {
  sub_neighbors[0].read_datagram(dgi, version);
  sub_neighbors[1].read_datagram(dgi, version);
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DispNeighbor::
write_datagram(Datagram &dg, int version) const {
  sub_neighbors[0].write_datagram(dg, version);
  sub_neighbors[1].write_datagram(dg, version);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DispCornerNeighbors::
read_datagram(DatagramIterator &dgi, int version) {
  for (int i = 0; i < MAX_DISP_CORNER_NEIGHBORS; i++) {
    neighbors[i] = dgi.get_uint16();
  }

  num_neighbors = dgi.get_uint8();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DispCornerNeighbors::
write_datagram(Datagram &dg, int version) const {
  for (int i = 0; i < MAX_DISP_CORNER_NEIGHBORS; i++) {
    dg.add_uint16(neighbors[i]);
  }

  dg.add_uint8(num_neighbors);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DispVert::
read_datagram(DatagramIterator &dgi, int version) {
  vector.read_datagram_fixed(dgi);
  dist = dgi.get_float32();
  alpha = dgi.get_float32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DispVert::
write_datagram(Datagram &dg, int version) const {
  vector.write_datagram_fixed(dg);
  dg.add_float32(dist);
  dg.add_float32(alpha);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DispTri::
read_datagram(DatagramIterator &dgi, int version) {
  tags = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DispTri::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(tags);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DispMultiBlend::
read_datagram(DatagramIterator &dgi, int version) {
  multi_blend.read_datagram_fixed(dgi);
  alpha_blend.read_datagram_fixed(dgi);
  for (int i = 0; i < MAX_DISP_MULTIBLEND_CHANNELS; i++) {
    multi_blend_colors[i].read_datagram_fixed(dgi);
  }
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DispMultiBlend::
write_datagram(Datagram &dg, int version) const {
  multi_blend.write_datagram_fixed(dg);
  alpha_blend.write_datagram_fixed(dg);
  for (int i = 0; i < MAX_DISP_MULTIBLEND_CHANNELS; i++) {
    multi_blend_colors[i].write_datagram_fixed(dg);
  }
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DDispInfo::
read_datagram(DatagramIterator &dgi, int version) {
  start_position.read_datagram_fixed(dgi);
  disp_vert_start = dgi.get_int32();
  disp_tri_start = dgi.get_int32();

  power = dgi.get_int32();
  min_tess = dgi.get_int32();
  smoothing_angle = dgi.get_float32();
  contents = dgi.get_int32();

  map_face = dgi.get_uint16();

  lightmap_alpha_start = dgi.get_int32();
  lightmap_sample_position_start = dgi.get_int32();

  for (int i = 0; i < 4; i++) {
    edge_neighbors[i].read_datagram(dgi, version);
  }

  for (int i = 0; i < 4; i++) {
    corner_neighbors[i].read_datagram(dgi, version);
  }

  for (int i = 0; i < ALLOWEDVERTS_SIZE; i++) {
    allowed_verts[i] = dgi.get_uint32();
  }
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DDispInfo::
write_datagram(Datagram &dg, int version) const {
  start_position.write_datagram_fixed(dg);
  dg.add_int32(disp_vert_start);
  dg.add_int32(disp_tri_start);

  dg.add_int32(power);
  dg.add_int32(min_tess);
  dg.add_float32(smoothing_angle);
  dg.add_int32(contents);

  dg.add_uint16(map_face);

  dg.add_int32(lightmap_alpha_start);
  dg.add_int32(lightmap_sample_position_start);

  for (int i = 0; i < 4; i++) {
    edge_neighbors[i].write_datagram(dg, version);
  }

  for (int i = 0; i < 4; i++) {
    corner_neighbors[i].write_datagram(dg, version);
  }

  for (int i = 0; i < ALLOWEDVERTS_SIZE; i++) {
    dg.add_uint32(allowed_verts[i]);
  }
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DEdge::
read_datagram(DatagramIterator &dgi, int version) {
  v[0] = dgi.get_uint16();
  v[1] = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DEdge::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(v[0]);
  dg.add_uint16(v[1]);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DPrimitive::
read_datagram(DatagramIterator &dgi, int version) {
  type = dgi.get_uint8();
  first_index = dgi.get_uint16();
  index_count = dgi.get_uint16();
  first_vert = dgi.get_uint16();
  vert_count = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DPrimitive::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint8(type);
  dg.add_uint16(first_index);
  dg.add_uint16(index_count);
  dg.add_uint16(first_vert);
  dg.add_uint16(vert_count);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DPrimVert::
read_datagram(DatagramIterator &dgi, int version) {
  pos.read_datagram_fixed(dgi);
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DPrimVert::
write_datagram(Datagram &dg, int version) const {
  pos.write_datagram_fixed(dg);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DFace::
read_datagram(DatagramIterator &dgi, int version) {
  plane_num = dgi.get_uint16();
  side = dgi.get_uint8();
  on_node = dgi.get_uint8();

  first_edge = dgi.get_int32();
  num_edges = dgi.get_int16();
  texinfo = dgi.get_int16();

  dispinfo = dgi.get_int16();
  surface_fog_volume_id = dgi.get_int16();

  for (int i = 0; i < MAXLIGHTMAPS; i++) {
    styles[i] = dgi.get_uint8();
  }

  lightofs = dgi.get_int32();
  area = dgi.get_float32();

  lightmap_mins[0] = dgi.get_int32();
  lightmap_mins[1] = dgi.get_int32();

  lightmap_size[0] = dgi.get_int32();
  lightmap_size[1] = dgi.get_int32();

  orig_face = dgi.get_int32();

  num_prims = dgi.get_uint16();

  first_prim_id = dgi.get_uint16();
  smoothing_groups = dgi.get_uint32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DFace::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(plane_num);
  dg.add_uint8(side);
  dg.add_uint8(on_node);

  dg.add_int32(first_edge);
  dg.add_int16(num_edges);
  dg.add_int16(texinfo);

  dg.add_int16(dispinfo);
  dg.add_int16(surface_fog_volume_id);

  for (int i = 0; i < MAXLIGHTMAPS; i++) {
    dg.add_uint8(styles[i]);
  }

  dg.add_int32(lightofs);
  dg.add_float32(area);

  dg.add_int32(lightmap_mins[0]);
  dg.add_int32(lightmap_mins[1]);

  dg.add_int32(lightmap_size[0]);
  dg.add_int32(lightmap_size[1]);

  dg.add_int32(orig_face);

  dg.add_uint16(num_prims);

  dg.add_uint16(first_prim_id);
  dg.add_uint32(smoothing_groups);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DFaceID::
read_datagram(DatagramIterator &dgi, int version) {
  hammer_face_id = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DFaceID::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(hammer_face_id);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DFaceBrushList::
read_datagram(DatagramIterator &dgi, int version) {
  face_brush_count = dgi.get_uint16();
  face_brush_start = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DFaceBrushList::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(face_brush_count);
  dg.add_uint16(face_brush_start);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DLeaf::
read_datagram(DatagramIterator &dgi, int version) {
  contents = dgi.get_int32();

  cluster = dgi.get_int16();

  area_flags = dgi.get_int16();

  mins[0] = dgi.get_int16();
  mins[1] = dgi.get_int16();
  mins[2] = dgi.get_int16();

  maxs[0] = dgi.get_int16();
  maxs[1] = dgi.get_int16();
  maxs[2] = dgi.get_int16();

  first_leaf_face = dgi.get_uint16();
  num_leaf_faces = dgi.get_uint16();

  first_leaf_brush = dgi.get_uint16();
  num_leaf_brushes = dgi.get_uint16();

  leaf_water_data_id = dgi.get_int16();

  // Yuck
  padding = dgi.get_int16();

  if (version < 1) {
    ambient_lighting.read_datagram(dgi);
  }
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DLeaf::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(contents);

  dg.add_int16(cluster);

  dg.add_int16(area_flags);

  dg.add_int16(mins[0]);
  dg.add_int16(mins[1]);
  dg.add_int16(mins[2]);

  dg.add_int16(maxs[0]);
  dg.add_int16(maxs[1]);
  dg.add_int16(maxs[2]);

  dg.add_uint16(first_leaf_face);
  dg.add_uint16(num_leaf_faces);

  dg.add_uint16(first_leaf_brush);
  dg.add_uint16(num_leaf_brushes);

  dg.add_int16(leaf_water_data_id);

  // Padding.. ugh
  dg.add_int16(0);

  if (version < 1) {
    ambient_lighting.write_datagram(dg);
  }
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DLeafAmbientLighting::
read_datagram(DatagramIterator &dgi, int version) {
  cube.read_datagram(dgi);
  x = dgi.get_uint8();
  y = dgi.get_uint8();
  z = dgi.get_uint8();
  pad = dgi.get_uint8();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DLeafAmbientLighting::
write_datagram(Datagram &dg, int version) const {
  cube.write_datagram(dg);
  dg.add_uint8(x);
  dg.add_uint8(y);
  dg.add_uint8(z);
  dg.add_uint8(pad);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DLeafAmbientIndex::
read_datagram(DatagramIterator &dgi, int version) {
  ambient_sample_count = dgi.get_uint16();
  first_ambient_sample = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DLeafAmbientIndex::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(ambient_sample_count);
  dg.add_uint16(first_ambient_sample);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DBrushSide::
read_datagram(DatagramIterator &dgi, int version) {
  plane_num = dgi.get_uint16();
  texinfo = dgi.get_int16();
  dispinfo = dgi.get_int16();
  bevel = dgi.get_uint8();
  thin = dgi.get_uint8();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DBrushSide::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(plane_num);
  dg.add_int16(texinfo);
  dg.add_int16(dispinfo);
  dg.add_uint8(bevel);
  dg.add_uint8(thin);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DBrush::
read_datagram(DatagramIterator &dgi, int version) {
  first_side = dgi.get_int32();
  num_sides = dgi.get_int32();
  contents = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DBrush::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(first_side);
  dg.add_int32(num_sides);
  dg.add_int32(contents);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DVis::
read_datagram(DatagramIterator &dgi, int version) {
  num_clusters = dgi.get_int32();

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      bitofs[i][j] = dgi.get_int32();
    }
  }
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DVis::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(num_clusters);

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      dg.add_int32(bitofs[i][j]);
    }
  }
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DAreaPortal::
read_datagram(DatagramIterator &dgi, int version) {
  portal_key = dgi.get_uint16();
  other_area = dgi.get_uint16();
  first_clip_portal_vert = dgi.get_uint16();
  num_clip_portal_verts = dgi.get_uint16();
  plane_num = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DAreaPortal::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(portal_key);
  dg.add_uint16(other_area);
  dg.add_uint16(first_clip_portal_vert);
  dg.add_uint16(num_clip_portal_verts);
  dg.add_int32(plane_num);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DArea::
read_datagram(DatagramIterator &dgi, int version) {
  num_area_portals = dgi.get_int32();
  first_area_portal = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DArea::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(num_area_portals);
  dg.add_int32(first_area_portal);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DLeafWaterData::
read_datagram(DatagramIterator &dgi, int version) {
  surface_z = dgi.get_float32();
  min_z = dgi.get_float32();
  surface_texinfo_id = dgi.get_int16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DLeafWaterData::
write_datagram(Datagram &dg, int version) const {
  dg.add_float32(surface_z);
  dg.add_float32(min_z);
  dg.add_int16(surface_texinfo_id);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void FaceMacroTextureInfo::
read_datagram(DatagramIterator &dgi, int version) {
  macro_texture_name_id = dgi.get_uint16();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void FaceMacroTextureInfo::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint16(macro_texture_name_id);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DWorldlight::
read_datagram(DatagramIterator &dgi, int version) {
  origin.read_datagram_fixed(dgi);
  intensity.read_datagram_fixed(dgi);
  normal.read_datagram_fixed(dgi);

  if (version >= 1) {
    shadow_cast_offset.read_datagram_fixed(dgi);
  }

  cluster = dgi.get_int32();
  type = (BSPEnums::EmitType)dgi.get_int32();
  style = dgi.get_int32();
  stopdot = dgi.get_float32();
  stopdot2 = dgi.get_float32();
  exponent = dgi.get_float32();
  radius = dgi.get_float32();
  constant_attn = dgi.get_float32();
  linear_attn = dgi.get_float32();
  quadratic_attn = dgi.get_float32();
  flags = dgi.get_int32();
  texinfo = dgi.get_int32();
  owner = dgi.get_int32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DWorldlight::
write_datagram(Datagram &dg, int version) const {
  origin.write_datagram_fixed(dg);
  intensity.write_datagram_fixed(dg);
  normal.write_datagram_fixed(dg);

  if (version >= 1) {
    shadow_cast_offset.write_datagram_fixed(dg);
  }

  dg.add_int32(cluster);
  dg.add_int32(type);
  dg.add_int32(style);
  dg.add_float32(stopdot);
  dg.add_float32(stopdot2);
  dg.add_float32(exponent);
  dg.add_float32(radius);
  dg.add_float32(constant_attn);
  dg.add_float32(linear_attn);
  dg.add_float32(quadratic_attn);
  dg.add_int32(flags);
  dg.add_int32(texinfo);
  dg.add_int32(owner);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DCubeMapSample::
read_datagram(DatagramIterator &dgi, int version) {
  origin[0] = dgi.get_int32();
  origin[1] = dgi.get_int32();
  origin[2] = dgi.get_int32();

  size = dgi.get_uint8();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DCubeMapSample::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(origin[0]);
  dg.add_int32(origin[1]);
  dg.add_int32(origin[2]);

  dg.add_uint8(size);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DOverlay::
read_datagram(DatagramIterator &dgi, int version) {
  id = dgi.get_int32();
  texinfo = dgi.get_int16();
  _face_count_render_order = dgi.get_uint16();

  for (int i = 0; i < OVERLAY_BSP_FACE_COUNT; i++) {
    faces[i] = dgi.get_int32();
  }

  u[0] = dgi.get_float32();
  u[1] = dgi.get_float32();

  v[0] = dgi.get_float32();
  v[1] = dgi.get_float32();

  for (int i = 0; i < 4; i++) {
    uv_points[i].read_datagram_fixed(dgi);
  }

  origin.read_datagram_fixed(dgi);
  basis_normal.read_datagram_fixed(dgi);
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DOverlay::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(id);
  dg.add_int16(texinfo);
  dg.add_uint16(_face_count_render_order);

  for (int i = 0; i < OVERLAY_BSP_FACE_COUNT; i++) {
    dg.add_int32(faces[i]);
  }

  dg.add_float32(u[0]);
  dg.add_float32(u[1]);

  dg.add_float32(v[0]);
  dg.add_float32(v[1]);

  for (int i = 0; i < 4; i++) {
    uv_points[i].write_datagram_fixed(dg);
  }

  origin.write_datagram_fixed(dg);
  basis_normal.write_datagram_fixed(dg);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DOverlayFade::
read_datagram(DatagramIterator &dgi, int version) {
  fade_dist_min_sq = dgi.get_float32();
  fade_dist_max_sq = dgi.get_float32();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DOverlayFade::
write_datagram(Datagram &dg, int version) const {
  dg.add_float32(fade_dist_min_sq);
  dg.add_float32(fade_dist_max_sq);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DOverlaySystemLevel::
read_datagram(DatagramIterator &dgi, int version) {
  min_cpu_level = dgi.get_uint8();
  max_cpu_level = dgi.get_uint8();
  min_gpu_level = dgi.get_uint8();
  max_gpu_level = dgi.get_uint8();
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DOverlaySystemLevel::
write_datagram(Datagram &dg, int version) const {
  dg.add_uint8(min_cpu_level);
  dg.add_uint8(max_cpu_level);
  dg.add_uint8(min_gpu_level);
  dg.add_uint8(max_gpu_level);
}

/**
 * Reads the lump from the indicated datagram with the given version.
 */
void DWaterOverlay::
read_datagram(DatagramIterator &dgi, int version) {
  id = dgi.get_int32();
  texinfo = dgi.get_int16();
  _face_count_render_order = dgi.get_uint16();

  for (int i = 0; i < WATEROVERLAY_BSP_FACE_COUNT; i++) {
    faces[i] = dgi.get_int32();
  }

  u[0] = dgi.get_float32();
  u[1] = dgi.get_float32();

  v[0] = dgi.get_float32();
  v[1] = dgi.get_float32();

  for (int i = 0; i < 4; i++) {
    uv_points[i].read_datagram_fixed(dgi);
  }

  origin.read_datagram_fixed(dgi);
  basis_normal.read_datagram_fixed(dgi);
}

/**
 * Writes the lump to the indicated datagram with the given version.
 */
void DWaterOverlay::
write_datagram(Datagram &dg, int version) const {
  dg.add_int32(id);
  dg.add_int16(texinfo);
  dg.add_uint16(_face_count_render_order);

  for (int i = 0; i < WATEROVERLAY_BSP_FACE_COUNT; i++) {
    dg.add_int32(faces[i]);
  }

  dg.add_float32(u[0]);
  dg.add_float32(u[1]);

  dg.add_float32(v[0]);
  dg.add_float32(v[1]);

  for (int i = 0; i < 4; i++) {
    uv_points[i].write_datagram_fixed(dg);
  }

  origin.write_datagram_fixed(dg);
  basis_normal.write_datagram_fixed(dg);
}
