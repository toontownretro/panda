/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspLoader.cxx
 * @author lachbr
 * @date 2021-01-02
 */

#include "bspLoader.h"
#include "bspFlags.h"
#include "lightmapPalettizer.h"
#include "transformState.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomNode.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "texturePool.h"
#include "string_utils.h"
#include "virtualFileSystem.h"
#include "materialAttrib.h"
#include "material.h"
#include "materialPool.h"
#include "modelNode.h"
#include "bspWorld.h"
#include "nodePath.h"
#include "materialParamTexture.h"

CPT(GeomVertexFormat) BSPLoader::_face_format = nullptr;

static PT(TextureStage) lightmap_stage = new TextureStage("lightmap");
static PT(TextureStage) bump_lightmap_stage = new TextureStage("lightmap_bumped");

/**
 * Builds up a scene graph from the BSP data and returns the top-level node.
 */
PT(BSPRoot) BSPLoader::
load() {
  // First, palettize the lightmaps.
  LightmapPalettizer palettes(_data);
  _lightmap_dir = palettes.palettize_lightmaps();

  _face_lightmap_info.resize(_data->dfaces.size());

  // build table of per-face beginning index into vertnormalindices
  _face_first_vert_normal.reserve(_data->dfaces.size());
  int normal_index = 0;
  for (size_t i = 0; i < _data->dfaces.size(); i++) {
    _face_first_vert_normal[i] = normal_index;
    normal_index += _data->dfaces[i].num_edges;
  }

  _top_node = new BSPRoot("level");
  _top_node->set_bsp_data(_data);

  if (_data->_pak_file) {
    // Mount the pak file that was embedded in the BSP file.
    VirtualFileSystem::get_global_ptr()->mount(
      _data->_pak_file, ".", VirtualFileSystem::MF_read_only);
  }

  load_models();

  return _top_node;
}

/**
 * Returns the GeomVertexFormat for a brush face.
 */
