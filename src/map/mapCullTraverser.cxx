/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapCullTraverser.cxx
 * @author brian
 * @date 2021-10-15
 */

#include "mapCullTraverser.h"
#include "cullTraverserData.h"
#include "pandaNode.h"
#include "mapData.h"
#include "mapNodeData.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "lightAttrib.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "shaderAttrib.h"

IMPLEMENT_CLASS(MapCullTraverser);

/**
 *
 */
MapCullTraverser::
MapCullTraverser(const CullTraverser &copy, MapData *data) :
  CullTraverser(copy),
  _data(data),
  _view_cluster(-1)
{
  //set_custom_is_in_view(true);
}

#if 0
/**
 *
 */
int MapCullTraverser::
custom_is_in_view(const CullTraverserData &data, const PandaNodePipelineReader &node_reader,
                  const TransformState *net_transform) {
  if (_view_cluster == -1) {
    return BoundingVolume::IF_all;
  }

  node_reader.check_cached(true);

  const PandaNode *node = node_reader.get_node();

  // Look up cached node data.
  MapNodeData *ndata = nullptr;
  TypedReferenceCount *udata = node->get_user_data();
  if (udata == nullptr) {
    PT(MapNodeData) new_data = new MapNodeData;
    new_data->_map_data = nullptr;
    ((PandaNode *)node)->set_user_data(new_data);
    ndata = new_data;
  } else {
    ndata = (MapNodeData *)udata;
  }

  const BoundingVolume *bounds = node_reader.get_bounds();
  if (net_transform != ndata->_net_transform ||
      bounds != ndata->_bounds ||
      _data != ndata->_map_data) {
    // Node area clusters are stale.  Recompute them.
    ndata->_net_transform = net_transform;
    ndata->_bounds = bounds;
    ndata->_map_data = _data;
    ndata->_clusters.clear();

    // Test the node against the cluster tree.
    CPT(GeometricBoundingVolume) vol;

    if (!net_transform->is_identity()) {
      // The node has a non-identity net transform.  Make a copy of
      // the bounds and transform it into world space.
      PT(GeometricBoundingVolume) gbv = DCAST(GeometricBoundingVolume,
        bounds->make_copy());
      gbv->xform(net_transform->get_mat());
      vol = gbv;

    } else {
      vol = DCAST(GeometricBoundingVolume, bounds);
    }

    ndata->_net_bounds = vol;

    const KDTree *tree = _data->get_area_cluster_tree();
    tree->get_leaf_values_containing_volume(vol, ndata->_clusters);
  }

  if (!_pvs.has_bits_in_common(ndata->_clusters)) {
    // Node is not in the current PVS.  Cull.
    return BoundingVolume::IF_no_intersection;

  } else if ((_pvs | ndata->_clusters) == _pvs) {
    // If the OR of our PVS and the clusters that the node occupies yields
    // the PVS, we know that the node and everything below it is completely
    // contained within the PVS, so we don't have to test any further.
    return BoundingVolume::IF_all;

  } else {
    // The node is within the PVS, but there might be a descendent that is not,
    // so we should keep testing further down the graph.
    return BoundingVolume::IF_some;
  }
}
#endif

/**
 * Computes the lighting information for the given model node.
 */
