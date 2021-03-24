/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspLoader.h
 * @author lachbr
 * @date 2021-01-02
 */

#ifndef BSPLOADER_H
#define BSPLOADER_H

#include "config_bsp.h"
#include "bspRoot.h"
#include "bspData.h"
#include "lightmapPalettizer.h"

/**
 * Converts a BSP structure read from a BSP file into a scene graph suitable
 * for rendering.
 */
class EXPCL_PANDA_BSP BSPLoader {
PUBLISHED:
  INLINE BSPLoader(BSPData *data, float scale_factor = 0.0625f);

  PT(BSPRoot) load();

  static const GeomVertexFormat *get_face_format();

private:
  struct DFaceLightmapInfo {
    float s_scale, t_scale;
    float s_offset, t_offset;
    int texmins[2];
    int texsize[2];
    LightmapPalette::Entry *palette_entry;
  };

  void load_models();
  void load_model_faces(size_t model, PandaNode *model_node);
  void load_displacement(int face_num, GeomNode *geom_node);
  void write_face_vertex(GeomVertexData *vertex_data, int face_num,
                         int vert_num, const DVertex *vertex);

  void init_dface_lightmap_info(DFaceLightmapInfo *info, int facenum);

  LTexCoordf get_vertex_uv(const TexInfo *tinfo, const DVertex *vertex);
  LTexCoordf get_lightcoords(int face_num, const LVector3f &point);
private:
  PT(BSPData) _data;
  PT(BSPRoot) _top_node;
  float _scale_factor;
  PT(LightmapPaletteDirectory) _lightmap_dir;
  pvector<DFaceLightmapInfo> _face_lightmap_info;
  vector_int _face_first_vert_normal;

  static CPT(GeomVertexFormat) _face_format;
};

#include "bspLoader.I"

#endif // BSPLOADER_H