const GeomVertexFormat *BSPLoader::
get_face_format() {
  if (_face_format == nullptr) {
    PT(GeomVertexArrayFormat) arr_format = new GeomVertexArrayFormat;
    arr_format->add_column(InternalName::get_vertex(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_point);
    arr_format->add_column(InternalName::get_texcoord(), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
    arr_format->add_column(InternalName::make("texcoord_lightmap"), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
    arr_format->add_column(InternalName::get_normal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_normal);
    arr_format->add_column(InternalName::get_tangent(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
    arr_format->add_column(InternalName::get_binormal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);

    PT(GeomVertexFormat) format = new GeomVertexFormat;
    format->add_array(arr_format);

    _face_format = GeomVertexFormat::register_format(format);
  }

  return _face_format;
}

/**
 * Converts each DModel from the BSP data into a scene graph, containing
 * geometry for the faces.
 */
void BSPLoader::
load_models() {
  _face_geoms.clear();
  _face_geoms.resize(_data->dfaces.size());

  // First load each face into a geom/state pair.
  for (size_t i = 0; i < _data->dfaces.size(); i++) {
    size_t face_num = i;
    DFace &face = _data->dfaces[face_num];

    DFaceLightmapInfo lminfo;
    init_dface_lightmap_info(&lminfo, face_num);
    _face_lightmap_info[face_num] = lminfo;

    int num_verts = 0;

    PT(GeomVertexData) vertex_data = new
    GeomVertexData("face-vertices", get_face_format(),
                  GeomEnums::UH_static);

    if (face.dispinfo != -1) {
      // This is a displacement face.
      //load_displacement(face_num, geom_node);
      //continue;
    }

    pmap<DVertex *, int> prim_vert_indices;

    PT(Geom) geom = new Geom(vertex_data);
    PT(GeomTriangles) triangles = new GeomTriangles(GeomEnums::UH_static);

    // Regular brush face.
    int num_tris = face.num_edges - 2;
    for (int tri = 0; tri < num_tris; tri++) {
      for (int j = 2; j >= 0; j--) {

        int vnum;
        if (j == 0) {
          vnum = 0;
        } else {
          vnum = (tri + j) % face.num_edges;
        }

        DVertex *vert = _data->get_face_vertex(&face, vnum);

        auto it = prim_vert_indices.find(vert);
        if (it == prim_vert_indices.end()) {
          // We haven't written this vertex yet for this face.  Do it.
          write_face_vertex(vertex_data, face_num, vnum, vert);
          prim_vert_indices[vert] = num_verts++;
        }

        triangles->add_vertex(prim_vert_indices[vert]);
      }
      triangles->close_primitive();
    }

    geom->add_primitive(triangles);

    CPT(RenderState) state = RenderState::make_empty();

    // Look up the material for the face.
    if (face.texinfo >= 0 && face.texinfo < (int)_data->texinfo.size()) {
      const TexInfo *tinfo = &_data->texinfo[face.texinfo];
      if (tinfo->flags & (SURF_SKY | SURF_SKY2D | SURF_SKIP | SURF_NODRAW)) {
        FaceGeom face_geom;
        face_geom.geom = new Geom(vertex_data);
        face_geom.state = state;
        face_geom.cluster = -1;
        _face_geoms[face_num] = std::move(face_geom);
        continue;
      }
      if (tinfo->texdata >= 0 && tinfo->texdata < (int)_data->dtexdata.size()) {
        const DTexData *tdata = &_data->dtexdata[tinfo->texdata];
        std::string tex_name = _data->get_string(tdata->name_string_table_id);
        tex_name = downcase(tex_name);
        tex_name += ".pmat";
        Filename tex_filename = Filename::from_os_specific(tex_name);
        PT(Material) mat = MaterialPool::load_material(tex_filename);
        if (mat != nullptr) {
          state = state->set_attrib(MaterialAttrib::make(mat));
          MaterialParamBase *base_p = mat->get_param("base_color");
          if (base_p != nullptr && base_p->is_of_type(MaterialParamTexture::get_class_type())) {
            MaterialParamTexture *base_tex_p = DCAST(MaterialParamTexture, base_p);
            Texture *base_tex = base_tex_p->get_value();
            if (base_tex != nullptr && Texture::has_alpha(base_tex->get_format())) {
              state = state->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_dual));
            }
          }
        }

      } else {
        std::cout << "Bad texdata index " << tinfo->texdata << "\n";
      }
    } else {
      std::cout << "Bad texinfo index " << face.texinfo << "\n";
    }

    CPT(RenderAttrib) tattr = state->get_attrib_def(TextureAttrib::get_class_slot());
    // Tack on the lightmap texture.
    if (face.lightofs != -1) {
      tattr = DCAST(TextureAttrib, tattr)->add_on_stage(
        /*(tinfo->flags & SURF_BUMPLIGHT)*/false ? bump_lightmap_stage : lightmap_stage,
        lminfo.palette_entry->palette->texture);

    }
    state = state->set_attrib(tattr);

    FaceGeom face_geom;
    face_geom.geom = geom;
    face_geom.state = state;
    face_geom.cluster = -1;
    _face_geoms[face_num] = std::move(face_geom);
  }


  // Now associate the faces with clusters.
  for (size_t leaf_num = 0; leaf_num < _data->dleafs.size(); leaf_num++) {
    const DLeaf &leaf = _data->dleafs[leaf_num];
    for (size_t i = 0; i < leaf.num_leaf_faces; i++) {
      size_t leaf_face = leaf.first_leaf_face + i;
      size_t face_num = _data->dleaffaces[leaf_face];

      if (face_num >= (int)_data->dfaces.size()) {
        continue;
      }

      FaceGeom &face_geom = _face_geoms[face_num];
      face_geom.cluster = leaf.cluster;
    }
  }

  for (size_t i = 0; i < _data->dmodels.size(); i++)  {
    DModel &model = _data->dmodels[i];

    PT(PandaNode) model_node;
    if (i == 0) {
      model_node = new BSPWorld(_data);

    } else {
      std::ostringstream name_ss;
      name_ss << "model-" << i;
      model_node = new ModelNode(name_ss.str());
    }

    _top_node->add_child(model_node);

    // Load the faces of the model.
    load_model_faces(i, model_node);
  }
}

/**
 * Converts the DFaces of a given DModel to scene graph geometry.
 */
void BSPLoader::
load_model_faces(size_t model_num, PandaNode *model_node) {
  DModel &model = _data->dmodels[model_num];

  bool hdr = (_data->pdlightdata == &(_data->dlightdata_hdr));

  if (model_num == 0) {
    BSPWorld *bsp_world = DCAST(BSPWorld, model_node);

    // For the world, the faces for each cluster goes in it's own GeomNode, and
    // the Geoms in each cluster are flattened together.

    pmap<int, PT(GeomNode)> cluster_gn_map;
    for (int i = 0; i < model.num_faces; i++) {
      int face_num = model.first_face + i;
      const FaceGeom &face_geom = _face_geoms[face_num];
      int face_cluster = face_geom.cluster;
      if (face_cluster == -1) {
        continue;
      }
      auto it = cluster_gn_map.find(face_cluster);

      PT(GeomNode) geom_node;
      if (it == cluster_gn_map.end()) {
        // Create a new GeomNode for this cluster.
        std::ostringstream ss;
        ss << "cluster-" << face_cluster << "-geometry";
        geom_node = new GeomNode(ss.str());
        cluster_gn_map[face_cluster] = geom_node;
      } else {
        geom_node = (*it).second;
      }

      // Add the face geom to the cluster geom node.
      geom_node->add_geom(face_geom.geom, face_geom.state);
    }

    // Now flatten together the Geoms in each leaf GeomNode and add it to the
    // world node for rendering.
    for (auto it = cluster_gn_map.begin(); it != cluster_gn_map.end(); ++it) {
      int cluster_num = (*it).first;
      GeomNode *geom_node = (*it).second;
      NodePath(geom_node).flatten_strong();
      bsp_world->set_cluster_geom_node(cluster_num, geom_node);
    }

  } else {
    // Stuff all non-world faces into a single GeomNode.
    PT(GeomNode) geom_node = new GeomNode("geometry");
    for (int i = 0; i < model.num_faces; i++) {
      int face_num = model.first_face + i;
      const FaceGeom &face_geom = _face_geoms[face_num];
      geom_node->add_geom(face_geom.geom, face_geom.state);
    }
    NodePath(geom_node).flatten_strong();
    model_node->add_child(geom_node);
  }
}

/**
 * Loads a displacement surface.
 */
void BSPLoader::
load_displacement(int face_num, GeomNode *geom_node) {
  const DFace *face = &_data->dfaces[face_num];
  const DDispInfo *dispinfo = &_data->dispinfo[face->dispinfo];
  //dispinfo->
}

/**
 * Writes the indicated face vertex into the given vertex buffer.
 */
void BSPLoader::
write_face_vertex(GeomVertexData *data, int face_num,
                  int vert_num, const DVertex *vertex) {
  int row = data->get_num_rows();
  const DFace &face = _data->dfaces[face_num];
  const TexInfo &texinfo = _data->texinfo[face.texinfo];

  LVector3f normal = _data->vertnormals[
    _data->vertnormalindices[_face_first_vert_normal[face_num] + vert_num]];

  // Also calculate the tangent and binormal.
  LVector3f binormal;
  binormal.set(texinfo.texture_vecs[1][0], texinfo.texture_vecs[1][1],
               texinfo.texture_vecs[1][2]);
  binormal.normalize();
  LVector3f tangent;
  tangent = normal.cross(binormal).normalized();
  binormal = tangent.cross(normal).normalized();
  // Adjust for backwards mapping if need be.
  LVector3f tmp = LVector3f(
    texinfo.texture_vecs[0][0], texinfo.texture_vecs[0][1],
    texinfo.texture_vecs[0][2]).cross(
      LVector3f(
        texinfo.texture_vecs[1][0], texinfo.texture_vecs[1][1],
        texinfo.texture_vecs[1][2]));
  if (normal.dot(tmp) > 0.0f) {
    tangent = -tangent;
  }

  GeomVertexWriter tangent_writer(data, InternalName::get_tangent());
  tangent_writer.set_row(row);
  tangent_writer.add_data3f(tangent);

  GeomVertexWriter binormal_writer(data, InternalName::get_binormal());
  binormal_writer.set_row(row);
  binormal_writer.add_data3f(binormal);

  GeomVertexWriter normal_writer(data, InternalName::get_normal());
  normal_writer.set_row(row);
  normal_writer.add_data3f(normal);

  GeomVertexWriter vertex_writer(data, InternalName::get_vertex());
  vertex_writer.set_row(row);
  vertex_writer.add_data3f(vertex->point * _scale_factor);

  GeomVertexWriter uv_writer(data, InternalName::get_texcoord());
  uv_writer.set_row(row);
  uv_writer.add_data2f(get_vertex_uv(&_data->texinfo[face.texinfo], vertex));

  GeomVertexWriter luv_writer(data, InternalName::make("texcoord_lightmap"));
  luv_writer.set_row(row);
  luv_writer.add_data2f(get_lightcoords(face_num, vertex->point));
}

/**
 * Initializes the lightmap info structure for the given face.
 */
void BSPLoader::
init_dface_lightmap_info(BSPLoader::DFaceLightmapInfo *info, int facenum) {
  const DFace *face = &_data->dfaces[facenum];

  info->palette_entry = _lightmap_dir->face_palette_entries[facenum];

  info->texsize[0] = face->lightmap_size[0] + 1;
  info->texsize[1] = face->lightmap_size[1] + 1;
  info->texmins[0] = face->lightmap_mins[0];
  info->texmins[1] = face->lightmap_mins[1];

  if (info->palette_entry != nullptr) {
    info->s_scale = 1.0 / (float)info->palette_entry->palette->size[0];
    info->s_offset = (float)info->palette_entry->offset[0] * info->s_scale;
  } else {
    info->s_scale = 1.0 / info->texsize[0];
    info->s_offset = 0.0;
  }

  if (info->palette_entry != nullptr) {
    info->t_scale = 1.0 / (float)info->palette_entry->palette->size[1];
    info->t_offset = (float)info->palette_entry->offset[1] * info->t_scale;
  } else {
    info->t_scale = 1.0 / info->texsize[1];
    info->t_offset = 0.0;
  }
}

/**
 * Calculates the texture UV coordinates for the given face vertex.
 */
LTexCoordf BSPLoader::
get_vertex_uv(const TexInfo *tinfo, const DVertex *vertex) {
  const DTexData *tdata = &_data->dtexdata[tinfo->texdata];

  LVector3f s_vec(tinfo->texture_vecs[0][0], tinfo->texture_vecs[0][1], tinfo->texture_vecs[0][2]);
  float s_dist = tinfo->texture_vecs[0][3];

  LVector3f t_vec(tinfo->texture_vecs[1][0], tinfo->texture_vecs[1][1], tinfo->texture_vecs[1][2]);
  float t_dist = tinfo->texture_vecs[1][3];

  LTexCoordf uv(s_vec.dot(vertex->point) + s_dist, t_vec.dot(vertex->point) + t_dist);
  uv[0] /= tdata->width;
  uv[1] /= -tdata->height;

  return uv;
}

/**
 * Calculates lightmap coordinates for a point on a face.
 */
LTexCoordf BSPLoader::
get_lightcoords(int face_num, const LVector3f &point) {
  const DFace *face = &_data->dfaces[face_num];
  const TexInfo *tinfo = &_data->texinfo[face->texinfo];
  const DFaceLightmapInfo *lminfo = &_face_lightmap_info[face_num];

  LTexCoordf lightcoord;

  lightcoord[0] = DotProduct(point, tinfo->lightmap_vecs[0]) +
    tinfo->lightmap_vecs[0][3];
  lightcoord[0] -= lminfo->texmins[0];
  lightcoord[0] += 0.5;

  lightcoord[1] = DotProduct(point, tinfo->lightmap_vecs[1]) +
    tinfo->lightmap_vecs[1][3];
  lightcoord[1] -= lminfo->texmins[1];
  lightcoord[1] += 0.5;

  lightcoord[0] *= lminfo->s_scale;
  lightcoord[0] += lminfo->s_offset;

  lightcoord[1] *= lminfo->t_scale;
  lightcoord[1] += lminfo->t_offset;

  return lightcoord;
}
