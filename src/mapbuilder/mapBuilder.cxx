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

// DEBUG INCLUDES
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"
#include "geomVertexWriter.h"

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
  if (_options.get_vis()) {
    VisBuilder vis(this);
    if (!vis.build()) {
      return EC_unknown_error;
    }

    if (_options.get_vis_show_solid_voxels()) {
      LineSegs lines("debug-solid-voxels");
      lines.set_color(LColor(0, 0, 1, 1));
      pvector<LPoint3i> solid_voxels = vis._voxels.get_solid_voxels();
      for (size_t i = 0; i < solid_voxels.size(); i++) {
        PT(BoundingBox) bounds = vis._voxels.get_voxel_bounds(solid_voxels[i]);
        const LPoint3 &mins = bounds->get_minq();
        const LPoint3 &maxs = bounds->get_maxq();
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
      }
      _out_top->add_child(lines.create());
    }

    if (_options.get_vis_show_areas()) {
      for (size_t i = 0; i < vis._area_clusters.size(); i++) {
        AreaCluster *cluster = vis._area_clusters[i];

        std::ostringstream ss;
        ss << "area-cluster-" << cluster->_id;
        LineSegs lines(ss.str());

        lines.set_color(cluster_colors[cluster->_id % 6]);

        for (const AreaCluster::AreaBounds &ab : cluster->_cluster_boxes) {
          PT(BoundingBox) bbox = vis._voxels.get_voxel_bounds(ab._min_voxel, ab._max_voxel);
          const LPoint3 &mins = bbox->get_minq();
          const LPoint3 &maxs = bbox->get_maxq();
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
        }

        _out_top->add_child(lines.create());
      }
    }
  }

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
        // Add the polygon to the physics triangle mesh.
        // Need to reverse them.
        pvector<LPoint3> phys_verts;
        phys_verts.resize(w->get_num_points());
        for (int k = 0; k < w->get_num_points(); k++) {
          phys_verts[k] = w->get_point(k);
        }
        std::reverse(phys_verts.begin(), phys_verts.end());
        phys_mesh_data->add_polygon(phys_verts);
        phys_polygons++;
      }
    }

    //if (geom_node->get_num_geoms() == 0 && phys_polygons == 0) {
      // No geometry or physics polygons.  Skip it.
    //  continue;
    //}

    MapMeshGroup out_group;
    out_group._clusters = group.clusters;
    _out_data->add_mesh_group(out_group);

    // The node we parent the mesh group to will decide which mesh group(s) to
    // render based on the current view cluster.
    _out_node->add_child(geom_node);

    if (phys_polygons > 0) {
      // Cook the physics mesh.
      if (!phys_mesh_data->cook_mesh()) {
        mapbuilder_cat.error()
          << "Failed to cook physics mesh for mesh group " << i << "\n";
        _out_data->add_model_phys_data(CPTA_uchar());
      } else {
        _out_data->add_model_phys_data(phys_mesh_data->get_mesh_data());
      }
    } else {
      _out_data->add_model_phys_data(CPTA_uchar());
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
  }

  // After building the lightmaps, we can flatten the Geoms within each mesh
  // group to reduce draw calls.  If we flattened before building lightmaps,
  // Geoms would have overlapping lightmap UVs.
  for (size_t i = 0; i < _out_data->get_num_mesh_groups(); i++) {
    NodePath(_out_node->get_child(i)).flatten_strong();
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

        if (side->_displacement != nullptr) {
          poly->_vis_occluder = false;

        } else if (poly_material != nullptr) {
          if (poly_material->has_tag("compile_clip") ||
              poly_material->has_tag("compile_trigger")) {
            poly->_vis_occluder = false;

          } else {
            poly->_vis_occluder = true;
          }

        } else {
          poly->_vis_occluder = true;
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

            disp_normals.push_back(dvert._normal.normalized());

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
            tri0->_vis_occluder = false;
            tri0->_in_group = false;
            tri0->_is_mesh = false;
            tri0->_lightmap_size = lightmap_size;
            LPoint3 p0 = disp_points[(tri_verts[0][0].first * num_cols) + tri_verts[0][0].second];
            LPoint3 p1 = disp_points[(tri_verts[0][1].first * num_cols) + tri_verts[0][1].second];
            LPoint3 p2 = disp_points[(tri_verts[0][2].first * num_cols) + tri_verts[0][2].second];
            LVector3 tri_normal = ((p1 - p0).cross(p2 - p0)).normalized();
            for (size_t ivert = 0; ivert < 3; ivert++) {
              size_t row = tri_verts[0][ivert].first;
              size_t col = tri_verts[0][ivert].second;
              size_t dvertindex = row * num_cols;
              dvertindex += col;
              const LPoint3 &dpoint = disp_points[dvertindex];
              tri0->_winding.add_point(dpoint);
              tri0->_normals.push_back(tri_normal);
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
            tri0->_vis_occluder = false;
            tri0->_in_group = false;
            tri0->_is_mesh = false;
            tri0->_lightmap_size = lightmap_size;
            p0 = disp_points[(tri_verts[1][0].first * num_cols) + tri_verts[1][0].second];
            p1 = disp_points[(tri_verts[1][1].first * num_cols) + tri_verts[1][1].second];
            p2 = disp_points[(tri_verts[1][2].first * num_cols) + tri_verts[1][2].second];
            tri_normal = ((p1 - p0).cross(p2 - p0)).normalized();
            for (size_t ivert = 0; ivert < 3; ivert++) {
              size_t row = tri_verts[1][ivert].first;
              size_t col = tri_verts[1][ivert].second;
              size_t dvertindex = row * num_cols;
              dvertindex += col;
              const LPoint3 &dpoint = disp_points[dvertindex];
              tri0->_winding.add_point(dpoint);
              tri0->_normals.push_back(tri_normal);
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

    if (ent->_properties.find("_exponent") != ent->_properties.end()) {
      light.exponent = atof(ent->_properties["_exponent"].c_str());
    } else {
      light.exponent = 1;
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

    } else if (ent->_class_name == "light_spot") {
      light.type = LightBuilder::LT_spot;

    } else {
      light.type = LightBuilder::LT_directional;

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
    }

    builder._lights.push_back(light);
  }

  if (!builder.solve()) {
    return EC_lightmap_failed;
  }

  return EC_ok;
}
