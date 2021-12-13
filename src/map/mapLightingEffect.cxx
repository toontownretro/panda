/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapLightingEffect.cxx
 * @author brian
 * @date 2021-12-06
 */

#include "mapLightingEffect.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "lightAttrib.h"
#include "pandaNode.h"
#include "luse.h"
#include "geometricBoundingVolume.h"
#include "nodePath.h"
#include "mapData.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "shaderAttrib.h"
#include "pvector.h"
#include "mapCullTraverser.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "directionalLight.h"

IMPLEMENT_CLASS(MapLightingEffect);

static PStatCollector map_lighting_coll("Cull:MapLightingEffect");

/**
 * The MapLightingEffect cannot be constructed directly from outside code.
 * Instead, use MapLightingEffect::make().
 */
MapLightingEffect::
MapLightingEffect() :
  _probe(nullptr),
  _probe_color(PTA_LVecBase3::empty_array(9)),
  _last_map_data(nullptr),
  _lighting_state(RenderState::make_empty())
{
}

/**
 * Creates a new MapLightingEffect for applying to a unqiue node.
 */
CPT(RenderEffect) MapLightingEffect::
make() {
  MapLightingEffect *effect = new MapLightingEffect;
  return return_new(effect);
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this effect during the cull traversal.
 */
bool MapLightingEffect::
has_cull_callback() const {
  return true;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.  This may include additional manipulation of render
 * state or additional visible/invisible decisions, or any other arbitrary
 * operation.
 *
 * At the time this function is called, the current node's transform and state
 * have not yet been applied to the net_transform and net_state.  This
 * callback may modify the node_transform and node_state to apply an effective
 * change to the render state at this level.
 */
bool MapLightingEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  ((MapLightingEffect *)this)->do_cull_callback(trav, data, node_transform, node_state);
  return true;
}

/**
 *
 */
int MapLightingEffect::
compare_to_impl(const RenderEffect *other) const {
  if (this != other) {
    return this < other ? -1 : 1;
  }
  return 0;
}

/**
 * Computes the lighting state and applies it to the running render state.
 */
void MapLightingEffect::
do_cull_callback(CullTraverser *trav, CullTraverserData &data,
                 CPT(TransformState) &node_transform,
                 CPT(RenderState) &node_state) {

  // FIXME: This is most definitely slow.

  PStatTimer timer(map_lighting_coll);

  static PT(TextureStage) cm_ts = new TextureStage("envmap");

  // Assume we have a MapCullTraverser.  This will crash if it's not,
  // but I don't want to spend time checking if it is.
  MapCullTraverser *mtrav = (MapCullTraverser *)trav;
  MapData *mdata = mtrav->_data;

  if (mdata == nullptr) {
    // No active map data.  Nothing to compute lighting state from.
    return;
  }

  // If the net state or the node's state turns off lights, don't do
  // anything.
  const LightAttrib *la;
  data._state->get_attrib_def(la);
  if (la->has_all_off()) {
    return;
  }
  node_state->get_attrib_def(la);
  if (la->has_all_off()) {
    return;
  }

  const TransformState *parent_net_transform = data._net_transform;

  CPT(TransformState) net_transform;
  if (parent_net_transform->is_identity()) {
    net_transform = node_transform;
  } else {
    net_transform = parent_net_transform->compose(node_transform);
  }

  PandaNode *node = data.node();
  PandaNodePipelineReader *node_reader = data.node_reader();

  if (net_transform != _last_transform || mdata != _last_map_data) {
    // Node moved or map changed.  We need to recompute its lighting
    // state.

    _last_transform = net_transform;
    _last_map_data = mdata;

    // Determine the lighting origin.  This the world-space geometric center
    // of the node's external bounding volume.
    LPoint3 pos;
    const GeometricBoundingVolume *bounds = (const GeometricBoundingVolume *)node_reader->get_bounds();
    if (!bounds->is_infinite()) {
      pos = bounds->get_approx_center();

      // Move it into world-space if not already.
      if (!parent_net_transform->is_identity()) {
        parent_net_transform->get_mat().xform_point_in_place(pos);
      }

    } else {
      pos = net_transform->get_pos();
    }

    // Locate closest cube map texture.
    Texture *closest = nullptr;
    PN_stdfloat closest_dist = 1e24;
    for (size_t i = 0; i < mdata->get_num_cube_maps(); i++) {
      const MapCubeMap *mcm = mdata->get_cube_map(i);
      PN_stdfloat dist = (pos - mcm->_pos).length_squared();
      if (dist < closest_dist) {
        closest = mcm->_texture;
        closest_dist = dist;
      }
    }

    RayTraceScene *rt_scene = mdata->get_trace_scene();

    // Located closest ambient probe.
    closest_dist = 1e24;
    const MapAmbientProbe *closest_probe = nullptr;
    for (size_t i = 0; i < mdata->get_num_ambient_probes(); i++) {
      const MapAmbientProbe *map = mdata->get_ambient_probe(i);
      PN_stdfloat dist = (pos - map->_pos).length_squared();
      if (dist < closest_dist) {
        // Check that we can actually trace to the probe.
        RayTraceHitResult ret;
        ret = rt_scene->trace_line(pos, map->_pos, 3);
        if (!ret.hit) {
          // Probe is visible from sample point, we can use it.
          closest_probe = map;
          closest_dist = dist;
        }
      }
    }

    CPT(RenderState) state = RenderState::make_empty();

    if (closest != _cube_map && closest != nullptr) {
      _cube_map = closest;
      CPT(RenderAttrib) tattr = TextureAttrib::make();
      tattr = DCAST(TextureAttrib, tattr)->add_on_stage(cm_ts, closest);
      state = state->set_attrib(tattr);

    } else {
      CPT(RenderAttrib) curr_tex = _lighting_state->get_attrib(TextureAttrib::get_class_slot());
      if (curr_tex != nullptr) {
        state = state->set_attrib(curr_tex);
      }

    }

    if (closest_probe != _probe && closest_probe != nullptr) {
      _probe = closest_probe;
      for (int i = 0; i < 9; i++) {
        _probe_color[i] = closest_probe->_color[i];
      }
      CPT(RenderAttrib) sattr = ShaderAttrib::make();
      sattr = DCAST(ShaderAttrib, sattr)->set_shader_input(ShaderInput("ambientProbe", _probe_color));
      state = state->set_attrib(sattr);

    } else {
      CPT(RenderAttrib) curr_shad = _lighting_state->get_attrib(ShaderAttrib::get_class_slot());
      if (curr_shad != nullptr) {
        state = state->set_attrib(curr_shad);
      }
    }

    pvector<NodePath> sorted_lights;
    sorted_lights.reserve(mdata->get_num_lights());
    for (int i = 0; i < mdata->get_num_lights(); i++) {
      if (mdata->get_light(i).node()->is_of_type(DirectionalLight::get_class_type())) {
        continue;
      }
      sorted_lights.push_back(mdata->get_light(i));
    }
    std::sort(sorted_lights.begin(), sorted_lights.end(), [pos](const NodePath &a, const NodePath &b) -> bool {
      return (pos - a.get_pos()).length_squared() < (pos - b.get_pos()).length_squared();
    });

    CPT(RenderAttrib) lattr = LightAttrib::make();
    int num_added_lights = 0;
    if (!mdata->_dir_light.is_empty()) {
      RayTraceHitResult ret = rt_scene->trace_ray(pos, -mdata->_dir_light_dir, 999999, 3);
      if (ret.hit) {
        RayTraceGeometry *geom = rt_scene->get_geometry(ret.geom_id);
        if ((geom->get_mask() & 2) != 0) {
          // Hit sky, sun is visible.
          lattr = DCAST(LightAttrib, lattr)->add_on_light(mdata->_dir_light);
          num_added_lights++;
        }
      }
    }
    for (size_t i = 0; num_added_lights < 4 && i < sorted_lights.size(); i++) {
      lattr = DCAST(LightAttrib, lattr)->add_on_light(sorted_lights[i]);
      num_added_lights++;
    }
    state = state->set_attrib(lattr);

    _lighting_state = state;
  }

  // Put the computed map lighting state onto the running render state.
  data._state = data._state->compose(_lighting_state);
}
