/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapBuilder.cxx
 * @author brian
 * @date 2021-07-06
 */

#include "config_mapbuilder.h"
#include "mapBuilder.h"
#include "boundingBox.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "winding.h"
#include "materialPool.h"
#include "string_utils.h"
#include "mapEntity.h"
#include "materialAttrib.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "texture.h"
#include "renderState.h"
#include "transparencyAttrib.h"
#include "physTriangleMeshData.h"
//#include "visBuilder.h"
#include "lineSegs.h"
#include "depthWriteAttrib.h"
#include "cullBinAttrib.h"
#include "threadManager.h"
#include "randomizer.h"
#include "colorAttrib.h"
#include "modelRoot.h"
#include "config_pgraph.h"
#include "lightBuilder.h"
#include "keyValues.h"
#include "sceneGraphAnalyzer.h"
#include "camera.h"
#include "frameBufferProperties.h"
#include "windowProperties.h"
#include "graphicsOutput.h"
#include "graphicsStateGuardian.h"
#include "displayRegion.h"
#include "perspectiveLens.h"
#include "graphicsEngine.h"
#include "graphicsPipeSelection.h"
#include "graphicsPipe.h"
#include "lightRampAttrib.h"
#include "antialiasAttrib.h"
#include "textureStage.h"
#include "vector_int.h"
#include "vector_string.h"
#include "shaderManager.h"
#include "config_shader.h"
#include "pointLight.h"
#include "cascadeLight.h"
#include "spotlight.h"
#include "visClusterSampler.h"
#include "visBuilderBSP.h"
#include "depthTestAttrib.h"
#include "mapObjects.h"
#include "physConvexMeshData.h"
#include "lightAttrib.h"
#include "alphaTestAttrib.h"
#include "decalProjector.h"
#include "string_utils.h"
#include "look_at.h"

#include <stack>

//#define HAVE_STEAM_AUDIO
#ifdef HAVE_STEAM_AUDIO
#include <phonon.h>
#endif

// Assuming that a hammer unit is 3/4 of an inch, multiply hammer units
// by this value to convert it to meters.
#define HAMMER_UNITS_TO_METERS 0.01905f

// DEBUG INCLUDES
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"
#include "geomVertexWriter.h"
#include "geomVertexReader.h"

static LColor cluster_colors[6] = {
  LColor(1.0, 0.5, 0.5, 1.0),
  LColor(1.0, 1.0, 0.5, 1.0),
  LColor(1.0, 0.5, 1.0, 1.0),
  LColor(0.5, 1.0, 0.5, 1.0),
  LColor(0.5, 1.0, 1.0, 1.0),
  LColor(0.5, 0.5, 1.0, 1.0),
};

/**
 *
 */
bool MapPoly::
overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const {
  LPoint3 verts[3];

  for (size_t j = 1; j < (_winding.get_num_points() - 1); j++) {
    verts[0] = _winding.get_point(0);
    verts[1] = _winding.get_point(j);
    verts[2] = _winding.get_point(j + 1);

    if (tri_box_overlap(box_center, box_half, verts[0], verts[1], verts[2])) {
      return true;
    }
  }

  return false;
}

/**
 *
 */
bool MapMesh::
overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const {
  LPoint3 verts[3];

  for (size_t i = 0; i < _polys.size(); i++) {
    if (_polys[i]->overlaps_box(box_center, box_half)) {
      return true;
    }
  }

  return false;
}

/**
 *
 */
MapBuilder::
MapBuilder(const MapBuildOptions &options) :
  _options(options),
  _3d_sky_mesh_index(-1)
{
}

/**
 * Does the dirty deed of actually building the map.  Returns true on success,
 * or false if there was a problem building the map.
 */
