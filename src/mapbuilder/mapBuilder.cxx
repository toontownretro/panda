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
#include "visBuilder.h"
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

#define HAVE_STEAM_AUDIO
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
  _options(options)
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

  ec = build_polygons();
  if (ec != EC_ok) {
    return ec;
  }

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

  // Make the octree bounds cubic and closest pow 2.
  LVector3 scene_size = _scene_maxs - _scene_mins;
  PN_stdfloat octree_size = ceil_pow_2(std::ceil(std::max(scene_size[0], std::max(scene_size[1], scene_size[2]))));
  LPoint3 octree_mins = _scene_mins;
  LPoint3 octree_maxs(_scene_mins + LPoint3(octree_size));

  lightbuilder_cat.info()
    << "Octree mins: " << octree_mins << " Octree maxs: " << octree_maxs << "\n";

  // Now build mesh groups by recursively dividing all polygons in an octree
  // fashion.
  pvector<MapGeomBase *> all_geoms;
  for (MapMesh *mesh : _meshes) {
    for (MapPoly *poly : mesh->_polys) {
      all_geoms.push_back(poly);
    }
  }
  divide_meshes(all_geoms, octree_mins, octree_maxs);

  mapbuilder_cat.info()
    << "Grouped " << all_geoms.size() << " polygons into " << _mesh_groups.size()
    << " groups\n";

  // Output entity information.
  for (size_t i = 0; i < _source_map->_entities.size(); i++) {
    MapEntitySrc *src_ent = _source_map->_entities[i];
    if (src_ent->_class_name == "func_detail") {
      continue;
    }

    PT(MapEntity) ent = new MapEntity;
    ent->set_class_name(src_ent->_class_name);

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

    _out_data->add_entity(ent);
  }

  //
  // VISIBILITY
  //
  switch (_options.get_vis()) {
  case MapBuildOptions::VT_voxel:
    {
      VisBuilder vis(this);
      if (!vis.build()) {
        return EC_unknown_error;
      }
    }
    break;

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

            Filename material_filename = downcase(side->_material_filename.get_fullpath());
            if (material_filename.get_extension().empty()) {
              material_filename.set_extension("pmat");
            }

            PT(Material) poly_material = MaterialPool::load_material(material_filename);

            if (poly_material != nullptr) {
              if ((poly_material->_attrib_flags & Material::F_transparency) != 0u &&
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
    }
    break;

  case MapBuildOptions::VT_none:
  default:
    break;
  }

#if 0
  {


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
        //if (!node->_opaque) {
          leaves.push_back(node);
        //}
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
    //_out_top->add_child(leaf_lines);

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

  }
#endif

  PT(GeomVertexArrayFormat) arr = new GeomVertexArrayFormat;
  arr->add_column(InternalName::get_vertex(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_point);
  arr->add_column(InternalName::get_normal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_normal);
  arr->add_column(InternalName::get_tangent(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
  arr->add_column(InternalName::get_binormal(), 3, GeomEnums::NT_stdfloat, GeomEnums::C_vector);
  arr->add_column(InternalName::get_texcoord(), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
  arr->add_column(InternalName::get_texcoord_name("lightmap"), 2, GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(arr);

  //arr->add_column(InternalName::make("blend"), 1, GeomEnums::NT_stdfloat, GeomEnums::C_other);
  //CPT(GeomVertexFormat) blend_format = GeomVertexFormat::register_format(arr);

  // Now write out the meshes to GeomNodes.

  // Start with the mesh groups.
  for (size_t i = 0; i < _mesh_groups.size(); i++) {
    const MapGeomGroup &group = _mesh_groups[i];

    // Now build the Geoms within the group.

    std::ostringstream ss;
    ss << "mesh-group-" << i;
    PT(GeomNode) geom_node = new GeomNode(ss.str());
    //geom_node->set_attrib(ColorAttrib::make_flat(cluster_colors[i % 6]));

    PT(PhysTriangleMeshData) phys_mesh_data = new PhysTriangleMeshData;
    vector_string surface_props;
    int phys_polygons = 0;

    pvector<MapPoly *> group_polys;

    for (MapGeomBase *geom : group.geoms) {
      if (geom->_is_mesh) {
        MapMesh *mesh = (MapMesh *)geom;
        for (MapPoly *poly : mesh->_polys) {
          group_polys.push_back(poly);
        }

      } else {
        group_polys.push_back((MapPoly *)geom);
      }
    }

    for (MapPoly *poly : group_polys) {
      PT(GeomVertexData) vdata = new GeomVertexData(
        geom_node->get_name(), format/*poly->_blends.empty() ? format : blend_format*/,
        GeomEnums::UH_static);

      add_poly_to_geom_node(poly, vdata, geom_node);

      Material *mat = poly->_material;
      Winding *w = &poly->_winding;

      bool add_phys = (mat != nullptr) ?
        (!mat->has_tag("compile_trigger") && !mat->has_tag("compile_nodraw")) : true;

      if (add_phys) {

        std::string surface_prop = "default";
        if (mat != nullptr && mat->has_tag("surface_prop")) {
          // Grab physics surface property from material.
          surface_prop = mat->get_tag_value("surface_prop");
        }

        // Find or add to surface prop list for this mesh.
        int mat_index = -1;
        for (int j = 0; j < (int)surface_props.size(); ++j) {
          if (surface_props[j] == surface_prop) {
            mat_index = j;
            break;
          }
        }
        if (mat_index == -1) {
          mat_index = (int)surface_props.size();
          surface_props.push_back(surface_prop);
        }

        // Add the polygon to the physics triangle mesh.
        // Need to reverse them.
        pvector<LPoint3> phys_verts;
        phys_verts.resize(w->get_num_points());
        for (int k = 0; k < w->get_num_points(); k++) {
          phys_verts[k] = w->get_point(k);
        }
        std::reverse(phys_verts.begin(), phys_verts.end());
        phys_mesh_data->add_polygon(phys_verts, mat_index);
        phys_polygons++;
      }
    }

    //if (geom_node->get_num_geoms() == 0 && phys_polygons == 0) {
      // No geometry or physics polygons.  Skip it.
    //  continue;
    //}

    MapMeshGroup out_group;
    out_group._clusters = group.clusters;
    out_group._geom_node = geom_node;
    _out_data->add_mesh_group(out_group);

    // The node we parent the mesh group to will decide which mesh group(s) to
    // render based on the current view cluster.
    _out_node->add_child(geom_node);

    if (phys_polygons > 0) {
      // Cook the physics mesh.
      if (!phys_mesh_data->cook_mesh()) {
        mapbuilder_cat.error()
          << "Failed to cook physics mesh for mesh group " << i << "\n";
        _out_data->add_model_phys_data(MapModelPhysData());
      } else {
        MapModelPhysData mm_phys_data;
        mm_phys_data._phys_mesh_data = phys_mesh_data->get_mesh_data();
        mm_phys_data._phys_surface_props = surface_props;
        _out_data->add_model_phys_data(mm_phys_data);
      }
    } else {
      _out_data->add_model_phys_data(MapModelPhysData());
    }
  }

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

  // After building the lightmaps, we can flatten the Geoms within each mesh
  // group to reduce draw calls.  If we flattened before building lightmaps,
  // Geoms would have overlapping lightmap UVs.
  for (size_t i = 0; i < _out_data->get_num_mesh_groups(); i++) {
    NodePath(_out_node->get_child(i)).flatten_strong();
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
  std::cerr << "progress: " << progress << "\n";
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
  ctx_settings.simdLevel = IPL_SIMDLEVEL_AVX512;
  ctx_settings.logCallback = ipl_log;
  IPLerror err = iplContextCreate(&ctx_settings, &context);
  assert(err == IPL_STATUS_SUCCESS);

  IPLEmbreeDeviceSettings embree_set{};
  IPLEmbreeDevice embree_dev = nullptr;
  err = iplEmbreeDeviceCreate(context, &embree_set, &embree_dev);
  assert(err == IPL_STATUS_SUCCESS);

  IPLScene scene = nullptr;
  IPLSceneSettings scene_settings{};
  memset(&scene_settings, 0, sizeof(IPLSceneSettings));
  scene_settings.type = IPL_SCENETYPE_EMBREE;
  scene_settings.embreeDevice = embree_dev;
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

  // Build up a huge vector of all Geoms in the entire scene.  World geometry
  // and static props.
  pvector<IPLGeomEntry> geoms;

  pvector<IPLMaterial> materials;
  pmap<std::string, int> material_indices;

  // Start with static world geometry.
  for (int i = 0; i < _out_node->get_num_children(); i++) {
    GeomNode *child = (GeomNode *)_out_node->get_child(i);
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
  }

  // Now get static props.
  for (int i = 0; i < _out_data->get_num_entities(); i++) {
    MapEntity *ent = _out_data->get_entity(i);
    if (ent->get_class_name() != "prop_static") {
      continue;
    }

    PDXElement *props = ent->get_properties();

    Filename model_filename = Filename::from_os_specific(props->get_attribute_value("model").get_string());
    model_filename.set_extension("bam");
    PT(PandaNode) prop_model_node = Loader::get_global_ptr()->load_sync(model_filename);
    if (prop_model_node == nullptr) {
      continue;
    }
    NodePath prop_model(prop_model_node);

    if (props->has_attribute("origin")) {
      LPoint3 pos;
      props->get_attribute_value("origin").to_vec3(pos);
      prop_model.set_pos(pos);
    }

    if (props->has_attribute("angles")) {
      LVecBase3 phr;
      props->get_attribute_value("angles").to_vec3(phr);
      prop_model.set_hpr(phr[1] - 90, -phr[0], phr[2]);
    }

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

        // Place probe here.
        IPLSphere sphere;
        sphere.center.x = leaf_center[0] * HAMMER_UNITS_TO_METERS;
        sphere.center.y = leaf_center[2] * HAMMER_UNITS_TO_METERS;
        sphere.center.z = -leaf_center[1] * HAMMER_UNITS_TO_METERS;
        sphere.radius = 10.0f;
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
      bake_params.sceneType = IPL_SCENETYPE_EMBREE;
      bake_params.identifier = identifier;
      int flags = IPL_REFLECTIONSBAKEFLAGS_BAKECONVOLUTION|IPL_REFLECTIONSBAKEFLAGS_BAKEPARAMETRIC;
      bake_params.bakeFlags = (IPLReflectionsBakeFlags)flags;
      bake_params.probeBatch = batch;
      bake_params.numRays = 32768;
      bake_params.numDiffuseSamples = 1024;
      bake_params.numBounces = 64;
      bake_params.simulatedDuration = 1.0f;
      bake_params.savedDuration = 1.0f;
      bake_params.order = 2;
      bake_params.numThreads = _options.get_num_threads();
      bake_params.irradianceMinDistance = 1.0f;
      bake_params.rayBatchSize = 1;
      bake_params.bakeBatchSize = 1;
      bake_params.openCLDevice = nullptr;
      bake_params.radeonRaysDevice = nullptr;
      iplReflectionsBakerBake(context, &bake_params, nullptr, nullptr);
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
  iplEmbreeDeviceRelease(&embree_dev);
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
        mat->has_tag("compile_trigger") ||
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

  // Check if the render state needs transparency.
  if (poly->_base_tex != nullptr && Texture::has_alpha(poly->_base_tex->get_format())) {
    state = state->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_dual));
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
 *
 */
void MapBuilder::
divide_meshes(const pvector<MapGeomBase *> &geoms, const LPoint3 &node_mins, const LPoint3 &node_maxs) {

  pvector<MapGeomBase *> unassigned_geoms = geoms;

  for (int i = 0; i < 8; i++) {
    LPoint3 this_mins = node_mins;
    LPoint3 this_maxs = node_maxs;

    LVector3 size = this_maxs - this_mins;
    size *= 0.5;

    if ((i & 4) != 0) {
      this_mins[0] += size[0];
    }
    if ((i & 2) != 0) {
      this_mins[1] += size[1];
    }
    if ((i & 1) != 0) {
      this_mins[2] += size[2];
    }

    this_maxs = this_mins + size;

    LVector3 qsize = size * 0.5f;

    BoundingBox node_bounds(this_mins, this_maxs);
    node_bounds.local_object();

    // The list of geoms at this node.
    pvector<MapGeomBase *> node_geoms;

    // Go through all the unassigned geoms at this node and see if they
    // can be assigned to us.
    for (auto it = unassigned_geoms.begin(); it != unassigned_geoms.end();) {
      MapGeomBase *geom = *it;
      if (geom->overlaps_box(this_mins + qsize, qsize)) {
        // Yes!  This geom can be assigned to us.
        node_geoms.push_back(geom);
        // Remove it from the unassigned list.
        it = unassigned_geoms.erase(it);
      } else {
        ++it;
      }
    }

    if (node_geoms.empty()) {
      // Nothing in this part of the world.
      continue;
    }

    if (size[0] <= _options.get_mesh_group_size()) {
      // We've reached the mesh group size threshold and we have a set of
      // map geometry contained within this node.  Create a mesh group here.

      MapGeomGroup group;
      group.geoms = node_geoms;
      group.bounds = new BoundingBox;
      for (MapGeomBase *geom : node_geoms) {
        assert(!geom->_in_group);
        group.bounds->extend_by(geom->_bounds);
        geom->_in_group = true;
      }
      _mesh_groups.push_back(std::move(group));

    } else {
      // Keep dividing meshes amongst octants until we reach the mesh group
      // size threshold.
      divide_meshes(node_geoms, this_mins, this_maxs);
    }
  }
}

/**
 * Builds a polygon soup from the convex solids and displacement surfaces in
 * the map.
 */
MapBuilder::ErrorCode MapBuilder::
build_polygons() {
  _world_mesh_index = -1;
  for (size_t i = 0; i < _source_map->_entities.size(); i++) {
    build_entity_polygons(i);
  }

  //ThreadManager::run_threads_on_individual(
  //  "BuildPolygons", _source_map->_entities.size(),
  // false, std::bind(&MapBuilder::build_entity_polygons, this, std::placeholders::_1));
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

  if (ent->_class_name == "func_door" ||
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

      // We now have the final polygon for the side.

      Filename material_filename = downcase(side->_material_filename.get_fullpath());
      if (material_filename.get_extension().empty()) {
        material_filename.set_extension("pmat");
      }

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
      // Twice the resolution for the GPU lightmapper.
      PN_stdfloat lightmap_scale = side->_lightmap_scale * 0.5f;
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
        poly->_side_id = side->_editor_id;
        poly->_winding = w;
        poly->_in_group = false;
        poly->_is_mesh = false;
        LPoint3 polymin, polymax;
        w.get_bounds(polymin, polymax);
        poly->_bounds = new BoundingBox(polymin, polymax);
        poly->_material = poly_material;
        poly->_base_tex = base_tex;

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

        for (int l = 0; l < 2; l++) {
          lmins[l] = std::floor(lmins[l]);
          lmaxs[l] = std::ceil(lmaxs[l]);
          lightmap_mins[l] = (int)lmins[l];
          poly->_lightmap_size[l] = (int)(lmaxs[l] - lmins[l]);
        }

        for (size_t ivert = 0; ivert < w.get_num_points(); ivert++) {
          const LPoint3 &point = w.get_point(ivert);
          LVecBase2 lightcoord;
          lightcoord[0] = point.dot(lightmap_vecs[0].get_xyz()) + lightmap_vecs[0][3];
          lightcoord[0] -= lightmap_mins[0];
          lightcoord[0] += 0.5;
          lightcoord[0] /= poly->_lightmap_size[0] + 1;

          lightcoord[1] = point.dot(lightmap_vecs[1].get_xyz()) + lightmap_vecs[1][3];
          lightcoord[1] -= lightmap_mins[1];
          lightcoord[1] += 0.5;
          lightcoord[1] /= poly->_lightmap_size[1] + 1;

          poly->_lightmap_uvs.push_back(lightcoord);
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

            // Do lightmap coordinates per quad on the displacement.
            std::pair<size_t, size_t> quad_verts[4] = {
              { irow, icol },
              { irow, icol + 1 },
              { irow + 1, icol },
              { irow + 1, icol + 1 }
            };
            LVecBase2 lmins(1e24), lmaxs(-1e24);
            LVecBase2i lightmap_mins, lightmap_size;
            for (size_t ivert = 0; ivert < 4; ivert++) {
              size_t row = quad_verts[ivert].first;
              size_t col = quad_verts[ivert].second;
              size_t dvertindex = (row * num_cols) + col;
              const LPoint3 &dpoint = disp_points[dvertindex];
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

            //
            // TRIANGLE 1
            //
            PT(MapPoly) tri0 = new MapPoly;
            tri0->_side_id = side->_editor_id;
            tri0->_vis_occluder = false;
            tri0->_in_group = false;
            tri0->_is_mesh = false;
            tri0->_lightmap_size = lightmap_size;
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
            }

            for (size_t ivert = 0; ivert < 3; ivert++) {
              const LPoint3 &dpoint = tri0->_winding.get_point(ivert);
              LVecBase2 lightcoord;
              lightcoord[0] = dpoint.dot(lightmap_vecs[0].get_xyz()) + lightmap_vecs[0][3];
              lightcoord[0] -= lightmap_mins[0];
              lightcoord[0] += 0.5;
              lightcoord[0] /= tri0->_lightmap_size[0] + 1;

              lightcoord[1] = dpoint.dot(lightmap_vecs[1].get_xyz()) + lightmap_vecs[1][3];
              lightcoord[1] -= lightmap_mins[1];
              lightcoord[1] += 0.5;
              lightcoord[1] /= tri0->_lightmap_size[1] + 1;

              tri0->_lightmap_uvs.push_back(lightcoord);
            }
            tri0->_material = poly_material;
            tri0->_base_tex = base_tex;
            LPoint3 tmins(1e24), tmaxs(-1e24);
            tri0->_winding.get_bounds(tmins, tmaxs);
            tri0->_bounds = new BoundingBox(tmins, tmaxs);
            solid_polys.push_back(tri0);

            //
            // TRIANGLE 2
            //
            tri0 = new MapPoly;
            tri0->_side_id = side->_editor_id;
            tri0->_vis_occluder = false;
            tri0->_in_group = false;
            tri0->_is_mesh = false;
            tri0->_lightmap_size = lightmap_size;
            p0 = disp_points[(tri_verts[1][0].first * num_cols) + tri_verts[1][0].second];
            p1 = disp_points[(tri_verts[1][1].first * num_cols) + tri_verts[1][1].second];
            p2 = disp_points[(tri_verts[1][2].first * num_cols) + tri_verts[1][2].second];
            tri_normal = ((p1 - p0).normalized().cross(p2 - p0).normalized()).normalized();
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
            }
            for (size_t ivert = 0; ivert < 3; ivert++) {
              const LPoint3 &dpoint = tri0->_winding.get_point(ivert);
              LVecBase2 lightcoord;
              lightcoord[0] = dpoint.dot(lightmap_vecs[0].get_xyz()) + lightmap_vecs[0][3];
              lightcoord[0] -= lightmap_mins[0];
              lightcoord[0] += 0.5;
              lightcoord[0] /= tri0->_lightmap_size[0] + 1;

              lightcoord[1] = dpoint.dot(lightmap_vecs[1].get_xyz()) + lightmap_vecs[1][3];
              lightcoord[1] -= lightmap_mins[1];
              lightcoord[1] += 0.5;
              lightcoord[1] /= tri0->_lightmap_size[1] + 1;

              tri0->_lightmap_uvs.push_back(lightcoord);
            }
            tri0->_material = poly_material;
            tri0->_base_tex = base_tex;
            tmins.set(1e24, 1e24, 1e24);
            tmaxs.set(-1e24, -1e24, -1e24);
            tri0->_winding.get_bounds(tmins, tmaxs);
            tri0->_bounds = new BoundingBox(tmins, tmaxs);
            solid_polys.push_back(tri0);
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

      if (poly->_material != nullptr && poly->_material->has_tag("compile_sky")) {
        // Skip sky polygons.  The lightmapper treats emptiness as the sky.
        continue;
      }

      NodePath geom_np(poly->_geom_node);

      builder.add_geom(poly->_geom_node->get_geom(poly->_geom_index),
                       poly->_geom_node->get_geom_state(poly->_geom_index),
                       geom_np.get_net_transform(), poly->_lightmap_size,
                       poly->_geom_node, poly->_geom_index);
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

  // Assign the sun light to any mesh groups that can see the sky.
  if (!dlnp.is_empty()) {
    for (size_t i = 0; i < _mesh_groups.size(); ++i) {
      if (_mesh_groups[i]._can_see_sky) {
        NodePath(_out_node->get_child(i)).set_light(dlnp);
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

  GraphicsEngine *engine = GraphicsEngine::get_global_ptr();
  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  PT(GraphicsPipe) pipe = selection->make_module_pipe("pandagl");
  if (pipe == nullptr) {
    return EC_unknown_error;
  }

  // Make sure we don't render any cube maps on surfaces when rendering
  // cube maps.
  //default_cube_map = "";

  //ShaderManager::get_global_ptr()->set_default_cube_map(nullptr);

  PT(Shader) filter_shader = Shader::load_compute(Shader::SL_GLSL, "shaders/cubemap_filter.compute.glsl");
  NodePath filter_state("cm_filter");
  filter_state.set_shader(filter_shader);

  FrameBufferProperties props;
  props.clear();
  WindowProperties winprops;
  winprops.clear();
  winprops.set_size(1, 1);

  PT(GraphicsOutput) output = engine->make_output(pipe, "cubemap_host", -1, props, winprops,
                                                  GraphicsPipe::BF_refuse_window);
  if (output == nullptr) {
    return EC_unknown_error;
  }
  GraphicsStateGuardian *gsg = output->get_gsg();

  props.set_rgba_bits(16, 16, 16, 16);
  props.set_depth_bits(1);
  //props.set_multisamples(0);
  props.set_force_hardware(true);
  props.set_float_color(true);

  // Make sure we antialias and render an HDR cube map.
  //_out_top->set_attrib(AntialiasAttrib::make(AntialiasAttrib::M_multisample));
  _out_top->set_attrib(LightRampAttrib::make_identity());

  PT(TextureStage) cm_stage = new TextureStage("envmap");

  pvector<vector_int> cm_side_lists;
  pvector<CPT(RenderState)> cm_states;

  for (auto it = _source_map->_entities.begin(); it != _source_map->_entities.end();) {
    MapEntitySrc *ent = *it;
    if (ent->_class_name != "env_cubemap") {
      ++it;
      continue;
    }

    // Place the cube map camera rig into the level scene graph.
    NodePath cam_rig("cubemap_cam_rig");
    cam_rig.reparent_to(NodePath(_out_top));

    // Position the camera at the origin of the cube map entity.
    LPoint3 pos = KeyValues::to_3f(ent->_properties["origin"]);
    cam_rig.set_pos(pos);

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

    //int size_option = atoi(ent->_properties["cubemapsize"].c_str());
    int size = 512;
    //if (size_option == 0) {
    //  size = 128;

    //} else {
    //  size = 1 << (size_option - 1);
    //}

    // Create the offscreen buffer and a camera/display region pair for each
    // cube map face.
    PT(GraphicsOutput) buffer = output->make_cube_map("cubemap_render", size, cam_rig,
                                                      PandaNode::get_all_camera_mask(), true, &props);
    if (buffer == nullptr) {
      return EC_unknown_error;
    }

    engine->open_windows();

    // Now render into the cube map texture.
    engine->render_frame();
    engine->render_frame();
    engine->sync_frame();

    gsg->finish();

    engine->remove_window(buffer);

    Texture *cm_tex = buffer->get_texture();
    // Make sure mipmaps are enabled.
    cm_tex->set_minfilter(SamplerState::FT_linear_mipmap_linear);
    cm_tex->set_magfilter(SamplerState::FT_linear);

    filter_state.set_shader_input("inputTexture", cm_tex);

    // Now filter the cube map down the mip chain.
    int mip = 0;
    while (size > 1) {
      size /= 2;
      mip++;
      filter_state.set_shader_input("outputTexture", cm_tex, false, true, -1, mip, 0);
      filter_state.set_shader_input("mipLevel_mipSize_numMips", LVecBase3i(mip, size, 10));
      gsg->set_state_and_transform(filter_state.get_state(), TransformState::make_identity());
      gsg->dispatch_compute(size / 16, size / 16, 6);
    }
    gsg->finish();

    engine->extract_texture_data(cm_tex, gsg);

    CPT(RenderAttrib) tattr = TextureAttrib::make();
    tattr = DCAST(TextureAttrib, tattr)->add_on_stage(cm_stage, cm_tex);
    cm_states.push_back(RenderState::make(tattr));

    // Save cube map texture in output map data.
    _out_data->add_cube_map(cm_tex, pos);

    // Dissolve the env_cubemap entity.
    it = _source_map->_entities.erase(it);

    cam_rig.remove_node();
  }

  engine->remove_window(output);

  //_out_top->clear_attrib(AntialiasAttrib::get_class_slot());
  _out_top->clear_attrib(LightRampAttrib::get_class_slot());

  // Now apply the cube map textures to map polygons.
  for (size_t i = 0; i < _meshes.size(); i++) {
    MapMesh *mesh = _meshes[i];
    for (size_t j = 0; j < mesh->_polys.size(); j++) {
      MapPoly *poly = mesh->_polys[j];
      if (poly->_material == nullptr) {
        continue;
      }

      MaterialParamBase *envmap_p = poly->_material->get_param("env_map");
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

  // Now build a K-D tree of cube map positions for locating the
  // closest cube map to use for a model.
  //KDTree cm_tree;
  //for (int i = 0; i < _out_data->get_num_cube_maps(); i++) {
  //  const MapCubeMap *mcm = _out_data->get_cube_map(i);
  //  cm_tree.add_input(mcm->_pos, mcm->_pos, i);
  //}
  //cm_tree.build();
  //_out_data->set_cube_map_tree(std::move(cm_tree));

  mapbuilder_cat.info()
    << "Done.\n";

  return EC_ok;
}