void MapCullTraverser::
update_model_lighting(CullTraverserData &data) {
  static PT(TextureStage) cm_ts = new TextureStage("envmap");

  const LightAttrib *la;
  data._state->get_attrib_def(la);

  if (la->has_all_off()) {
    return;
  }

  PandaNode *node = data.node();
  PandaNodePipelineReader *node_reader = data.node_reader();

  MapNodeData *ndata = nullptr;
  TypedReferenceCount *udata = node->get_user_data();
  if (udata == nullptr) {
    PT(MapNodeData) new_data = new MapNodeData;
    new_data->_map_data = nullptr;
    node->set_user_data(new_data);
    ndata = new_data;
  } else {
    ndata = (MapNodeData *)udata;
  }

  if (ndata->_light_data == nullptr) {
    ndata->_light_data = new MapLightData;
    ndata->_light_data->_probe = nullptr;
    ndata->_light_data->_probe_color = PTA_LVecBase3::empty_array(9);
  }

  bool transform_changed = data._net_transform != ndata->_light_data->_net_transform;

  if (transform_changed) {
    // Use the world-space center of the node's bounding volume to determine
    // the lighting for the node.  The net transform of the node might not be
    // the best way to figure out where the node is located, for instance
    // if the node is flattened.
    LPoint3 pos;
    if (ndata->_net_bounds != nullptr) {
      // Great!  We already computed the world-space bounding volume of the
      // node when we cull-tested the node.  Get center point from that.
      if (!ndata->_net_bounds->is_infinite()) {
        pos = ndata->_net_bounds->get_approx_center();
      } else {
        pos = data._net_transform->get_pos();
      }

    } else {
      // We don't know the world-space bounding volume of this node.
      // Get local bounds in parent-space and transform into world-space.
      const GeometricBoundingVolume *bounds = node_reader->get_bounds()->as_geometric_bounding_volume();
      if (!bounds->is_infinite()) {
        pos = bounds->get_approx_center();
        NodePath parent_path = data.get_node_path().get_parent();
        if (!parent_path.is_empty()) {
          CPT(TransformState) parent_net = parent_path.get_net_transform();
          if (!parent_net->is_identity()) {
            parent_net->get_mat().xform_point_in_place(pos);
          }
        }

      } else {
        pos = data._net_transform->get_pos();
      }
    }

    ndata->_light_data->_net_transform = data._net_transform;

    // Locate closest cube map texture.
    Texture *closest = nullptr;
    PN_stdfloat closest_dist = 1e24;
    for (size_t i = 0; i < _data->get_num_cube_maps(); i++) {
      const MapCubeMap *mcm = _data->get_cube_map(i);
      PN_stdfloat dist = (pos - mcm->_pos).length_squared();
      if (dist < closest_dist) {
        closest = mcm->_texture;
        closest_dist = dist;
      }
    }

    // Located closest ambient probe.
    closest_dist = 1e24;
    const MapAmbientProbe *closest_probe = nullptr;
    for (size_t i = 0; i < _data->get_num_ambient_probes(); i++) {
      const MapAmbientProbe *map = _data->get_ambient_probe(i);
      PN_stdfloat dist = (pos - map->_pos).length_squared();
      if (dist < closest_dist) {
        closest_probe = map;
        closest_dist = dist;
      }
    }

    CPT(RenderState) state = RenderState::make_empty();

    if (closest != ndata->_light_data->_cube_map && closest != nullptr) {
      ndata->_light_data->_cube_map = closest;
      CPT(RenderAttrib) tattr = TextureAttrib::make();
      tattr = DCAST(TextureAttrib, tattr)->add_on_stage(cm_ts, closest);
      state = state->set_attrib(tattr);

    } else {
      state = state->set_attrib(ndata->_light_data->_lighting_state->get_attrib(TextureAttrib::get_class_slot()));
    }

    if (closest_probe != ndata->_light_data->_probe && closest_probe != nullptr) {
      ndata->_light_data->_probe = closest_probe;
      for (int i = 0; i < 9; i++) {
        ndata->_light_data->_probe_color[i] = closest_probe->_color[i];
      }
      CPT(RenderAttrib) sattr = ShaderAttrib::make();
      sattr = DCAST(ShaderAttrib, sattr)->set_shader_input(ShaderInput("ambientProbe", ndata->_light_data->_probe_color));
      state = state->set_attrib(sattr);

    } else {
      state = state->set_attrib(ndata->_light_data->_lighting_state->get_attrib(ShaderAttrib::get_class_slot()));
    }

    pvector<NodePath> sorted_lights;
    sorted_lights.resize(_data->get_num_lights());
    for (int i = 0; i < _data->get_num_lights(); i++) {
      sorted_lights[i] = _data->get_light(i);
    }
    std::sort(sorted_lights.begin(), sorted_lights.end(), [pos](const NodePath &a, const NodePath &b) -> bool {
      return (pos - a.get_pos()).length_squared() < (pos - b.get_pos()).length_squared();
    });

    CPT(RenderAttrib) lattr = LightAttrib::make();
    for (size_t i = 0; i < 4 && i < sorted_lights.size(); i++) {
      lattr = DCAST(LightAttrib, lattr)->add_on_light(sorted_lights[i]);
    }
    state = state->set_attrib(lattr);

    ndata->_light_data->_lighting_state = state;
  }

  data._state = data._state->compose(ndata->_light_data->_lighting_state);
}

#if 0
/**
 *
 */
void MapCullTraverser::
traverse_below(CullTraverserData &data) {
  _nodes_pcollector.add_level(1);
  PandaNodePipelineReader *node_reader = data.node_reader();
  PandaNode *node = data.node();

  if (!data.is_this_node_hidden(_camera_mask)) {
    // If it's of ModelNode type, we need to update lighting information at
    // the root of this subgraph.
    if (node->is_exact_type(ModelNode::get_class_type()) || node->is_exact_type(ModelRoot::get_class_type())) {
      update_model_lighting(data);
    }

    node->add_for_draw(this, data);

    // Check for a decal effect.
    const RenderEffects *node_effects = node_reader->get_effects();
    if (node_effects->has_decal()) {
      // If we *are* implementing decals with DepthOffsetAttribs, apply it
      // now, so that each child of this node gets offset by a tiny amount.
      data._state = data._state->compose(get_depth_offset_state());
#ifndef NDEBUG
      // This is just a sanity check message.
      if (!node->is_geom_node()) {
        pgraph_cat.error()
          << "DecalEffect applied to " << *node << ", not a GeomNode.\n";
      }
#endif
    }
  }

  // Now visit all the node's children.
  PandaNode::Children children = node_reader->get_children();
  node_reader->release();
  int num_children = children.get_num_children();
  if (!node->has_selective_visibility()) {
    for (int i = 0; i < num_children; ++i) {
      const PandaNode::DownConnection &child = children.get_child_connection(i);
      traverse_child(data, child, data._state);
    }
  } else {
    int i = node->get_first_visible_child();
    while (i < num_children) {
      const PandaNode::DownConnection &child = children.get_child_connection(i);
      traverse_child(data, child, data._state);
      i = node->get_next_visible_child(i);
    }
  }
}
#endif

/**
 *
 */
void MapCullTraverser::
determine_view_cluster() {
  _view_cluster = -1;
  _pvs.clear();

  const KDTree *tree = _data->get_area_cluster_tree();
  if (tree == nullptr) {
    return;
  }

  LPoint3 view_pos = get_camera_transform()->get_pos();
  _view_cluster = tree->get_leaf_value_from_point(view_pos);
  if (_view_cluster != -1) {
    _pvs.set_bit(_view_cluster);
    const AreaClusterPVS *pvs = _data->get_cluster_pvs(_view_cluster);
    for (size_t i = 0; i < pvs->get_num_visible_clusters(); i++) {
      _pvs.set_bit(pvs->get_visible_cluster(i));
    }
  }
}