MapBuilder::ErrorCode MapBuilder::
build() {
  ThreadManager::_num_threads = _options.get_num_threads();

  _source_map = new MapFile;

  Filename input_fullpath = _options._input_filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(input_fullpath, get_model_path())) {
    mapbuilder_cat.error()
      << "Could not find input file " << _options._input_filename << " on model path "
      << get_model_path() << "\n";
    return EC_input_not_found;
  }

  mapbuilder_cat.info()
    << "Reading input " << input_fullpath << "\n";

  if (!_source_map->read(input_fullpath)) {
    mapbuilder_cat.error()
      << "Could not read source map file " << input_fullpath << "\n";
    return EC_input_invalid;
  }

  // Source map file is read in.

  _out_data = new MapData;
  _out_top = new ModelRoot(_source_map->_filename.get_basename_wo_extension());
  _out_node = new MapRoot(_out_data);
  _out_top->add_child(_out_node);

  // First step: create polygons.  For each entity, intersect the planes of
  // each solid that belongs to the entity.  Subtract intersecting solids
  // within each entity.  Convert displacements to polygons.

  ErrorCode ec = EC_ok;

  // Write static prop entities.
  for (auto it = _source_map->_entities.begin(); it != _source_map->_entities.end();) {
    MapEntitySrc *sent = *it;
    if (sent->_class_name != "prop_static") {
      ++it;
      continue;
    }

    MapStaticProp sprop;
    sprop._model_filename = Filename::from_os_specific(sent->_properties["model"]);
    sprop._model_filename.set_extension("bam");
    sprop._flags = 0;

    if (sent->_properties.find("origin") != sent->_properties.end()) {
      sprop._pos = KeyValues::to_3f(sent->_properties["origin"]);
    } else {
      sprop._pos.set(0, 0, 0);
    }
    if (sent->_properties.find("angles") != sent->_properties.end()) {
      LVecBase3f phr = KeyValues::to_3f(sent->_properties["angles"]);
      sprop._hpr.set(phr[1] - 90, -phr[0], phr[2]);
    } else {
      sprop._hpr.set(0, 0, 0);
    }
    if (sent->_properties.find("skin") != sent->_properties.end()) {
      string_to_int(sent->_properties["skin"], sprop._skin);
    } else {
      sprop._skin = 0;
    }
    if (sent->_properties.find("solid") != sent->_properties.end()) {
      int solid;
      string_to_int(sent->_properties["solid"], solid);
      sprop._solid = (solid != 0);
    } else {
      sprop._solid = false;
    }
    if (sent->_properties.find("disableshadows") != sent->_properties.end()) {
      int disable_shadows;
      string_to_int(sent->_properties["disableshadows"], disable_shadows);
      if (disable_shadows != 0) {
        sprop._flags |= MapStaticProp::F_no_shadows;
      }
    }
    if (sent->_properties.find("disablevertexlighting") != sent->_properties.end()) {
      int disable_vertex_lighting;
      string_to_int(sent->_properties["disablevertexlighting"], disable_vertex_lighting);
      if (disable_vertex_lighting != 0) {
        sprop._flags |= MapStaticProp::F_no_vertex_lighting;
      }
    }

    _out_data->_static_props.push_back(std::move(sprop));

    it = _source_map->_entities.erase(it);
  }

  ec = build_polygons();
  if (ec != EC_ok) {
    return ec;
  }

  // Swap world mesh into index 0.
  if (_world_mesh_index != 0) {
    PT(MapMesh) tmp = _meshes[0];
    _meshes[0] = _meshes[_world_mesh_index];
    _meshes[_world_mesh_index] = tmp;
    _world_mesh_index = 0;
  }

  // Merge func_detail meshes into the world mesh.
  for (auto it = _meshes.begin(); it != _meshes.end();) {
    MapMesh *mesh = *it;
    MapEntitySrc *ent = _source_map->_entities[mesh->_entity];
    if (ent->_class_name != "func_detail") {
      ++it;
      continue;
    }

    for (MapPoly *poly : mesh->_polys) {
      _meshes[0]->_polys.push_back(poly);
    }
    it = _meshes.erase(it);
  }

  // Now erase func_detail entities.
  //for (auto it = _source_map->_entities.begin(); it != _source_map->_entities.end();) {
  //  MapEntitySrc *ent = *it;
  //  if (ent->_class_name == "func_detail") {
  //    it = _source_map->_entities.erase(it);
  //  } else {
  //    ++it;
  //  }
  //}

  // Calculate scene bounds.
  _scene_mins.set(1e+9, 1e+9, 1e+9);
  _scene_maxs.set(-1e+9, -1e+9, -1e+9);

  for (size_t i = 0; i < _meshes.size(); i++) {
    MapMesh *mesh = _meshes[i];
    for (size_t j = 0; j < mesh->_polys.size(); j++) {
      MapPoly *poly = mesh->_polys[j];
      Winding *w = &(poly->_winding);
      for (int k = 0; k < w->get_num_points(); k++) {
        LPoint3 point = w->get_point(k);

        _scene_mins[0] = std::min(point[0], _scene_mins[0]);
        _scene_mins[1] = std::min(point[1], _scene_mins[1]);
        _scene_mins[2] = std::min(point[2], _scene_mins[2]);

        _scene_maxs[0] = std::max(point[0], _scene_maxs[0]);
        _scene_maxs[1] = std::max(point[1], _scene_maxs[1]);
        _scene_maxs[2] = std::max(point[2], _scene_maxs[2]);
      }
    }
  }

  _scene_bounds = new BoundingBox(_scene_mins, _scene_maxs);

  // Output entity information.
  for (size_t i = 0; i < _source_map->_entities.size(); i++) {
    MapEntitySrc *src_ent = _source_map->_entities[i];
    if (src_ent->_class_name == "func_detail") {
      continue;
    }

    PT(MapEntity) ent = new MapEntity;
    ent->set_class_name(src_ent->_class_name);
    // Find mesh referencing entity.
    for (size_t j = 0; j < _meshes.size(); ++j) {
      if (_meshes[j]->_entity == i) {
        ent->set_model_index((int)j);
      }
    }

    PT(PDXElement) props = new PDXElement;
    for (auto it = src_ent->_properties.begin(); it != src_ent->_properties.end(); ++it) {
      const std::string &key = (*it).first;
      if (key == "origin" ||
          key == "angles") {
        PDXValue value;
        value.from_vec3(KeyValues::to_3f((*it).second));
        props->set_attribute(key, value);

      } else {
        props->set_attribute(key, (*it).second);
      }
    }
    ent->set_properties(props);

    for (const MapEntityConnection &conn : src_ent->_connections) {
      MapEntity::Connection mconn;
      mconn._output_name = conn._output_name;
      mconn._target_name = conn._entity_target_name;
      mconn._input_name = conn._input_name;
      mconn._delay = conn._delay;
      mconn._repeat = (conn._repeat > 0);
      mconn._parameters.push_back(conn._parameters);
      ent->add_connection(mconn);
    }

    _out_data->add_entity(ent);
  }

  //
  // VISIBILITY
  //
  switch (_options.get_vis()) {
  //case MapBuildOptions::VT_voxel:
  //  {
  //    VisBuilder vis(this);
  //    if (!vis.build()) {
  //      return EC_unknown_error;
  //    }
  //  }
  //  break;

  case MapBuildOptions::VT_bsp:
    {
      VisBuilderBSP vis;
      vis._builder = this;
      vis._hint_split = false;

      int sky_faces = 0;

      // Generate structural BSP solids.  This is the input to the solid-leaf
      // BSP tree.
      for (MapSolid *solid : _source_map->_world->_solids) {

        bool structural = true;
        for (MapSide *side : solid->_sides) {
          if (side->_displacement != nullptr) {
            structural = false;
            break;
          }
        }

        if (!structural) {
          continue;
        }

        PT(BSPSolid) bsp_solid = new BSPSolid;
        bool has_skip = false;
        bool has_hint = false;
        for (size_t i = 0; i < solid->_sides.size(); ++i) {
          MapSide *side = solid->_sides[i];

          bool hint = false;
          bool skip = false;
          bool sky = false;
          std::string matname = side->_material_filename.get_basename_wo_extension();
          matname = downcase(matname);
          if (matname.find("toolshint") != std::string::npos) {
            hint = true;

          } else if (matname.find("toolsskip") != std::string::npos ||
                    matname.find("toolsclip") != std::string::npos ||
                    matname.find("toolsplayerclip") != std::string::npos ||
                    matname.find("toolsareaportal") != std::string::npos ||
                    matname.find("toolsblock_los") != std::string::npos ||
                    matname.find("toolsblockbullets") != std::string::npos ||
                    matname.find("toolsblocklight") != std::string::npos ||
                    matname.find("toolsoccluder") != std::string::npos ||
                    matname.find("toolstrigger") != std::string::npos) {
            skip = true;

          } else if (matname.find("toolsskybox") != std::string::npos) {
            sky_faces++;
            sky = true;
          }

          if (!hint && !skip) {
            // Check if the side's material enables alpha of some sort.  If it
            // does, the side cannot be opaque.

            Filename material_filename = Filename("materials/" + downcase(side->_material_filename.get_fullpath_wo_extension()) + ".mto");

            PT(Material) poly_material = MaterialPool::load_material(material_filename);

            if (poly_material != nullptr) {
              if (poly_material->has_tag("compile_water")) {
                // Water cuts visleaves, but doesn't block visibility.
                hint = true;

              } else if ((poly_material->_attrib_flags & Material::F_transparency) != 0u &&
                  poly_material->_transparency_mode > 0) {
                skip = true;

              } else if ((poly_material->_attrib_flags & Material::F_alpha_test) != 0u &&
                         poly_material->_alpha_test_mode > 0) {
                skip = true;
              }
            }
          }

          if (skip) {
            has_skip = true;
          }
          if (hint) {
            has_hint = true;
          }

          Winding w(solid->_sides[i]->_plane);
          for (size_t j = 0; j < solid->_sides.size(); ++j) {
            if (j == i) {
              continue;
            }
            w = w.chop(-solid->_sides[j]->_plane);
          }
          PT(BSPFace) bsp_face = new BSPFace;
          bsp_face->_winding = w;
          bsp_face->_priority = 0;
          bsp_face->_hint = hint;
          bsp_face->_contents = 0;
          bsp_face->_sky = sky;
          bsp_solid->_faces.push_back(bsp_face);
          if (!skip) {
            vis._input_faces.push_back(bsp_face);
          }
        }
        bsp_solid->_opaque = !has_skip && !has_hint;
        if (bsp_solid->_opaque) {
          vis._input_solids.push_back(bsp_solid);
        }
      }

      mapbuilder_cat.info()
        << sky_faces << " sky faces\n";

      if (!vis.bake()) {
        return EC_unknown_error;
      }

      if (vis._is_leaked && vis._leak_path.size() > 1u) {
        // Show path to the outside node.
        LineSegs segs("leak_path");
        LColorf start_color(0, 0, 1, 1);
        LColorf end_color(1, 0, 0, 1);
        segs.set_thickness(2.0f);
        segs.set_color(start_color);
        segs.move_to(vis._leak_path[0]);
        for (size_t i = 1; i < vis._leak_path.size(); ++i) {
          float frac = (float)i / (float)(vis._leak_path.size() - 1u);
          LColorf color = end_color * frac + start_color * (1.0f - frac);
          segs.set_color(color);
          segs.draw_to(vis._leak_path[i]);
        }
        PT(GeomNode) leak_lines = segs.create();
        _out_top->add_child(leak_lines);
      }

      // Save off portal centers for audio probe placement.
      for (BSPVisPortal *p : vis._portal_list) {
        auto it = std::find(_portal_centers.begin(), _portal_centers.end(), p->_origin);
        if (it == _portal_centers.end()) {
          _portal_centers.push_back(p->_origin);
        }
      }

#if 0
      // Put leaf bounds in there
      LineSegs segs("leaves");
      segs.set_color(LColor(1, 0, 0, 1));

      // Collect all leaves.
      pvector<BSPNode *> leaves;
      std::stack<BSPNode *> node_stack;
      node_stack.push(vis._tree_root);
      while (!node_stack.empty()) {
        BSPNode *node = node_stack.top();
        node_stack.pop();

        if (!node->is_leaf()) {
          node_stack.push(node->_children[FRONT_CHILD]);
          node_stack.push(node->_children[BACK_CHILD]);

        } else {
          if (!node->_opaque) {
            leaves.push_back(node);
          }
        }
      }

      for (BSPNode *leaf : leaves) {

        // Collect all boundary planes of the leaf.
        pvector<LPlane> leaf_planes;
        BSPNode *node = leaf->_parent;
        BSPNode *child = leaf;
        while (node != nullptr) {
          LPlane plane = node->_plane;
          if (node->_children[FRONT_CHILD] == child) {
            // Front side.
            leaf_planes.push_back(plane);
          } else {
            // Back side.
            leaf_planes.push_back(-plane);
          }
          child = node;
          node = node->_parent;
        }

        // Now make windings for each plane and clip them to every other plane.
        for (size_t i = 0; i < leaf_planes.size(); ++i) {
          Winding w(leaf_planes[i]);
          for (size_t j = 0; j < leaf_planes.size(); ++j) {
            if (i == j) {
              continue;
            }
            w = w.chop(leaf_planes[j]);
            if (w.is_empty()) {
              break;
            }
          }

          if (w.is_empty()) {
            continue;
          }

          // Draw line segment outline of winding.
          segs.move_to(w.get_point(0));
          for (int j = 1; j < w.get_num_points(); ++j) {
            segs.draw_to(w.get_point(j));
          }
          // Close the loop.
          segs.draw_to(w.get_point(0));
        }
      }

      PT(GeomNode) leaf_lines = segs.create();
      leaf_lines->set_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
      leaf_lines->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
      leaf_lines->set_attrib(CullBinAttrib::make("fixed", 1));
      _out_top->add_child(leaf_lines);

      // Now do portals.
      LineSegs psegs("portals");
      psegs.set_color(LColor(0, 0, 1, 1));
      pset<BSPPortal *> drawn_portals;
      for (BSPNode *leaf : leaves) {
        for (BSPPortal *portal : leaf->_portals) {
          if (drawn_portals.find(portal) != drawn_portals.end()) {
            continue;
          }
          drawn_portals.insert(portal);

          if (portal->_winding.is_empty()) {
            continue;
          }

          psegs.move_to(portal->_winding.get_point(0));
          for (int i = 1; i < portal->_winding.get_num_points(); ++i) {
            psegs.draw_to(portal->_winding.get_point(i));
          }
          psegs.draw_to(portal->_winding.get_point(0));
        }
      }
      PT(GeomNode) portal_lines = psegs.create();
      portal_lines->set_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
      portal_lines->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
      portal_lines->set_attrib(CullBinAttrib::make("fixed", 2));
      //_out_top->add_child(portal_lines);
#endif
    }
    break;

  case MapBuildOptions::VT_none:
  default:
    break;
  }

  PT(GeomVertexArrayFormat) arr = new GeomVertexArrayFormat;
  arr->add_column(InternalName::get_vertex(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_point);
  arr->add_column(InternalName::get_normal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_normal);
  arr->add_column(InternalName::get_tangent(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
  arr->add_column(InternalName::get_binormal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
  arr->add_column(InternalName::get_texcoord(), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
  arr->add_column(InternalName::get_texcoord_name("lightmap"), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(arr);

  PT(GeomVertexArrayFormat) blend_arr = new GeomVertexArrayFormat(*arr);
  blend_arr->add_column(InternalName::make("blend"), 1, GeomEnums::NT_stdfloat, GeomEnums::C_other);
  CPT(GeomVertexFormat) blend_format = GeomVertexFormat::register_format(blend_arr);

  // Now write out the meshes to GeomNodes.

  for (size_t i = 0; i < _meshes.size(); ++i) {
    MapModel model;

    LPoint3 mmins(9999999);
    LPoint3 mmaxs(-9999999);

    const MapMesh *mesh = _meshes[i];
    MapEntitySrc *ent = _source_map->_entities[mesh->_entity];
    std::string ent_name;
    bool is_world = false;
    if (mesh->_entity == 0) {
      if (mesh->_3d_sky_mesh) {
        ent_name = "3d_skybox";
      } else {
        ent_name = "world";
      }
      is_world = true;
    } else {
      ent_name = ent->_class_name + ":" + ent->_properties["targetname"];
    }

    // Build Geoms out of the mesh's MapPolys.

    std::ostringstream ss;
    ss << "mesh-" << i << "-" << ent_name;
    model._geom_node = new GeomNode(ss.str());
    GeomNode *geom_node = model._geom_node;

    for (MapPoly *poly : mesh->_polys) {
      PT(GeomVertexData) vdata = new GeomVertexData(
        geom_node->get_name(), poly->_blends.empty() ? format : blend_format,
        GeomEnums::UH_static);
      add_poly_to_geom_node(poly, vdata, geom_node);

      for (int j = 0; j < poly->_winding.get_num_points(); ++j) {
        mmins = mmins.fmin(poly->_winding.get_point(j));
        mmaxs = mmaxs.fmax(poly->_winding.get_point(j));
      }
    }

    model._mins = mmins;
    model._maxs = mmaxs;

    // Build physics for model.
    build_entity_physics((int)i, model);

    if (geom_node->get_num_geoms() > 0) {
      _out_top->add_child(geom_node);
    }

    _out_data->add_model(model);
  }

  _out_data->_3d_sky_model = _3d_sky_mesh_index;

  if (mapbuilder_cat.is_debug()) {
    mapbuilder_cat.debug()
      << "Pre flatten graph:\n";
    SceneGraphAnalyzer analyzer;
    analyzer.add_node(_out_top);
    analyzer.write(mapbuilder_cat.debug(false));
  }

  if (_options.get_light()) {
    // Now compute lighting.
    ec = build_lighting();
    if (ec != EC_ok) {
      return ec;
    }

    // Render cube maps.
    ec = render_cube_maps();
    if (ec != EC_ok) {
      return ec;
    }
  }

  build_overlays();

  // After building the lightmaps, we can flatten the Geoms within each mesh
  // group to reduce draw calls.  If we flattened before building lightmaps,
  // Geoms would have overlapping lightmap UVs.
  for (size_t i = 0; i < _out_data->get_num_models(); i++) {
    NodePath(_out_data->get_model(i)->get_geom_node()).flatten_strong();
  }

  if (_options._do_steam_audio) {
    ec = bake_steam_audio();
    if (ec != EC_ok) {
      return ec;
    }
  }

  if (mapbuilder_cat.is_debug()) {
    mapbuilder_cat.debug()
      << "Post flatten graph:\n";
    SceneGraphAnalyzer analyzer;
    analyzer.add_node(_out_top);
    analyzer.write(mapbuilder_cat.debug(false));
  }

  NodePath(_out_top).write_bam_file(_options._output_filename);

  return EC_ok;
}

#ifdef HAVE_STEAM_AUDIO
#ifndef CPPPARSER
void
ipl_log(IPLLogLevel lvl, const char *msg) {
  std::cerr << "IPL: lvl " << lvl << ", msg " << std::string(msg) << "\n";
}
#endif
#endif

class IPLGeomEntry {
public:
  CPT(Geom) geom;
  const Material *mat;
  int mat_index;
};

#ifdef HAVE_STEAM_AUDIO
#ifndef CPPPARSER
void
ipl_progress_callback(IPLfloat32 progress, void *user_data) {
  std::cout << "Progress: " << (int)(progress * 100.0f) << "%\r" << std::flush;
  //std::cerr << "progress: " << progress << "\n";
}
#endif
#endif

/**
 * Bakes Steam Audio information into the map.
 */
MapBuilder::ErrorCode MapBuilder::
bake_steam_audio() {
#ifndef HAVE_STEAM_AUDIO
  return EC_ok;
#else

  IPLContext context = nullptr;
  IPLContextSettings ctx_settings{};
  ctx_settings.version = STEAMAUDIO_VERSION;
  ctx_settings.allocateCallback = nullptr;
  ctx_settings.freeCallback = nullptr;
  ctx_settings.simdLevel = IPL_SIMDLEVEL_AVX2;
  ctx_settings.logCallback = ipl_log;
  IPLerror err = iplContextCreate(&ctx_settings, &context);
  if (err != IPL_STATUS_SUCCESS) {
    return EC_ok;
  }
  //std::cout << err << "\n";
  //std::cout << "Version: " << STEAMAUDIO_VERSION << "\n";
  //assert(err == IPL_STATUS_SUCCESS);


  //IPLEmbreeDeviceSettings embree_set{};
  //IPLEmbreeDevice embree_dev = nullptr;
  //err = iplEmbreeDeviceCreate(context, &embree_set, &embree_dev);
  //assert(err == IPL_STATUS_SUCCESS);

  IPLOpenCLDeviceList ocl_d_list;
  IPLOpenCLDeviceSettings cl_settings{};
  cl_settings.type = IPL_OPENCLDEVICETYPE_GPU;
  cl_settings.requiresTAN = IPL_FALSE;
  cl_settings.numCUsToReserve = 0;
  cl_settings.fractionCUsForIRUpdate = 0.0f;
  err = iplOpenCLDeviceListCreate(context, &cl_settings, &ocl_d_list);
  assert(err == IPL_STATUS_SUCCESS);
  assert(iplOpenCLDeviceListGetNumDevices(ocl_d_list) != 0);
  IPLOpenCLDevice ocl_dev;
  err = iplOpenCLDeviceCreate(context, ocl_d_list, 0, &ocl_dev);
  assert(err == IPL_STATUS_SUCCESS);

  iplOpenCLDeviceListRelease(&ocl_d_list);

  IPLRadeonRaysDeviceSettings rr_settings{};
  IPLRadeonRaysDevice rr_dev;
  err = iplRadeonRaysDeviceCreate(ocl_dev, &rr_settings, &rr_dev);
  assert(err == IPL_STATUS_SUCCESS);

  IPLScene scene = nullptr;
  IPLSceneSettings scene_settings{};
  memset(&scene_settings, 0, sizeof(IPLSceneSettings));
  scene_settings.type = IPL_SCENETYPE_RADEONRAYS;
  scene_settings.radeonRaysDevice = rr_dev;
  err = iplSceneCreate(context, &scene_settings, &scene);
  assert(err == IPL_STATUS_SUCCESS);

  pmap<std::string, IPLMaterial> surface_props;
  surface_props["default"] = {0.10f,0.20f,0.30f,0.05f,0.100f,0.050f,0.030f};
  surface_props["wood"] = {0.11f,0.07f,0.06f,0.05f,0.070f,0.014f,0.005f};
  surface_props["metal"] = {0.20f,0.07f,0.06f,0.05f,0.200f,0.025f,0.010f};
  surface_props["brick"] = {0.03f,0.04f,0.07f,0.05f,0.015f,0.015f,0.015f};
  surface_props["concrete"] = {0.05f,0.07f,0.08f,0.05f,0.015f,0.002f,0.001f};
  surface_props["gravel"] = {0.60f,0.70f,0.80f,0.05f,0.031f,0.012f,0.008f};
  surface_props["rock"] = {0.13f,0.20f,0.24f,0.05f,0.015f,0.002f,0.001f};
  surface_props["carpet"] = {0.24f,0.69f,0.73f,0.05f,0.020f,0.005f,0.003f};
  surface_props["plaster"] = {0.12f,0.06f,0.04f,0.05f,0.056f,0.056f,0.004f};
  surface_props["sky"] = {1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};
  surface_props["default_silent"] = {1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};

  // Build up a huge vector of all Geoms in the entire scene.  World geometry
  // and static props.
  pvector<IPLGeomEntry> geoms;

  pvector<IPLMaterial> materials;
  pmap<std::string, int> material_indices;

  // Start with static world geometry.
  const MapModel *world_model = _out_data->get_model(0);
  //for (int i = 0; i < _out_node->get_num_children(); i++) {
    GeomNode *child = world_model->get_geom_node();
    for (int j = 0; j < child->get_num_geoms(); j++) {
      const RenderState *state = child->get_geom_state(j);
      const MaterialAttrib *mattr;
      state->get_attrib_def(mattr);
      IPLGeomEntry entry;
      entry.geom = child->get_geom(j);
      entry.mat = mattr->get_material();

      // get this from the pmat file tags.
      std::string surfaceprop = "default";
      if (mattr->get_material() != nullptr) {
        //if (mattr->get_material()->has_tag("compile_sky")) {
          // Skip sky polygons/geoms.
        //  continue;
        //}
        if (mattr->get_material()->has_tag("surface_prop")) {
          surfaceprop = mattr->get_material()->get_tag_value("surface_prop");
          if (surface_props.find(surfaceprop) == surface_props.end()) {
            surfaceprop = "default";
          }
        }
      }
      int mat_index;
      auto it = material_indices.find(surfaceprop);
      if (it == material_indices.end()) {
        mat_index = (int)materials.size();
        materials.push_back(surface_props[surfaceprop]);
        material_indices[surfaceprop] = mat_index;

      } else {
        mat_index = it->second;
      }
      entry.mat_index = mat_index;

      geoms.push_back(entry);
    }
  //}

  // Now get static props.
  for (int i = 0; i < _out_data->get_num_static_props(); i++) {
    const MapStaticProp *sprop = _out_data->get_static_prop(i);
    PT(PandaNode) prop_model_node = Loader::get_global_ptr()->load_sync(sprop->get_model_filename());
    if (prop_model_node == nullptr) {
      continue;
    }
    NodePath prop_model(prop_model_node);
    prop_model.set_pos(sprop->get_pos());
    prop_model.set_hpr(sprop->get_hpr());

    std::string surfaceprop = "default";
    ModelRoot *mroot = DCAST(ModelRoot, prop_model_node);
    PDXElement *cdata = mroot->get_custom_data();
    if (cdata != nullptr) {
      // Check for a surface prop.
      if (cdata->has_attribute("surfaceprop")) {
        surfaceprop = cdata->get_attribute_value("surfaceprop").get_string();
        if (surface_props.find(surfaceprop) == surface_props.end()) {
          surfaceprop = "default";
        }
      }
    }

    int mat_index;
    auto it = material_indices.find(surfaceprop);
    if (it == material_indices.end()) {
      mat_index = (int)materials.size();
      materials.push_back(surface_props[surfaceprop]);
      material_indices[surfaceprop] = mat_index;

    } else {
      mat_index = it->second;
    }

    // Move transforms and attribs down to vertices.
    prop_model.flatten_light();

    NodePathCollection geom_nodes;
    // Get all the Geoms and associated materials.
    // If there's an LOD, only get Geoms from the lowest LOD level.
    NodePath lod = prop_model.find("**/+LODNode");
    if (!lod.is_empty()) {
      NodePath lowest_lod = lod.get_child(lod.get_num_children() - 1);
      if (lowest_lod.node()->is_geom_node()) {
        geom_nodes.add_path(lowest_lod);
      }
      geom_nodes.add_paths_from(lowest_lod.find_all_matches("**/+GeomNode"));

    } else {
      // Otherwise get all the Geoms.
      geom_nodes = prop_model.find_all_matches("**/+GeomNode");
    }

    for (int j = 0; j < geom_nodes.get_num_paths(); j++) {
      NodePath geom_np = geom_nodes.get_path(j);
      GeomNode *geom_node = (GeomNode *)geom_np.node();
      for (int k = 0; k < geom_node->get_num_geoms(); k++) {
        const RenderState *state = geom_node->get_geom_state(k);
        const MaterialAttrib *mattr;
        state->get_attrib_def(mattr);
        IPLGeomEntry entry;
        entry.geom = geom_node->get_geom(k);
        entry.mat = mattr->get_material();
        entry.mat_index = mat_index;
        geoms.push_back(entry);
      }
    }
  }

  mapbuilder_cat.info()
    << "Building IPL static mesh\n";

  // We've got the Geoms.  Now build up triangle lists.
  pvector<IPLVector3> verts;
  pvector<IPLTriangle> tris;
  pvector<IPLint32> tri_materials;

  pmap<LPoint3, size_t> vert_indices;

  size_t geom_count = geoms.size();

  mapbuilder_cat.info()
    << materials.size() << " unique IPL materials\n";

  for (size_t i = 0; i < geom_count; i++) {
    PT(Geom) dgeom = geoms[i].geom->decompose();

    GeomVertexReader reader(dgeom->get_vertex_data(), InternalName::get_vertex());

    for (size_t j = 0; j < dgeom->get_num_primitives(); j++) {
      const GeomPrimitive *prim = dgeom->get_primitive(j);
      for (size_t k = 0; k < prim->get_num_primitives(); k++) {
        size_t start = prim->get_primitive_start(k);

        IPLTriangle tri;
        for (size_t l = 0; l < 3; l++) {
          size_t v = start + l;
          int vtx = prim->get_vertex(v);
          reader.set_row(vtx);
          LPoint3 pos = reader.get_data3f();

          size_t ipl_index;
          auto it = vert_indices.find(pos);
          if (it != vert_indices.end()) {
            ipl_index = it->second;
          } else {
            ipl_index = verts.size();
            // Go from inches to meters.
            verts.push_back({ pos[0] * HAMMER_UNITS_TO_METERS, pos[2] * HAMMER_UNITS_TO_METERS, -pos[1] * HAMMER_UNITS_TO_METERS });
            vert_indices[pos] = ipl_index;
          }

          tri.indices[l] = ipl_index;
        }
        tris.push_back(tri);
        tri_materials.push_back(geoms[i].mat_index);
      }
    }
  }

  IPLStaticMesh static_mesh = nullptr;
  IPLStaticMeshSettings mesh_settings{};
  mesh_settings.materials = materials.data();
  mesh_settings.numMaterials = materials.size();
  mesh_settings.vertices = verts.data();
  mesh_settings.numVertices = verts.size();
  mesh_settings.triangles = tris.data();
  mesh_settings.materialIndices = tri_materials.data();
  mesh_settings.numTriangles = tris.size();
  err = iplStaticMeshCreate(scene, &mesh_settings, &static_mesh);
  assert(err == IPL_STATUS_SUCCESS);
  iplStaticMeshAdd(static_mesh, scene);
  iplSceneCommit(scene);

  if (_options._do_steam_audio_pathing || _options._do_steam_audio_reflections) {
    IPLProbeBatch batch = nullptr;
    iplProbeBatchCreate(context, &batch);

    int num_probes = 0;

    if (_options._do_vis != MapBuildOptions::VT_bsp) {
      // Start at the lowest corner of the level bounds and work our way to the top.
      for (PN_stdfloat z = _scene_mins[2]; z <= _scene_maxs[2]; z += 256.0f) {
        for (PN_stdfloat y = _scene_mins[1]; y <= _scene_maxs[1]; y += 256.0f) {
          for (PN_stdfloat x = _scene_mins[0]; x <= _scene_maxs[0]; x += 256.0f) {
            LPoint3 pos(x, y, z);
            if (_out_data->get_area_cluster_tree()->get_leaf_value_from_point(pos) == -1) {
              // Probe is not in valid cluster.  Skip it.
              continue;
            }

            IPLSphere sphere;
            sphere.center.x = pos[0] * HAMMER_UNITS_TO_METERS;
            sphere.center.y = pos[2] * HAMMER_UNITS_TO_METERS;
            sphere.center.z = -pos[1] * HAMMER_UNITS_TO_METERS;
            sphere.radius = 10.0f;
            iplProbeBatchAddProbe(batch, sphere);
            num_probes++;
          }
        }
      }
    } else {
      // If we computed BSP visibility, we can place a probe at the center of
      // each leaf.

      mapbuilder_cat.info()
        << "Generating probes from BSP tree...\n";

      const BSPTree *tree = (const BSPTree *)_out_data->get_area_cluster_tree();

      pvector<LPoint3> probe_positions;
      pvector<float> probe_radii;
      for (size_t i = 0; i < tree->_leaves.size(); ++i) {
        if (tree->_leaves[i].solid || tree->_leaves[i].value < 0) {
          // Don't place a probe in solid leaves.
          continue;
        }

        // Gather the planes of all parent nodes of the leaf.
        pvector<LPlane> boundary_planes;
        pvector<Winding> boundary_windings;
        int node_idx = tree->_leaf_parents[i];
        int child = ~((int)i);
        while (node_idx >= 0) {
          LPlane plane = tree->_nodes[node_idx].plane;
          if (tree->_nodes[node_idx].children[BACK_CHILD] == child) {
            // Back side.
            plane.flip();
          }
          boundary_planes.push_back(plane);
          boundary_windings.push_back(Winding(plane));
          child = node_idx;
          node_idx = tree->_node_parents[node_idx];
        }

        // Intersect all planes to get windings for the leaf.
        for (size_t j = 0; j < boundary_windings.size(); ++j) {
          for (size_t k = 0; k < boundary_planes.size(); ++k) {
            if (k == j) {
              continue;
            }
            // Flip the plane because we want to keep the back-side.
            boundary_windings[j] = boundary_windings[j].chop(boundary_planes[k]);
          }
        }

        // Average all winding vertex positions to get leaf center.
        LPoint3 leaf_center(0.0f);
        int total_points = 0;
        for (size_t j = 0; j < boundary_windings.size(); ++j) {
          for (int k = 0; k < boundary_windings[j].get_num_points(); ++k) {
            leaf_center += boundary_windings[j].get_point(k);
            total_points++;
          }
        }
        leaf_center /= total_points;

        // Now determine radius.
        float radius = 0.0f;
        for (size_t j = 0; j < boundary_windings.size(); ++j) {
          for (int k = 0; k < boundary_windings[j].get_num_points(); ++k) {
            radius = std::max(radius, (boundary_windings[j].get_point(k) - leaf_center).length());
          }
        }

        probe_positions.push_back(leaf_center);
        probe_radii.push_back(radius);
      }

      //// Also place probes at center of each portal.
      //for (const LPoint3 &portal_center : _portal_centers) {
      //  probe_positions.push_back(portal_center);
      //}

      for (size_t i = 0; i < probe_positions.size(); ++i) {
        LPoint3 center = probe_positions[i];
        float radius = probe_radii[i];

        // Place probe here.
        IPLSphere sphere;
        sphere.center.x = center[0] * HAMMER_UNITS_TO_METERS;
        sphere.center.y = center[2] * HAMMER_UNITS_TO_METERS;
        sphere.center.z = -center[1] * HAMMER_UNITS_TO_METERS;
        sphere.radius = radius * HAMMER_UNITS_TO_METERS;
        iplProbeBatchAddProbe(batch, sphere);
        num_probes++;
      }
    }

    mapbuilder_cat.info()
      << num_probes << " audio probes\n";

    iplProbeBatchCommit(batch);

    if (_options._do_steam_audio_reflections) {
      mapbuilder_cat.info()
        << "Baking listener-centric reverb\n";

      IPLBakedDataIdentifier identifier{};
      identifier.type = IPL_BAKEDDATATYPE_REFLECTIONS;
      identifier.variation = IPL_BAKEDDATAVARIATION_REVERB;
      IPLReflectionsBakeParams bake_params{};
      bake_params.scene = scene;
      bake_params.sceneType = IPL_SCENETYPE_RADEONRAYS;
      bake_params.identifier = identifier;
      int flags = IPL_REFLECTIONSBAKEFLAGS_BAKECONVOLUTION;
      bake_params.bakeFlags = (IPLReflectionsBakeFlags)flags;
      bake_params.probeBatch = batch;
      bake_params.numRays = 32768;
      bake_params.numDiffuseSamples = 2048;
      bake_params.numBounces = 100;
      bake_params.simulatedDuration = 1.0f;
      bake_params.savedDuration = 1.0f;
      bake_params.order = 2;
      bake_params.numThreads = _options.get_num_threads();
      bake_params.irradianceMinDistance = 1.0f;
      bake_params.rayBatchSize = 1;
      bake_params.bakeBatchSize = 64;
      bake_params.openCLDevice = ocl_dev;
      bake_params.radeonRaysDevice = rr_dev;
      iplReflectionsBakerBake(context, &bake_params, ipl_progress_callback, nullptr);
    }

    if (_options._do_steam_audio_pathing) {
      mapbuilder_cat.info()
        << "Baking audio pathing\n";

      IPLBakedDataIdentifier identifier{};
      identifier.type = IPL_BAKEDDATATYPE_PATHING;
      identifier.variation = IPL_BAKEDDATAVARIATION_DYNAMIC;
      IPLPathBakeParams path_params{};
      path_params.scene = scene;
      path_params.identifier = identifier;
      path_params.numThreads = _options.get_num_threads();
      path_params.pathRange = 100.0f;
      path_params.visRange = 50.0f;
      path_params.probeBatch = batch;
      path_params.numSamples = 32;
      path_params.radius = 2.0f;
      path_params.threshold = 0.05f;
      iplPathBakerBake(context, &path_params, nullptr, nullptr);
    }

    // Serialize the probe batch.
    IPLSerializedObjectSettings probe_so_settings{};
    IPLSerializedObject batch_obj = nullptr;
    err = iplSerializedObjectCreate(context, &probe_so_settings, &batch_obj);
    assert(err == IPL_STATUS_SUCCESS);
    iplProbeBatchSave(batch, batch_obj);

    // Chuck it into the MapData.
    PTA_uchar batch_data;
    batch_data.resize(iplSerializedObjectGetSize(batch_obj));
    memcpy(batch_data.p(), iplSerializedObjectGetData(batch_obj), batch_data.size());
    _out_data->_steam_audio_probe_data = batch_data;
    mapbuilder_cat.info()
      << "IPL refl probe data size: " << batch_data.size() << " bytes\n";

    iplProbeBatchRelease(&batch);
    iplSerializedObjectRelease(&batch_obj);
  }

  // Chuck it into the MapData.
  PTA_uchar pverts, ptris, ptri_materials, pmaterials;
  pverts.resize(verts.size() * sizeof(IPLVector3));
  ptris.resize(tris.size() * sizeof(IPLTriangle));
  pmaterials.resize(materials.size() * sizeof(IPLMaterial));
  ptri_materials.resize(tri_materials.size() * sizeof(IPLint32));
  memcpy(pverts.p(), (unsigned char *)verts.data(), pverts.size());
  memcpy(ptris.p(), (unsigned char *)tris.data(), ptris.size());
  memcpy(ptri_materials.p(), (unsigned char *)tri_materials.data(), ptri_materials.size());
  memcpy(pmaterials.p(), (unsigned char *)materials.data(), pmaterials.size());
  _out_data->_steam_audio_scene_data.verts = pverts;
  _out_data->_steam_audio_scene_data.tris = ptris;
  _out_data->_steam_audio_scene_data.tri_materials = ptri_materials;
  _out_data->_steam_audio_scene_data.materials = pmaterials;
  mapbuilder_cat.info()
    << "IPL scene data size: " << pverts.size() + ptris.size() + ptri_materials.size() + pmaterials.size() << " bytes\n";

  // Clean up our work.
  iplStaticMeshRelease(&static_mesh);
  iplSceneRelease(&scene);
  //iplEmbreeDeviceRelease(&embree_dev);
  iplRadeonRaysDeviceRelease(&rr_dev);
  iplOpenCLDeviceRelease(&ocl_dev);
  iplContextRelease(&context);

  return EC_ok;
#endif
}

/**
 * Creates a Geom and RenderState for the indicated MapPoly and adds it to the
 * indicated GeomNode.
 */
void MapBuilder::
add_poly_to_geom_node(MapPoly *poly, GeomVertexData *vdata, GeomNode *geom_node) {
  if (!poly->_visible) {
    // Polygon was determined to be in solid space by the BSP preprocessor.
    // Don't write a Geom for it.
    return;
  }

  int start = vdata->get_num_rows();

  GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
  GeomVertexWriter nwriter(vdata, InternalName::get_normal());
  GeomVertexWriter twriter(vdata, InternalName::get_texcoord());
  GeomVertexWriter lwriter(vdata, InternalName::get_texcoord_name("lightmap"));
  GeomVertexWriter bwriter(vdata, InternalName::make("blend"));
  GeomVertexWriter tanwriter(vdata, InternalName::get_tangent());
  GeomVertexWriter binwriter(vdata, InternalName::get_binormal());
  vwriter.set_row(start);
  nwriter.set_row(start);
  twriter.set_row(start);
  lwriter.set_row(start);
  bwriter.set_row(start);
  tanwriter.set_row(start);
  binwriter.set_row(start);

  const Winding *w = &(poly->_winding);

  Material *mat = poly->_material;

  // Fill up the render state for the polygon.
  CPT(RenderState) state = RenderState::make_empty();
  if (mat != nullptr) {
    if (mat->has_tag("compile_clip") ||
        //mat->has_tag("compile_trigger") ||
        mat->has_tag("compile_nodraw")) {
      // Skip these for rendering.  We still added the physics mesh above.
      return;
    }

    state = state->set_attrib(MaterialAttrib::make(mat));

    if (mat->has_tag("compile_sky")) {
      // Sky needs to render first and not write depth.
      state = state->set_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
      state = state->set_attrib(CullBinAttrib::make("background", 0));
    }
  }

  for (size_t k = 0; k < w->get_num_points(); k++) {
    LPoint3 point = w->get_point(k);
    vwriter.add_data3f(point);
    LPoint3 normal = poly->_normals[k].normalized();
    nwriter.add_data3f(normal);
    twriter.add_data2f(poly->_uvs[k]);
    lwriter.add_data2f(poly->_lightmap_uvs[k]);
    if (bwriter.has_column()) {
      bwriter.add_data1f(poly->_blends[k]);
    }

    // Calculate tangent and binormal from the normal.
    LVector3 x;
    if (cabs(normal[0]) >= cabs(normal[1]) && cabs(normal[0]) >= cabs(normal[2])) {
      x = LVector3::unit_x();
    } else if (cabs(normal[1]) >= cabs(normal[2])) {
      x = LVector3::unit_y();
    } else {
      x = LVector3::unit_z();
    }
    LVector3 v0 = (x == LVector3::unit_z()) ? LVector3::unit_x() : LVector3::unit_z();
    LVector3 tangent = v0.cross(normal).normalized();
    LVector3 binormal = tangent.cross(normal).normalized();
    tanwriter.add_data3f(tangent);
    binwriter.add_data3f(binormal);
  }

  PT(GeomTriangles) tris = new GeomTriangles(GeomEnums::UH_static);
  for (size_t k = 1; k < (w->get_num_points() - 1); k++) {
    tris->add_vertices(start + k + 1, start + k, start);
    tris->close_primitive();
  }

  // Keep track of this for when we compute lightmaps.
  poly->_geom_index = geom_node->get_num_geoms();
  poly->_geom_node = geom_node;

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(tris);
  geom_node->add_geom(geom, state);
}

/**
 * Builds a polygon soup from the convex solids and displacement surfaces in
 * the map.
 */
MapBuilder::ErrorCode MapBuilder::
build_polygons() {
  _world_mesh_index = -1;
  //for (size_t i = 0; i < _source_map->_entities.size(); i++) {
  //  build_entity_polygons(i);
  //}

  ThreadManager::run_threads_on_individual(
    "BuildPolygons", _source_map->_entities.size(),
    false, std::bind(&MapBuilder::build_entity_polygons, this, std::placeholders::_1));
  return EC_ok;
}

/**
 * List of neighboring MapPolys within the angle threshold that share a vertex
 * position.
 */
class PolyVertRef {
public:
  MapPoly *poly;
  int vertex;
  LVector3 normal;
};
typedef pvector<PolyVertRef> PolyVertGroup;
typedef pmap<LPoint3, PolyVertGroup> PolyVertCollection;

/**
 *
 */
void MapBuilder::
build_entity_polygons(int i) {
  MapEntitySrc *ent = _source_map->_entities[i];

  if (//ent->_class_name == "func_door" ||
      ent->_class_name == "func_respawnroomvisualizer") {
    // TEMPORARY
    return;
  }

  if (ent->_solids.size() == 0) {
    return;
  }

  PT(MapMesh) ent_mesh = new MapMesh;
  ent_mesh->_in_group = false;
  ent_mesh->_is_mesh = true;
  ent_mesh->_entity = i;
  LPoint3 minp, maxp;
  minp.set(1e+9, 1e+9, 1e+9);
  maxp.set(-1e+9, -1e+9, -1e+9);

  for (size_t j = 0; j < ent->_solids.size(); j++) {
    MapSolid *solid = ent->_solids[j];

    bool bad_solid = false;

    pvector<PT(MapPoly)> solid_polys;

    bool solid_has_disp_sides = false;
    for (size_t k = 0; k < solid->_sides.size(); k++) {
      MapSide *side = solid->_sides[k];
      if (side->_displacement != nullptr) {
        solid_has_disp_sides = true;
        break;
      }
    }

    for (size_t k = 0; k < solid->_sides.size() && !bad_solid; k++) {
      MapSide *side = solid->_sides[k];

      if (solid_has_disp_sides && side->_displacement == nullptr) {
        // If the solid has a displacement side, all other sides that aren't
        // also displacement sides are ignored.
        continue;
      }

      // Start with a gigantic winding from the side's plane.
      Winding w(side->_plane);

      // Then iteratively chop the winding by the planes of all other sides.
      for (size_t l = 0; l < solid->_sides.size(); l++) {
        if (l == k) {
          continue;
        }
        MapSide *other_side = solid->_sides[l];
        Winding chopped = w.chop(-other_side->_plane);
        if (chopped.is_empty()) {
          mapbuilder_cat.error()
            << "Bad winding chop solid " << solid->_editor_id << " side " << side->_editor_id << " against side " << other_side->_editor_id << "\n";
          mapbuilder_cat.error()
            << "Side plane " << side->_plane << " chop plane " << other_side->_plane << "\n";
          for (size_t m = 0; m < w.get_num_points(); m++) {
            mapbuilder_cat.error()
              << w.get_point(m) << "\n";
          }
          w.clear();
          break;
        }
        w = chopped;
      }

      if (w.is_empty()) {
        bad_solid = true;
        break;
      }

      w.round_points();

      // We now have the final polygon for the side.

      Filename material_filename = Filename("materials/" + downcase(side->_material_filename.get_fullpath_wo_extension()) + ".mto");

      PT(Material) poly_material = MaterialPool::load_material(material_filename);

      if (poly_material != nullptr &&
          (poly_material->has_tag("compile_hint") ||
            poly_material->has_tag("compile_skip") ||
            poly_material->has_tag("compile_areaportal"))) {
        continue;
      }

      for (size_t l = 0; l < w.get_num_points(); l++) {
        LPoint3 p = w.get_point(l);
        minp[0] = std::min(p[0], minp[0]);
        minp[1] = std::min(p[1], minp[1]);
        minp[2] = std::min(p[2], minp[2]);
        maxp[0] = std::max(p[0], maxp[0]);
        maxp[1] = std::max(p[1], maxp[1]);
        maxp[2] = std::max(p[2], maxp[2]);
      }

      // Extract texture dimensions.
      PT(Texture) base_tex;
      LVecBase2i tex_dim(1, 1);
      if (poly_material != nullptr) {
        MaterialParamBase *base_color_p = poly_material->get_param("base_color");
        if (base_color_p != nullptr && base_color_p->is_of_type(MaterialParamTexture::get_class_type())) {
          base_tex = ((MaterialParamTexture *)base_color_p)->get_value();
          if (base_tex != nullptr) {
            tex_dim[0] = base_tex->get_x_size();
            tex_dim[1] = base_tex->get_y_size();
          }
        }
      }

      LPoint3 origin(0);

      // Calculate texture vectors.
      LVector4 texture_vecs[2];
      texture_vecs[0][0] = side->_u_axis[0] / side->_uv_scale[0];
      texture_vecs[0][1] = side->_u_axis[1] / side->_uv_scale[0];
      texture_vecs[0][2] = side->_u_axis[2] / side->_uv_scale[0];
      texture_vecs[0][3] = side->_uv_shift[0] + origin.dot(texture_vecs[0].get_xyz());
      texture_vecs[1][0] = side->_v_axis[0] / side->_uv_scale[1];
      texture_vecs[1][1] = side->_v_axis[1] / side->_uv_scale[1];
      texture_vecs[1][2] = side->_v_axis[2] / side->_uv_scale[1];
      texture_vecs[1][3] = side->_uv_shift[1] + origin.dot(texture_vecs[1].get_xyz());

      // Calculate lightmap vectors.
      // Twice the resolution for the GPU lightmapper, but no smaller than 1 unit
      // per luxel.
      PN_stdfloat lightmap_scale = std::max(1.0f, side->_lightmap_scale * 0.5f);
      LVector4 lightmap_vecs[2];
      lightmap_vecs[0][0] = side->_u_axis[0] / lightmap_scale;
      lightmap_vecs[0][1] = side->_u_axis[1] / lightmap_scale;
      lightmap_vecs[0][2] = side->_u_axis[2] / lightmap_scale;
      lightmap_vecs[1][0] = side->_v_axis[0] / lightmap_scale;
      lightmap_vecs[1][1] = side->_v_axis[1] / lightmap_scale;
      lightmap_vecs[1][2] = side->_v_axis[2] / lightmap_scale;
      PN_stdfloat shift_scale_u = side->_uv_scale[0] / lightmap_scale;
      PN_stdfloat shift_scale_v = side->_uv_scale[1] / lightmap_scale;
      lightmap_vecs[0][3] = shift_scale_u * side->_uv_shift[0] + origin.dot(lightmap_vecs[0].get_xyz());
      lightmap_vecs[1][3] = shift_scale_v * side->_uv_shift[1] + origin.dot(lightmap_vecs[1].get_xyz());

      if (side->_displacement == nullptr) {
        // A regular non-displacement brush face.
        PT(MapPoly) poly = new MapPoly;
        poly->_visible = true;
        poly->_side_id = side->_editor_id;
        poly->_winding = w;
        poly->_in_group = false;
        poly->_is_mesh = false;
        LPoint3 polymin, polymax;
        w.get_bounds(polymin, polymax);
        poly->_bounds = new BoundingBox(polymin, polymax);
        poly->_material = poly_material;
        poly->_base_tex = base_tex;

        _side_polys[poly->_side_id].push_back(poly);

        LVector3 winding_normal = w.get_plane().get_normal().normalized();
        for (size_t ivert = 0; ivert < w.get_num_points(); ivert++) {
          poly->_normals.push_back(winding_normal);
        }

        poly->_vis_occluder = true;

        if (side->_displacement != nullptr) {
          poly->_vis_occluder = false;

        } else if (base_tex != nullptr && Texture::has_alpha(base_tex->get_format())) {
          poly->_vis_occluder = false;

        } else if (poly_material != nullptr &&
                   (poly_material->has_tag("compile_clip") ||
                    poly_material->has_tag("compile_trigger"))) {
          poly->_vis_occluder = false;
        }

        for (size_t ivert = 0; ivert < w.get_num_points(); ivert++) {
          const LPoint3 &point = w.get_point(ivert);
          LVecBase2 uv(
            texture_vecs[0].get_xyz().dot(point) + texture_vecs[0][3],
            texture_vecs[1].get_xyz().dot(point) + texture_vecs[1][3]
          );
          uv[0] /= tex_dim[0];
          uv[1] /= -tex_dim[1];
          poly->_uvs.push_back(uv);
        }

        // Calc lightmap size and mins.
        LVecBase2 lmins(1e24);
        LVecBase2 lmaxs(-1e24);

        LVecBase2i lightmap_mins;

        for (int ivert = 0; ivert < w.get_num_points(); ivert++) {
          LPoint3 wpt = w.get_point(ivert);
          for (int l = 0; l < 2; l++) {
            PN_stdfloat val = wpt[0] * lightmap_vecs[l][0] +
                              wpt[1] * lightmap_vecs[l][1] +
                              wpt[2] * lightmap_vecs[l][2] +
                              lightmap_vecs[l][3];
            lmins[l] = std::min(val, lmins[l]);
            lmaxs[l] = std::max(val, lmaxs[l]);
          }
        }

        //std::cout << "Face\n";

        for (int l = 0; l < 2; l++) {
          lmins[l] = std::floor(lmins[l]);
          lmaxs[l] = std::ceil(lmaxs[l]);
          lightmap_mins[l] = (int)lmins[l];
          poly->_lightmap_size[l] = (int)(lmaxs[l] - lmins[l]);
        }

        //std::cout << "lmins " << lightmap_mins << ", extents " << poly->_lightmap_size << "\n";

        for (size_t ivert = 0; ivert < w.get_num_points(); ivert++) {
          const LPoint3 &point = w.get_point(ivert);
          LVecBase2 lightcoord;
          lightcoord[0] = point.dot(lightmap_vecs[0].get_xyz()) + lightmap_vecs[0][3];
          lightcoord[0] -= lmins[0];
          //lightcoord[0] += 0.5;
          lightcoord[0] /= poly->_lightmap_size[0];

          lightcoord[1] = point.dot(lightmap_vecs[1].get_xyz()) + lightmap_vecs[1][3];
          lightcoord[1] -= lmins[1];
          //lightcoord[1] += 0.5;
          lightcoord[1] /= poly->_lightmap_size[1];

          poly->_lightmap_uvs.push_back(lightcoord);
          //std::cout << "luv " << lightcoord << "\n";
        }

        solid_polys.push_back(poly);

      } else {
        // This is a displacement brush face.  Build up a set of MapPolys
        // for each displacement triangle.

        int start_index = w.get_closest_point(side->_displacement->_start_position);
        int ul = start_index;
        int ur = (start_index + 3) % w.get_num_points();
        int lr = (start_index + 2) % w.get_num_points();
        int ll = (start_index + 1) % w.get_num_points();

        LVector3 winding_normal = w.get_plane().get_normal().normalized();

        //LVector3 ad = w.get_point((start_index + 3) % w.get_num_points()) - w.get_point(start_index);
        //LVector3 ab = w.get_point((start_index + 1) % w.get_num_points()) - w.get_point(start_index);

        pvector<LPoint3> disp_points;
        pvector<LVector3> disp_normals;
        pvector<LVecBase2> disp_uvs;
        pvector<LVecBase2> disp_lightmap_uvs;
        vector_stdfloat disp_blends;

        size_t num_rows = side->_displacement->_rows.size();
        size_t num_cols = side->_displacement->_rows[0]._vertices.size();

        // Collect all displacement vertex data.
        for (size_t irow = 0; irow < num_rows; irow++) {
          for (size_t icol = 0; icol < num_cols; icol++) {
            const MapDisplacementVertex &dvert = side->_displacement->_rows[irow]._vertices[icol];

            disp_normals.push_back(winding_normal);

            disp_blends.push_back(dvert._alpha);

            PN_stdfloat ooint = 1.0f / (PN_stdfloat)(num_rows - 1);

            LPoint3 end_pts[2];
            end_pts[0] = (w.get_point(ul) * (1.0f - irow * ooint)) + (w.get_point(ll) * irow * ooint);
            end_pts[1] = (w.get_point(ur) * (1.0f - irow * ooint)) + (w.get_point(lr) * irow * ooint);

            LPoint3 dpoint = (end_pts[0] * (1.0f - icol * ooint)) + (end_pts[1] * icol * ooint);
            dpoint += winding_normal * side->_displacement->_elevation;
            dpoint += dvert._normal * dvert._distance;
            LVector3 offset = dvert._offset;
            offset.componentwise_mult(dvert._offset_normal);
            dpoint += offset;

            disp_points.push_back(dpoint);

            LVecBase2 duv(
              texture_vecs[0].get_xyz().dot(dpoint) + texture_vecs[0][3],
              texture_vecs[1].get_xyz().dot(dpoint) + texture_vecs[1][3]
            );
            duv[0] /= tex_dim[0];
            duv[1] /= -tex_dim[1];
            disp_uvs.push_back(duv);
          }
        }

        // Now build a MapPoly for each displacement triangle.

        for (size_t irow = 0; irow < num_rows - 1; irow++) {
          for (size_t icol = 0; icol < num_cols - 1; icol++) {

            std::pair<size_t, size_t> tri_verts[2][3];
            if (irow % 2 == icol % 2) {
              tri_verts[0][2] = { irow + 1, icol };
              tri_verts[0][1] = { irow, icol };
              tri_verts[0][0] = { irow + 1, icol + 1};

              tri_verts[1][2] = { irow, icol };
              tri_verts[1][1] = { irow, icol + 1 };
              tri_verts[1][0] = { irow + 1, icol + 1 };

            } else {
              tri_verts[0][2] = { irow + 1, icol };
              tri_verts[0][1] = { irow, icol };
              tri_verts[0][0] = { irow, icol + 1 };

              tri_verts[1][2] = { irow + 1, icol };
              tri_verts[1][1] = { irow, icol + 1 };
              tri_verts[1][0] = { irow + 1, icol + 1 };
            }

            LVecBase2 lmins(1e24), lmaxs(-1e24);
            LVecBase2i lightmap_mins, lightmap_size;
            //
            // TRIANGLE 1
            //
            PT(MapPoly) tri0 = new MapPoly;
            tri0->_visible = true;
            tri0->_side_id = side->_editor_id;
            tri0->_vis_occluder = false;
            tri0->_in_group = false;
            tri0->_is_mesh = false;
            LPoint3 p0 = disp_points[(tri_verts[0][0].first * num_cols) + tri_verts[0][0].second];
            LPoint3 p1 = disp_points[(tri_verts[0][1].first * num_cols) + tri_verts[0][1].second];
            LPoint3 p2 = disp_points[(tri_verts[0][2].first * num_cols) + tri_verts[0][2].second];
            LVector3 tri_normal = ((p1 - p0).normalized().cross(p2 - p0).normalized()).normalized();
            for (size_t ivert = 0; ivert < 3; ivert++) {
              size_t row = tri_verts[0][ivert].first;
              size_t col = tri_verts[0][ivert].second;
              size_t dvertindex = row * num_cols;
              dvertindex += col;
              const LPoint3 &dpoint = disp_points[dvertindex];
              tri0->_winding.add_point(dpoint);
              tri0->_normals.push_back(-tri_normal);
              tri0->_uvs.push_back(disp_uvs[dvertindex]);
              tri0->_blends.push_back(disp_blends[dvertindex]);

              for (int l = 0; l < 2; l++) {
                PN_stdfloat val = dpoint[0] * lightmap_vecs[l][0] +
                                  dpoint[1] * lightmap_vecs[l][1] +
                                  dpoint[2] * lightmap_vecs[l][2] +
                                  lightmap_vecs[l][3];
                lmins[l] = std::min(val, lmins[l]);
                lmaxs[l] = std::max(val, lmaxs[l]);
              }
            }
            for (int l = 0; l < 2; l++) {
              lmins[l] = std::floor(lmins[l]);
              lmaxs[l] = std::ceil(lmaxs[l]);
              lightmap_mins[l] = (int)lmins[l];
              lightmap_size[l] = (int)(lmaxs[l] - lmins[l]);
            }

            tri0->_lightmap_size = lightmap_size;

            for (size_t ivert = 0; ivert < 3; ivert++) {
              const LPoint3 &dpoint = tri0->_winding.get_point(ivert);
              LVecBase2 lightcoord;
              lightcoord[0] = dpoint.dot(lightmap_vecs[0].get_xyz()) + lightmap_vecs[0][3];
              lightcoord[0] -= lmins[0];
              //lightcoord[0] += 0.5;
              lightcoord[0] /= tri0->_lightmap_size[0];// + 1;

              lightcoord[1] = dpoint.dot(lightmap_vecs[1].get_xyz()) + lightmap_vecs[1][3];
              lightcoord[1] -= lmins[1];
              //lightcoord[1] += 0.5;
              lightcoord[1] /= tri0->_lightmap_size[1];// + 1;

              tri0->_lightmap_uvs.push_back(lightcoord);
            }
            tri0->_material = poly_material;
            tri0->_base_tex = base_tex;
            LPoint3 tmins(1e24), tmaxs(-1e24);
            tri0->_winding.get_bounds(tmins, tmaxs);
            tri0->_bounds = new BoundingBox(tmins, tmaxs);
            solid_polys.push_back(tri0);
            _side_polys[tri0->_side_id].push_back(tri0);

            //
            // TRIANGLE 2
            //
            tri0 = new MapPoly;
            tri0->_visible = true;
            tri0->_side_id = side->_editor_id;
            tri0->_vis_occluder = false;
            tri0->_in_group = false;
            tri0->_is_mesh = false;
            p0 = disp_points[(tri_verts[1][0].first * num_cols) + tri_verts[1][0].second];
            p1 = disp_points[(tri_verts[1][1].first * num_cols) + tri_verts[1][1].second];
            p2 = disp_points[(tri_verts[1][2].first * num_cols) + tri_verts[1][2].second];
            tri_normal = ((p1 - p0).normalized().cross(p2 - p0).normalized()).normalized();
            lmins = LVecBase2(1e24);
            lmaxs = LVecBase2(-1e24);
            for (size_t ivert = 0; ivert < 3; ivert++) {
              size_t row = tri_verts[1][ivert].first;
              size_t col = tri_verts[1][ivert].second;
              size_t dvertindex = row * num_cols;
              dvertindex += col;
              const LPoint3 &dpoint = disp_points[dvertindex];
              tri0->_winding.add_point(dpoint);
              tri0->_normals.push_back(-tri_normal);
              tri0->_uvs.push_back(disp_uvs[dvertindex]);
              tri0->_blends.push_back(disp_blends[dvertindex]);

              for (int l = 0; l < 2; l++) {
                PN_stdfloat val = dpoint[0] * lightmap_vecs[l][0] +
                                  dpoint[1] * lightmap_vecs[l][1] +
                                  dpoint[2] * lightmap_vecs[l][2] +
                                  lightmap_vecs[l][3];
                lmins[l] = std::min(val, lmins[l]);
                lmaxs[l] = std::max(val, lmaxs[l]);
              }
            }
            for (int l = 0; l < 2; l++) {
              lmins[l] = std::floor(lmins[l]);
              lmaxs[l] = std::ceil(lmaxs[l]);
              lightmap_mins[l] = (int)lmins[l];
              lightmap_size[l] = (int)(lmaxs[l] - lmins[l]);
            }

            tri0->_lightmap_size = lightmap_size;

            for (size_t ivert = 0; ivert < 3; ivert++) {
              const LPoint3 &dpoint = tri0->_winding.get_point(ivert);
              LVecBase2 lightcoord;
              lightcoord[0] = dpoint.dot(lightmap_vecs[0].get_xyz()) + lightmap_vecs[0][3];
              lightcoord[0] -= lmins[0];
              //lightcoord[0] += 0.5;
              lightcoord[0] /= tri0->_lightmap_size[0];// + 1;

              lightcoord[1] = dpoint.dot(lightmap_vecs[1].get_xyz()) + lightmap_vecs[1][3];
              lightcoord[1] -= lmins[1];
              //lightcoord[1] += 0.5;
              lightcoord[1] /= tri0->_lightmap_size[1];// + 1;

              tri0->_lightmap_uvs.push_back(lightcoord);
            }
            tri0->_material = poly_material;
            tri0->_base_tex = base_tex;
            tmins.set(1e24, 1e24, 1e24);
            tmaxs.set(-1e24, -1e24, -1e24);
            tri0->_winding.get_bounds(tmins, tmaxs);
            tri0->_bounds = new BoundingBox(tmins, tmaxs);
            solid_polys.push_back(tri0);
            _side_polys[tri0->_side_id].push_back(tri0);
          }
        }
      }

      if (mapbuilder_cat.is_debug()) {
        mapbuilder_cat.debug()
          << "Solid " << j << " side " << k << " winding:\n";
        mapbuilder_cat.debug(false)
          << w.get_num_points() << " points\n";
        for (size_t l = 0; l < w.get_num_points(); l++) {
          mapbuilder_cat.debug(false)
            << "\t" << w.get_point(l) << "\n";
        }
        mapbuilder_cat.debug(false)
          << "\tArea: " << w.get_area() << "\n";
        mapbuilder_cat.debug(false)
          << "\tCenter: " << w.get_center() << "\n";
        mapbuilder_cat.debug(false)
          << "\tPlane: " << w.get_plane() << "\n";
      }
    }

    if (bad_solid) {
      continue;
    }

    for (size_t k = 0; k < solid_polys.size(); k++) {
      ent_mesh->_polys.push_back(solid_polys[k]);
    }
  }

  if (ent_mesh->_polys.size() == 0) {
    return;
  }

  ent_mesh->_bounds = new BoundingBox(minp, maxp);

  // Now compute smoothed vertex normals.

  // First, collect all the common vertices and the polygons that reference
  // them.

  PolyVertCollection collection;
  PN_stdfloat cos_angle = cos(deg_2_rad(45.0f));
  for (size_t i = 0; i < ent_mesh->_polys.size(); ++i) {
    MapPoly *poly = ent_mesh->_polys[i];
    PolyVertRef ref;
    ref.poly = poly;
    ref.normal = poly->_normals[0];

    // Now add each vertex from the polygon separately to our collection.
    for (int j = 0; j < poly->_winding.get_num_points(); ++j) {
      ref.vertex = j;
      collection[poly->_winding.get_point(j)].push_back(ref);
    }
  }

  for (auto ci = collection.begin(); ci != collection.end(); ++ci) {
    PolyVertGroup &group = (*ci).second;

    auto gi = group.begin();
    while (gi != group.end()) {
      const PolyVertRef &base_ref = (*gi);
      PolyVertGroup new_group;
      PolyVertGroup leftover_group;
      new_group.push_back(base_ref);
      ++gi;

      while (gi != group.end()) {
        const PolyVertRef &ref = (*gi);
        PN_stdfloat dot = base_ref.normal.dot(ref.normal);
        if (dot > cos_angle) {
          // Close enough to same angle.
          new_group.push_back(ref);
        } else {
          // These polygons are not.
          leftover_group.push_back(ref);
        }
        ++gi;
      }

      LVector3 normal(0.0f);
      for (auto ngi = new_group.begin(); ngi != new_group.end(); ++ngi) {
        const PolyVertRef &ref = (*ngi);
        normal += ref.normal;
      }
      normal /= (PN_stdfloat)new_group.size();
      normal.normalize();

      // Now we have the common normal; apply it to all the vertices.
      for (auto ngi = new_group.begin(); ngi != new_group.end(); ++ngi) {
        const PolyVertRef &ref = (*ngi);
        ref.poly->_normals[ref.vertex] = normal;
      }

      group.swap(leftover_group);
      gi = group.begin();
    }
  }

  ThreadManager::lock();
  if (i == 0) {
    _world_mesh_index = (int)_meshes.size();
  }
  _meshes.push_back(ent_mesh);
  ThreadManager::unlock();
}

/**
 * Computes a lightmap for all polygons in the level.
 */
MapBuilder::ErrorCode MapBuilder::
build_lighting() {
  LightBuilder builder;

  // Make the lights 5000 times as bright as the original .vmf lights.
  // Works better with the physically based camera.
  static constexpr PN_stdfloat light_scale_factor = 1.0f;//5000.0f;

  // Add map polygons to lightmapper.
  for (size_t i = 0; i < _meshes.size(); i++) {
    MapMesh *mesh = _meshes[i];
    for (size_t j = 0; j < mesh->_polys.size(); j++) {
      MapPoly *poly = mesh->_polys[j];

      if (poly->_geom_node == nullptr || poly->_geom_index == -1) {
        continue;
      }

      bool is_sky = false;

      if (poly->_material != nullptr) {
        if (poly->_material->has_tag("compile_trigger")) {
          continue;
        } else if (poly->_material->has_tag("compile_sky")) {
          is_sky = true;
        }
      }

      if (!is_sky) {
        NodePath geom_np(poly->_geom_node);

        uint32_t contents = 0;
        if (poly->_material != nullptr) {
          if (poly->_material->has_tag("compile_water")) {
            // Water don't block or reflect light, but we want a lightmap for it.
            contents |= LightBuilder::C_dont_block_light;
            contents |= LightBuilder::C_dont_reflect_light;
          }
        }

        builder.add_geom(poly->_geom_node->get_geom(poly->_geom_index),
                        poly->_geom_node->get_geom_state(poly->_geom_index),
                        geom_np.get_net_transform(), poly->_lightmap_size,
                        poly->_geom_node, poly->_geom_index, contents);

      } else {
        // Add sky triangles as occluders (not lightmapped) with the sky
        // contents, so rays that hit them bring in the sky/sun color.
        const Winding &w = poly->_winding;
        for (size_t ipoint = 1; ipoint < (w.get_num_points() - 1); ++ipoint) {
          LightBuilder::OccluderTri otri;
          otri.a = w.get_point(ipoint + 1);
          otri.b = w.get_point(ipoint);
          otri.c = w.get_point(0);
          otri.contents = LightBuilder::C_sky;
          builder._occluder_tris.push_back(std::move(otri));
        }
      }
    }
  }

  // Now get static props.
  for (int i = 0; i < _out_data->get_num_static_props(); i++) {
    MapStaticProp *sprop = (MapStaticProp *)_out_data->get_static_prop(i);

    PT(PandaNode) prop_model_node = Loader::get_global_ptr()->load_sync(sprop->get_model_filename());
    if (prop_model_node == nullptr) {
      continue;
    }
    ModelRoot *prop_mdl_root = DCAST(ModelRoot, prop_model_node);
    NodePath prop_model(prop_model_node);
    prop_model.set_pos(sprop->get_pos());
    prop_model.set_hpr(sprop->get_hpr());
    int skin = sprop->get_skin();
    if (skin >= 0 && skin < prop_mdl_root->get_num_material_groups()) {
      prop_mdl_root->set_active_material_group(skin);
    }

    prop_model.flatten_light();

    // Get all the Geoms.
    // If there's an LOD, only get Geoms from the lowest LOD level.
    NodePath lod = prop_model.find("**/+LODNode");
    if (!lod.is_empty()) {
      prop_model = lod.get_child(0);
    }

    pvector<std::pair<CPT(Geom), CPT(RenderState)>> geoms;
    r_collect_geoms(prop_model.node(), geoms);

    sprop->_geom_vertex_lighting.resize(geoms.size());

    if ((sprop->_flags & MapStaticProp::F_no_vertex_lighting)) {
      continue;
    }

    // Now add the triangles from all the geoms as occluders.
    for (int j = 0; j < (int)geoms.size(); ++j) {
      CPT(Geom) geom = geoms[j].first;
      CPT(RenderState) state = geoms[j].second;

      bool cast_shadows = (sprop->_flags & MapStaticProp::F_no_shadows) == 0;

      // Exclude triangles with transparency enabled.
      const TransparencyAttrib *trans;
      state->get_attrib_def(trans);
      if (trans->get_mode() != TransparencyAttrib::M_none) {
        cast_shadows = false;
      }
      if (state->has_attrib(AlphaTestAttrib::get_class_slot())) {
        cast_shadows = false;
      }
      const MaterialAttrib *mattr;
      state->get_attrib_def(mattr);
      Material *mat = mattr->get_material();
      if (mat != nullptr) {
        if ((mat->_attrib_flags & Material::F_transparency) != 0u &&
            mat->_transparency_mode > 0) {
          cast_shadows = false;

        } else if ((mat->_attrib_flags & Material::F_alpha_test) != 0u &&
                    mat->_alpha_test_mode > 0) {
          cast_shadows = false;
        }
      }

      uint32_t contents = 0;
      if (!cast_shadows) {
        contents |= LightBuilder::C_dont_block_light;
      }

      builder.add_vertex_geom(geom, state, TransformState::make_identity(), i, j, contents);
    }
  }

  NodePath dlnp;

  // Now add the lights.
  for (size_t i = 0; i < _source_map->_entities.size(); i++) {
    MapEntitySrc *ent = _source_map->_entities[i];
    if (ent->_class_name != "light" &&
        ent->_class_name != "light_spot" &&
        ent->_class_name != "light_environment") {
      // Not a light entity.
      continue;
    }

    LightBuilder::LightmapLight light;

    if (ent->_properties.find("origin") != ent->_properties.end()) {
      light.pos = KeyValues::to_3f(ent->_properties["origin"]);

    } else {
      light.pos.set(0, 0, 0);
    }

    if (ent->_properties.find("angles") != ent->_properties.end()) {
      // pitch raw roll -> (yaw - 90) pitch roll
      LVecBase3 phr = KeyValues::to_3f(ent->_properties["angles"]);
      light.hpr[0] = phr[1] - 90;
      light.hpr[1] = phr[0];
      light.hpr[2] = phr[2];

    } else {
      light.hpr.set(0, 0, 0);
    }

    if (ent->_properties.find("pitch") != ent->_properties.end()) {
      light.hpr[1] = atof(ent->_properties["pitch"].c_str());
    }

    if (ent->_properties.find("_light") != ent->_properties.end()) {
      light.color = KeyValues::to_4f(ent->_properties["_light"]);
      PN_stdfloat scalar = (light.color[3] / 255.0f) * light_scale_factor;
      light.color[0] = std::pow(light.color[0] / 255.0f, 2.2f) * scalar;
      light.color[1] = std::pow(light.color[1] / 255.0f, 2.2f) * scalar;
      light.color[2] = std::pow(light.color[2] / 255.0f, 2.2f) * scalar;
      light.color[3] = 1.0f;

    } else {
      light.color.set(1, 1, 1, 1);
    }

    PN_stdfloat d50 = 0.0f;
    if (ent->_properties.find("_fifty_percent_distance") != ent->_properties.end()) {
      d50 = atof(ent->_properties["_fifty_percent_distance"].c_str());
    }

    if (d50) {
      PN_stdfloat d0 = 0.0f;
      if (ent->_properties.find("_zero_percent_distance") != ent->_properties.end()) {
        d0 = atof(ent->_properties["_zero_percent_distance"].c_str());
      }
      if (d0 < d50) {
        d0 = d50 * 2.0f;
      }
      PN_stdfloat a = 0, b = 1, c = 0;
      if (!solve_inverse_quadratic_monotonic(0, 1.0f, d50, 2.0f, d0, 256.0f, a, b, c)) {
      }

      PN_stdfloat v50 = c + d50 * (b + d50 * a);
      PN_stdfloat scale = 2.0f / v50;
      a *= scale;
      b *= scale;
      c *= scale;
      light.constant = c;
      light.linear = b;
      light.quadratic = a;

    } else {
      if (ent->_properties.find("_constant_attn") != ent->_properties.end()) {
        light.constant = std::max(0.0f, (float)atof(ent->_properties["_constant_attn"].c_str()));
      } else {
        light.constant = 0;
      }

      if (ent->_properties.find("_linear_attn") != ent->_properties.end()) {
        light.linear = std::max(0.0f, (float)atof(ent->_properties["_linear_attn"].c_str()));
      } else {
        light.linear = 0;
      }

      if (ent->_properties.find("_quadratic_attn") != ent->_properties.end()) {
        light.quadratic = std::max(0.0f, (float)atof(ent->_properties["_quadratic_attn"].c_str()));
      } else {
        light.quadratic = 0;
      }

      if (light.constant == 0 &&
          light.linear == 0 &&
          light.quadratic == 0) {
        light.constant = 1;
      }

      // Scale intensity for unit 100 distance.
      PN_stdfloat ratio = (light.constant + 100 * light.linear + 100 * 100 * light.quadratic);
      if (ratio > 0) {
        light.color[0] *= ratio;
        light.color[1] *= ratio;
        light.color[2] *= ratio;
      }
    }

    if (ent->_properties.find("_exponent") != ent->_properties.end()) {
      light.exponent = atof(ent->_properties["_exponent"].c_str());
      if (!light.exponent) {
        light.exponent = 1;
      }
    } else {
      light.exponent = 1;
    }

    if (ent->_properties.find("_inner_cone") != ent->_properties.end()) {
      light.inner_cone = atof(ent->_properties["_inner_cone"].c_str());
    } else {
      light.inner_cone = 30.0f;
    }

    if (ent->_properties.find("_cone") != ent->_properties.end()) {
      light.outer_cone = atof(ent->_properties["_cone"].c_str());
    } else {
      light.outer_cone = 45.0f;
    }

    if (ent->_class_name == "light") {
      light.type = LightBuilder::LT_point;

      PT(PointLight) pl = new PointLight("pl");
      pl->set_color(light.color);
      pl->set_attenuation(LVecBase3(light.constant, light.linear, light.quadratic));
      NodePath plnp(pl);
      plnp.set_pos(light.pos);
      _out_data->add_light(plnp);

    } else if (ent->_class_name == "light_spot") {
      light.type = LightBuilder::LT_spot;

      PT(Spotlight) sl = new Spotlight("sl");
      sl->set_color(light.color);
      sl->set_attenuation(LVecBase3(light.constant, light.linear, light.quadratic));
      sl->set_inner_cone(light.inner_cone);
      sl->set_outer_cone(light.outer_cone);
      sl->set_exponent(light.exponent);
      NodePath slnp(sl);
      slnp.set_pos(light.pos);
      slnp.set_hpr(light.hpr);
      _out_data->add_light(slnp);

    } else {
      light.type = LightBuilder::LT_directional;

      // We can do sunlight dynamically with cascaded shadow maps.
      // We still want the sun to contribute to indirect light, though.
      light.bake_direct = false;

      // Use the ambient color from the light_environment as the sky color
      // for the lightmapper.

      if (ent->_properties.find("_ambient") != ent->_properties.end()) {
        LColor sky_color = KeyValues::to_4f(ent->_properties["_ambient"]);
        PN_stdfloat scalar = (sky_color[3] / 255.0f) * light_scale_factor;
        sky_color[0] = std::pow(sky_color[0] / 255.0f, 2.2f) * scalar;
        sky_color[1] = std::pow(sky_color[1] / 255.0f, 2.2f) * scalar;
        sky_color[2] = std::pow(sky_color[2] / 255.0f, 2.2f) * scalar;
        sky_color[3] = 1.0f;

        builder.set_sky_color(sky_color);
      }

      if (ent->_properties.find("SunSpreadAngle") != ent->_properties.end()) {
        builder.set_sun_angular_extent(atof(ent->_properties["SunSpreadAngle"].c_str()));
      }

      PT(CascadeLight) dl = new CascadeLight("dl");
      dl->set_color(light.color);
      dlnp = NodePath(dl);
      dlnp.set_hpr(light.hpr);
      _out_data->add_light(dlnp);
    }

    builder._lights.push_back(light);
  }

  // Add ambient probes.

  // Start at the lowest corner of the level bounds and work our way to the top.
  for (PN_stdfloat z = _scene_mins[2]; z <= _scene_maxs[2]; z += 128.0f) {
    for (PN_stdfloat y = _scene_mins[1]; y <= _scene_maxs[1]; y += 128.0f) {
      for (PN_stdfloat x = _scene_mins[0]; x <= _scene_maxs[0]; x += 128.0f) {
        LPoint3 pos(x, y, z);
        if (_out_data->get_area_cluster_tree()->get_leaf_value_from_point(pos) == -1) {
          // Probe is not in valid cluster.  Skip it.
          continue;
        }

        builder._probes.push_back({ pos });
      }
    }
  }

  //VisClusterSampler sampler(_out_data);
  //LVecBase3 probe_density(128.0f, 128.0f, 128.0f);
  //for (int i = 0; i < _out_data->get_num_clusters(); i++) {
  //  pset<LPoint3> samples;
  //  sampler.generate_samples(i, probe_density, 128, 1, samples);
  //  for (const LPoint3 &sample : samples) {
  //    builder._probes.push_back({ sample });
  //  }
  //}

  mapbuilder_cat.info()
    << builder._probes.size() << " ambient probes\n";

  if (!builder.solve()) {
    return EC_lightmap_failed;
  }

#if 1
  // Write static prop vertex light arrays.
  for (const LightBuilder::LightmapGeom &lgeom : builder._geoms) {
    if (lgeom.light_mode != LightBuilder::LightmapGeom::LM_per_vertex) {
      continue;
    }

    MapStaticProp *sprop = (MapStaticProp *)_out_data->get_static_prop(lgeom.model_index);
    sprop->_geom_vertex_lighting[lgeom.geom_index] = lgeom.vertex_light_array;
  }
#endif

#if 0
  // Write debug data.
  LightDebugData &ld_data = _out_data->_light_debug_data;
  for (const LightBuilder::LightmapVertex &v : builder._vertices) {
    LightDebugData::Vertex lv;
    lv.pos = v.pos;
    ld_data._vertices.push_back(lv);
  }
  for (const LightBuilder::LightmapTri &tri : builder._triangles) {
    LightDebugData::Triangle lt;
    lt.vert0 = tri.indices[0];
    lt.vert1 = tri.indices[1];
    lt.vert2 = tri.indices[2];
    ld_data._triangles.push_back(lt);
  }
  for (const LightBuilder::KDNode *pnode = builder._kd_tree_head; pnode != nullptr; pnode = pnode->next) {
    const LightBuilder::KDNode &node = *pnode;

    LightDebugData::KDNode ln;
    ln.first_tri = node.first_triangle;
    ln.num_tris = node.num_triangles;
    ln.back_child = node.get_child_node_index(0);
    ln.front_child = node.get_child_node_index(1);
    ln.mins = node.mins;
    ln.maxs = node.maxs;
    for (int j = 0; j < 6; ++j) {
      ln.neighbors[j] = node.get_neighbor_node_index(j);
    }
    ln.axis = node.axis;
    ln.dist = node.dist;
    ld_data._kd_nodes.push_back(ln);
  }
  for (unsigned int itri : builder._kd_tri_list) {
    ld_data._tri_list.push_back(itri);
  }
#endif

#if 0
  // Debug K-D tree.
  std::stack<int> node_stack;
  std::stack<int> depth_stack;
  node_stack.push(0);
  depth_stack.push(0);
  LineSegs lines("kd");
  while (!node_stack.empty()) {
    int node_idx = node_stack.top();
    node_stack.pop();
    int depth = depth_stack.top();
    depth_stack.pop();

    const LightBuilder::KDNode &node = builder._kd_nodes[node_idx];

    LColor color(1, 0, 0, 1);
    //color[depth % 3] = 1.0f;
    lines.set_color(color);

    const LPoint3 &mins = node.mins;
    const LPoint3 &maxs = node.maxs;

    lines.move_to(mins);
    lines.draw_to(LPoint3(mins.get_x(), mins.get_y(), maxs.get_z()));
    lines.draw_to(LPoint3(mins.get_x(), maxs.get_y(), maxs.get_z()));
    lines.draw_to(LPoint3(mins.get_x(), maxs.get_y(), mins.get_z()));
    lines.draw_to(mins);
    lines.draw_to(LPoint3(maxs.get_x(), mins.get_y(), mins.get_z()));
    lines.draw_to(LPoint3(maxs.get_x(), mins.get_y(), maxs.get_z()));
    lines.draw_to(LPoint3(mins.get_x(), mins.get_y(), maxs.get_z()));
    lines.move_to(LPoint3(maxs.get_x(), mins.get_y(), maxs.get_z()));
    lines.draw_to(maxs);
    lines.draw_to(LPoint3(mins.get_x(), maxs.get_y(), maxs.get_z()));
    lines.move_to(maxs);
    lines.draw_to(LPoint3(maxs.get_x(), maxs.get_y(), mins.get_z()));
    lines.draw_to(LPoint3(mins.get_x(), maxs.get_y(), mins.get_z()));
    lines.move_to(LPoint3(maxs.get_x(), maxs.get_y(), mins.get_z()));
    lines.draw_to(LPoint3(maxs.get_x(), mins.get_y(), mins.get_z()));

    if (node.children[0] != -1) {
      node_stack.push(node.children[0]);
      depth_stack.push(depth + 1);
    }
    if (node.children[1] != -1) {
      node_stack.push(node.children[1]);
      depth_stack.push(depth + 1);
    }
  }

  _out_top->add_child(lines.create());
#endif

  // Now output the probes to the output map data.
  for (size_t i = 0; i < builder._probes.size(); i++) {
    const LightBuilder::LightmapAmbientProbe &probe = builder._probes[i];
    MapAmbientProbe mprobe;
    mprobe._pos = probe.pos;
    for (int j = 0; j < 9; j++) {
      //std::cout << probe.data[j] << "\n";
      mprobe._color[j] = probe.data[j];
    }
    _out_data->add_ambient_probe(mprobe);
  }

  // Assign the sun light to any polys that can see the sky.
  if (!dlnp.is_empty()) {
    for (size_t i = 0; i < _meshes[0]->_polys.size(); ++i) {
      MapPoly *poly = _meshes[0]->_polys[i];
      if (poly->_sees_sky && poly->_geom_node != nullptr) {
        CPT(RenderState) state = poly->_geom_node->get_geom_state(poly->_geom_index);
        CPT(LightAttrib) lattr;
        state->get_attrib_def(lattr);
        state = state->set_attrib(lattr->add_on_light(dlnp));
        poly->_geom_node->set_geom_state(poly->_geom_index, state);
      }
    }
  }

  return EC_ok;
}

/**
 * Bakes and prefilters a cube map texture for each env_cubemap entity in the
 * map.
 */
MapBuilder::ErrorCode MapBuilder::
render_cube_maps() {
  mapbuilder_cat.info()
    << "Baking cube map textures...\n";

  PT(TextureStage) cm_stage = new TextureStage("envmap");

  pvector<vector_int> cm_side_lists;
  pvector<CPT(RenderState)> cm_states;

  for (auto it = _source_map->_entities.begin(); it != _source_map->_entities.end();) {
    MapEntitySrc *ent = *it;
    if (ent->_class_name != "env_cubemap") {
      ++it;
      continue;
    }

    // Position the camera at the origin of the cube map entity.
    LPoint3 pos = KeyValues::to_3f(ent->_properties["origin"]);

    vector_int side_list;
    // The cube map may have a list of sides that should be explicitly given
    // this cube map and not the closest one.
    if (ent->_properties.find("sides") != ent->_properties.end()) {
      vector_string str_side_list;
      extract_words(ent->_properties["sides"], str_side_list);
      for (size_t i = 0; i < str_side_list.size(); i++) {
        int side_id;
        if (!string_to_int(str_side_list[i], side_id)) {
          return EC_unknown_error;
        }
        side_list.push_back(side_id);
      }
    }
    cm_side_lists.push_back(side_list);

    int size_option = atoi(ent->_properties["cubemapsize"].c_str());
    int size;
    if (size_option == 0) {
      size = 128;

    } else {
      size = 1 << (size_option - 1);
    }

    // Just create a cube map texture with no image data.  The images will be
    // baked in the show.
    std::ostringstream ss;
    ss << "cubemap-" << pos[0] << "." << pos[1] << "." << pos[2] << "-" << size;
    PT(Texture) cm_tex = new Texture(ss.str());
    cm_tex->setup_cube_map(size, Texture::T_half_float, Texture::F_rgb16);
    cm_tex->set_clear_color(LColor(0, 0, 0, 1));
    cm_tex->set_minfilter(SamplerState::FT_linear);
    cm_tex->set_magfilter(SamplerState::FT_linear);
    cm_tex->set_wrap_u(SamplerState::WM_clamp);
    cm_tex->set_wrap_v(SamplerState::WM_clamp);
    cm_tex->set_wrap_w(SamplerState::WM_clamp);

    CPT(RenderAttrib) tattr = TextureAttrib::make();
    tattr = DCAST(TextureAttrib, tattr)->add_on_stage(cm_stage, cm_tex);
    cm_states.push_back(RenderState::make(tattr));

    // Save cube map texture in output map data.
    _out_data->add_cube_map(cm_tex, pos, size);

    // Dissolve the env_cubemap entity.
    it = _source_map->_entities.erase(it);
  }

  // Now apply the cube map textures to map polygons.
  for (size_t i = 0; i < _meshes.size(); i++) {
    MapMesh *mesh = _meshes[i];
    for (size_t j = 0; j < mesh->_polys.size(); j++) {
      MapPoly *poly = mesh->_polys[j];
      if (poly->_material == nullptr) {
        continue;
      }

      MaterialParamBase *envmap_p = poly->_material->get_param("envmap");
      if (envmap_p == nullptr || !envmap_p->is_of_type(MaterialParamBool::get_class_type())) {
        continue;
      }

      if (!DCAST(MaterialParamBool, envmap_p)->get_value()) {
        // Env map disabled for this poly's material.
        continue;
      }

      LPoint3 center = poly->_winding.get_center();
      PN_stdfloat closest_distance = 1e24;
      int closest = -1;
      for (size_t k = 0; k < cm_states.size(); k++) {
        const MapCubeMap *mcm = _out_data->get_cube_map(k);

        if (std::find(cm_side_lists[k].begin(), cm_side_lists[k].end(), poly->_side_id) != cm_side_lists[k].end()) {
          // This side was explicitly assigned to this cube map.  Use it.
          closest = k;
          break;
        }

        // Otherwise compute if it's the closest to the polygon's center.
        PN_stdfloat dist = (center - mcm->_pos).length_squared();
        if (dist < closest_distance) {
          closest_distance = dist;
          closest = k;
        }
      }

      if (closest != -1 && poly->_geom_node != nullptr && poly->_geom_index >= 0) {
        // Apply the texture of the selected cube map to the polygon's render state.
        CPT(RenderState) state = poly->_geom_node->get_geom_state(poly->_geom_index);
        state = state->compose(cm_states[closest]);
        poly->_geom_node->set_geom_state(poly->_geom_index, state);
      }
    }
  }

  mapbuilder_cat.info()
    << "Done.\n";

  return EC_ok;
}

class CollideGroupWorkingData {
public:
  PT(PhysTriangleMeshData) tri_mesh_data;
  vector_string surface_props;
};

/**
 * Bakes physics meshes for each brush entity in the level.
 *
 * Both a triangle mesh and a convex mesh are created for each entity.
 * Triangle meshes should be used for brush entities that are solid and
 * visible.  The convex mesh should be used for "volume" entities like
 * triggers.  Triangle meshes cannot be used as a trigger volume, and
 * convex meshes cannot have per-triangle materials.
 */
void MapBuilder::
build_entity_physics(int mesh_index, MapModel &model) {
  MapMesh *mesh = _meshes[mesh_index];
  int ent_index = mesh->_entity;
  MapEntitySrc *ent = _source_map->_entities[ent_index];

  // First do the triangle mesh.  Build it from all of the mesh's
  // MapPolys, which are polygons formed by clipping together the
  // brush planes, and displacement polygons.
  pmap<std::string, CollideGroupWorkingData> collide_type_groups;

  for (MapPoly *poly : mesh->_polys) {
    Material *mat = poly->_material;
    Winding *w = &poly->_winding;

    if (w->is_empty()) {
      continue;
    }

    std::string surface_prop = "default";
    std::string collide_type = "";
    if (mat != nullptr) {
      if (mat->has_tag("surface_prop")) {
        // Grab the physics surface property from the material.
        surface_prop = mat->get_tag_value("surface_prop");
      }
      if (mat->has_tag("collide_type")) {
        collide_type = mat->get_tag_value("collide_type");
      }
    }

    // Find collision group containing this collide type.
    auto git = collide_type_groups.find(collide_type);
    CollideGroupWorkingData *group;
    if (git == collide_type_groups.end()) {
      collide_type_groups[collide_type] = CollideGroupWorkingData();
      collide_type_groups[collide_type].tri_mesh_data = new PhysTriangleMeshData;
      group = &collide_type_groups[collide_type];
    } else {
      group = &(*git).second;
    }

    // Find or add to surface prop list for this collision group.
    vector_string::const_iterator it = std::find(group->surface_props.begin(),
      group->surface_props.end(), surface_prop);
    int mat_index;
    if (it == group->surface_props.end()) {
      // New material.
      mat_index = (int)group->surface_props.size();
      group->surface_props.push_back(surface_prop);
    } else {
      // Existing index.
      mat_index = it - group->surface_props.begin();
    }

    // Add the polygon to the physics triangle mesh.
    // Need to reverse the vertex order.
    pvector<LPoint3> phys_verts;
    phys_verts.resize(w->get_num_points());
    for (int i = 0; i < w->get_num_points(); ++i) {
      phys_verts[i] = w->get_point(i);
    }
    std::reverse(phys_verts.begin(), phys_verts.end());
    group->tri_mesh_data->add_polygon(phys_verts, mat_index);
  }

  // Cook a triangle mesh for each collide group, output collide group info.
  for (auto it = collide_type_groups.begin(); it != collide_type_groups.end(); ++it) {
    const CollideGroupWorkingData &group = (*it).second;
    const std::string &collide_type = (*it).first;

    // Cook the triangle mesh.
    bool ret = group.tri_mesh_data->cook_mesh();
    if (!ret) {
      mapbuilder_cat.warning()
        << "Failed to cook triangle mesh for entity " << ent_index
        << ", classname " << ent->_class_name << "\n";
    }

    MapModel::CollisionGroup mgroup;
    mgroup._collide_type = collide_type;
    mgroup._tri_mesh_data = group.tri_mesh_data->get_mesh_data();
    mgroup._phys_surface_props = group.surface_props;
    model._tri_groups.push_back(std::move(mgroup));
  }

  // Now build convex mesh pieces.  One for each brush in the entity.
  // Don't do this for the world.
  if (ent_index == 0) {
    return;
  }

  for (MapSolid *solid : ent->_solids) {
    PT(PhysConvexMeshData) cm_data = new PhysConvexMeshData;

    // Get polygon for side by clipping a huge winding along side
    // plane by all other side planes.
    for (MapSide *side : solid->_sides) {
      Winding w(side->_plane);
      for (MapSide *other_side : solid->_sides) {
        if (side == other_side) {
          continue;
        }
        w = w.chop(-other_side->_plane);
      }

      // Add winding points to convex mesh.
      for (int i = 0; i < w.get_num_points(); ++i) {
        cm_data->add_point(w.get_point(i));
      }
    }

    // Now cook the convex mesh piece.
    bool ret = cm_data->cook_mesh();
    assert(ret);

    model._convex_mesh_data.push_back(cm_data->get_mesh_data());
  }
}

/**
 *
 */
void MapBuilder::
r_collect_geoms(PandaNode *node, pvector<std::pair<CPT(Geom), CPT(RenderState) > > &geoms) {
  if (node->is_geom_node()) {
    GeomNode *gn = (GeomNode *)node;
    for (size_t i = 0; i < gn->get_num_geoms(); ++i) {
      geoms.push_back({ gn->get_geom(i), gn->get_geom_state(i) });
    }
  }

  for (int i = 0; i < node->get_num_children(); ++i) {
    r_collect_geoms(node->get_child(i), geoms);
  }
}

/**
 *
 */
void MapBuilder::
build_overlays() {

  LVector3 face_normals[6] = {
    LVector3::up(), // floor
    LVector3::down(), // ceiling
    LVector3::back(), // south-facing wall
    LVector3::forward(), // north-facing wall
    LVector3::left(), // west-facing wall
    LVector3::right() // east-facing wall
  };

  for (size_t i = 0; i < _source_map->_entities.size(); ++i) {
    MapEntitySrc *sent = _source_map->_entities[i];
    if (sent->_class_name != "info_overlay") {
      continue;
    }

    LPoint3 pos = KeyValues::to_3f(sent->_properties["BasisOrigin"]);
    LVector3 basis_u = KeyValues::to_3f(sent->_properties["BasisU"]).normalized();
    LVector3 basis_normal = KeyValues::to_3f(sent->_properties["BasisNormal"]).normalized();
    float u_start, u_end, v_start, v_end;
    string_to_float(sent->_properties["StartU"], u_start);
    string_to_float(sent->_properties["EndU"], u_end);
    string_to_float(sent->_properties["StartV"], v_start);
    string_to_float(sent->_properties["EndV"], v_end);

    // RenderOrder simply maps to a depth offset value.
    // We increment by one so we still offset from the wall with
    // RenderOrder 0.
    int offset;
    string_to_int(sent->_properties["RenderOrder"], offset);
    offset++;

    LVector3 basis_v = -basis_normal.cross(basis_u).normalized();

    LMatrix4 m = LMatrix4::ident_mat();
    m.set_row(0, basis_u);
    m.set_row(1, basis_normal);
    m.set_row(2, basis_v);
    m.set_row(3, pos);

    Filename mat_filename("materials/" + downcase(sent->_properties["material"]) + ".mto");
    PT(Material) mat = MaterialPool::load_material(mat_filename);
    if (mat == nullptr) {
      continue;
    }

    LPoint3 ll = KeyValues::to_3f(sent->_properties["uv0"]);
    LPoint3 ur = KeyValues::to_3f(sent->_properties["uv2"]);

    LVecBase3 size = ur - ll;
    size[2] = std::max(size[0], size[1]);
    size *= 0.5;
    PN_stdfloat tmp = size[1];
    size[1] = size[2];
    size[2] = tmp;

    CPT(TransformState) ts = TransformState::make_mat(m);

    std::string sides_str = sent->_properties["sides"];
    vector_string sides_str_vec;
    tokenize(sides_str, sides_str_vec, " ", true);

    vector_int sides;
    for (const std::string &side_str : sides_str_vec) {
      int side;
      string_to_int(side_str, side);
      sides.push_back(side);
    }

    PT(GeomNode) overlay_geom_node = new GeomNode("overlay");
    NodePath np_ident("ident");

    LVecBase3 uv_scale(-(u_end - u_start), (v_end - v_start), 1);
    LVecBase3 uv_offset(u_end, 1.0f - v_end, 0);
    CPT(TransformState) uv_transform = TransformState::make_pos_hpr_scale(uv_offset, LVecBase3(0.0f), uv_scale);

    for (int side_id : sides) {
      for (MapPoly *poly : _side_polys[side_id]) {
        if (poly->_geom_node == nullptr ||
            poly->_geom_index == -1) {
          // Poly didn't result in visible geometry, so don't try to decal it.
          continue;
        }
        NodePath np("tmp");
        np.set_state(poly->_geom_node->get_geom_state(poly->_geom_index));
        np.set_depth_offset(offset);
        np.set_material(mat);
        DecalProjector proj;
        proj.set_projector_parent(np_ident);
        proj.set_projector_transform(ts);
        proj.set_projector_bounds(-size, size);
        proj.set_decal_parent(np_ident);
        proj.set_decal_render_state(np.get_state());
        proj.set_decal_uv_transform(uv_transform);

        PT(GeomNode) tmp_geom_node = new GeomNode("tmp");
        tmp_geom_node->add_geom(poly->_geom_node->get_geom(poly->_geom_index)->make_copy(),
          poly->_geom_node->get_geom_state(poly->_geom_index));
        np = NodePath(tmp_geom_node);
        np.reparent_to(np_ident);

        if (proj.project(np)) {
          PT(PandaNode) frag = proj.generate();
          overlay_geom_node->add_geoms_from(DCAST(GeomNode, frag));
        }
      }
    }

    if (overlay_geom_node->get_num_geoms() == 0) {
      continue;
    }

    NodePath(overlay_geom_node).flatten_strong();
    _out_data->add_overlay(overlay_geom_node);
  }
}
