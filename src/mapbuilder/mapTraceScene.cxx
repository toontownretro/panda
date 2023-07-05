/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapTraceScene.cxx
 * @author brian
 * @date 2023-06-21
 */

#include "mapTraceScene.h"
#include "mapBuilder.h"
#include "loader.h"
#include "nodePath.h"
#include "rayTrace.h"

/**
 *
 */
MapTraceScene::
MapTraceScene(MapData *data) :
  _data(data)
{
}

/**
 * Builds the trace scene.
 */
void MapTraceScene::
build(unsigned int mask) {
  RayTrace::initialize();

  _scene = new RayTraceScene;

  if (mask & M_world) {
    _mesh_world = new RayTraceTriangleMesh;
    _mesh_world->set_mask(M_world);
    // Add world level geometry.
    const MapModel *world_model = _data->get_model(0);
    GeomNode *child = world_model->get_geom_node();
    for (int j = 0; j < child->get_num_geoms(); j++) {
      _mesh_world->add_triangles_from_geom(child->get_geom(j));
    }
  }

  if (mask & M_static_prop) {
    _mesh_props = new RayTraceTriangleMesh;
    _mesh_props->set_mask(M_static_prop);
    // Add static props (lowest LOD).
    for (int i = 0; i < _data->get_num_static_props(); i++) {
      const MapStaticProp *sprop = _data->get_static_prop(i);
      PT(PandaNode) prop_model_node = Loader::get_global_ptr()->load_sync(sprop->get_model_filename());
      if (prop_model_node == nullptr) {
        continue;
      }
      NodePath prop_model(prop_model_node);
      prop_model.set_pos(sprop->get_pos());
      prop_model.set_hpr(sprop->get_hpr());

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
          _mesh_props->add_triangles_from_geom(geom_node->get_geom(k));
        }
      }
    }
  }
}

/**
 *
 */
bool MapTraceScene::
hits_backface(const LPoint3 &start, const LPoint3 &end, unsigned int mask) {
  RayTraceHitResult ret = _scene->trace_line(start, end, mask);
  if (!ret.has_hit()) {
    return false;
  }

  LVector3 ray_dir = (end - start).normalized();

  if (ret.get_hit_normal().dot(ray_dir) >= 0.0f) {
    // If triangle normal is same direction as ray, we hit backface.
    return true;
  }

  return false;
}
