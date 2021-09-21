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

  VisBuilder vis(this);
  if (!vis.build()) {
    return EC_unknown_error;
  }

  // Now write out the meshes to GeomNodes.

  // Start with the mesh groups.
  int mesh_group_index = 0;
  for (auto it = vis._vis_mesh_groups.begin(); it != vis._vis_mesh_groups.end(); ++it) {
    const VisMeshGroup &group = it->second;

    MapMeshGroup out_group;
    for (auto cit = it->first.begin(); cit != it->first.end(); ++cit) {
      out_group._clusters.set_bit(*cit);
    }
    _out_data->add_mesh_group(out_group);

    // Now build the Geoms within the group.

    std::ostringstream ss;
    ss << "mesh-group-" << mesh_group_index;
    PT(GeomNode) geom_node = new GeomNode(ss.str());
    //geom_node->set_attrib(ColorAttrib::make_flat(cluster_colors[mesh_group_index % 6]));

    PT(GeomVertexData) vdata = new GeomVertexData(
      geom_node->get_name(), GeomVertexFormat::get_v3n3t2(),
      GeomEnums::UH_static);

    PT(PhysTriangleMeshData) phys_mesh_data = new PhysTriangleMeshData;
    int phys_polygons = 0;

    // The world mesh is assigned to mesh groups on a per-polygon basis.
    for (MapPoly *poly : group.world_polys) {
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

    // These are non-world meshes, like func_details.  They are treated as
    // single units.
    for (MapMesh *mesh : group.meshes) {
      for (MapPoly *poly : mesh->_polys) {
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
    }

    if (phys_polygons > 0) {
      // Cook the physics mesh.
      if (!phys_mesh_data->cook_mesh()) {
        mapbuilder_cat.error()
          << "Failed to cook physics mesh for mesh group " << mesh_group_index << "\n";
        _out_data->add_model_phys_data(CPTA_uchar());
      } else {
        _out_data->add_model_phys_data(phys_mesh_data->get_mesh_data());
      }
    } else {
      _out_data->add_model_phys_data(CPTA_uchar());
    }

    // Try to combine all the polygon Geoms into as few Geoms as possible.
    geom_node->unify(UINT16_MAX, false);

    // The node we parent the mesh group to will decide which mesh group(s) to
    // render based on the current view cluster.
    _out_node->add_child(geom_node);

    mesh_group_index++;
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
  vwriter.set_row(start);
  nwriter.set_row(start);
  twriter.set_row(start);

  const Winding *w = &(poly->_winding);
  LVector3 normal = -(w->get_plane().get_normal());

  Material *mat = poly->_material;

  // Fill up the render state for the polygon.
  CPT(RenderState) state = RenderState::make_empty();
  Texture *tex = nullptr;
  LVecBase2i tex_dim(1, 1);
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

    // Check if the render state needs transparency.
    MaterialParamBase *base_p = mat->get_param("base_color");
    if (base_p != nullptr && base_p->is_of_type(MaterialParamTexture::get_class_type())) {
      MaterialParamTexture *base_tex_p = DCAST(MaterialParamTexture, base_p);
      tex = base_tex_p->get_value();
      if (tex != nullptr && Texture::has_alpha(tex->get_format())) {
        state = state->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_dual));
      }

      if (tex != nullptr) {
        // Extract texture dimensions to calculate UVs.
        tex_dim[0] = tex->get_orig_file_x_size();
        tex_dim[1] = tex->get_orig_file_y_size();
      }
    }
  }

  for (size_t k = 0; k < w->get_num_points(); k++) {
    vwriter.add_data3f(w->get_point(k));
    nwriter.add_data3f(normal);

    // Calcuate the UV coordinate for the vertex.
    LVecBase2 uv(
      poly->_texture_vecs[0].get_xyz().dot(w->get_point(k)) + poly->_texture_vecs[0][3],
      poly->_texture_vecs[1].get_xyz().dot(w->get_point(k)) + poly->_texture_vecs[1][3]
    );
    uv[0] /= tex_dim[0];
    uv[1] /= -tex_dim[1];
    twriter.add_data2f(uv);
  }

  PT(GeomTriangles) tris = new GeomTriangles(GeomEnums::UH_static);
  for (size_t k = 1; k < (w->get_num_points() - 1); k++) {
    tris->add_vertices(start + k + 1, start + k, start);
    tris->close_primitive();
  }

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
  ThreadManager::run_threads_on_individual(
    "BuildPolygons", _source_map->_entities.size(),
    false, std::bind(&MapBuilder::build_entity_polygons, this, std::placeholders::_1));
  return EC_ok;
}

/**
 *
 */
void MapBuilder::
build_entity_polygons(int i) {
  MapEntitySrc *ent = _source_map->_entities[i];

  if (ent->_solids.size() == 0) {
    return;
  }

  PT(MapMesh) ent_mesh = new MapMesh;
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

      PT(MapPoly) poly = new MapPoly;
      poly->_winding = w;
      poly->_material = poly_material;

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

      LPoint3 origin(0);
      poly->_texture_vecs[0][0] = side->_u_axis[0] / side->_uv_scale[0];
      poly->_texture_vecs[0][1] = side->_u_axis[1] / side->_uv_scale[0];
      poly->_texture_vecs[0][2] = side->_u_axis[2] / side->_uv_scale[0];
      poly->_texture_vecs[0][3] = side->_uv_shift[0] + origin.dot(poly->_texture_vecs[0].get_xyz());
      poly->_texture_vecs[1][0] = side->_v_axis[0] / side->_uv_scale[1];
      poly->_texture_vecs[1][1] = side->_v_axis[1] / side->_uv_scale[1];
      poly->_texture_vecs[1][2] = side->_v_axis[2] / side->_uv_scale[1];
      poly->_texture_vecs[1][3] = side->_uv_shift[1] + origin.dot(poly->_texture_vecs[1].get_xyz());
      solid_polys.push_back(poly);

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
