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
#include "bitMask.h"
#include "textureStagePool.h"
#include "configVariableDouble.h"
#include "cmath.h"
#include "light.h"
#include "pointLight.h"
#include "spotlight.h"
#include "ordered_vector.h"

IMPLEMENT_CLASS(MapLightingEffect);

static PStatCollector map_lighting_coll("Cull:MapLightingEffect");
static PStatCollector map_lighting_cubemap_coll("Cull:MapLightingEffect:CubeMap");
static PStatCollector map_lighting_probe_coll("Cull:MapLightingEffect:Probe");
static PStatCollector map_lighting_light_cand_coll("Cull:MapLightingEffect:BuildLightCandidates");
static PStatCollector map_lighting_sort_cand_coll("Cull:MapLightingEffect:SortLightCandidates");
static PStatCollector map_lighting_apply_light_coll("Cull:MapLightingEffect:ApplyLights");
static ConfigVariableDouble map_lighting_effect_quantize_amount
  ("map-lighting-effect-quantize-amount", 8.0, // 8 hammer units, half a foot.
   PRC_DESC("Specifies how much to quantize node positions when considering "
            "whether or not to recompute the lighting for a node.  Node positions "
            "will be rounded to the nearest multiple of the specified amount. "
            "A higher value will make nodes have to move a further distance in "
            "order for lighting to be recomputed."));
UpdateSeq MapLightingEffect::_next_update = UpdateSeq::initial();
NodePath MapLightingEffect::_dynamic_light_root;
const MapLightingEffect *MapLightingEffect::_list = nullptr;

/**
 * The MapLightingEffect cannot be constructed directly from outside code.
 * Instead, use MapLightingEffect::make().
 */
MapLightingEffect::
MapLightingEffect() :
  _probe(nullptr),
  _probe_color(PTA_LVecBase3::empty_array(9)),
  _last_map_data(nullptr),
  _last_pos(0.0f),
  _lighting_state(RenderState::make_empty()),
  _has_lighting_origin(false),
  _lighting_origin(0.0f, 0.0f, 0.0f),
  _use_position(false),
  _max_lights(4),
  _last_update(UpdateSeq::old())
{
}

/**
 *
 */
MapLightingEffect::
~MapLightingEffect() {
  //remove_from_linked_list();
}

/**
 * Creates a new MapLightingEffect for applying to a unique node.
 */
CPT(RenderEffect) MapLightingEffect::
make(BitMask32 camera_mask, bool use_position, unsigned int flags, int max_lights) {
  MapLightingEffect *effect = new MapLightingEffect;
  effect->_camera_mask = camera_mask;
  effect->_use_position = use_position;
  effect->_flags = flags;
  effect->_max_lights = max_lights;

  //effect->add_to_linked_list();

  return return_new(effect);
}

/**
 * Creates a new MapLightingEffect for applying to a unique node.
 */
CPT(RenderEffect) MapLightingEffect::
make(BitMask32 camera_mask, const LPoint3 &lighting_origin, unsigned int flags, int max_lights) {
  MapLightingEffect *effect = new MapLightingEffect;
  effect->_camera_mask = camera_mask;
  effect->_use_position = true;
  effect->_has_lighting_origin = true;
  effect->_lighting_origin = lighting_origin;
  effect->_flags = flags;
  effect->_max_lights = max_lights;

  //effect->add_to_linked_list();

  return return_new(effect);
}

/**
 * Returns the last computed lighting state.
 */
const RenderState *MapLightingEffect::
get_current_lighting_state() const {
  return _lighting_state;
}

/**
 *
 */
void MapLightingEffect::
compute_lighting(const TransformState *net_transform, MapData *map_data,
                 const GeometricBoundingVolume *node_bounds,
                 const TransformState *parent_net_transform) const {
  ((MapLightingEffect *)this)->do_compute_lighting(net_transform, map_data, node_bounds, parent_net_transform);
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
  PStatTimer timer(map_lighting_coll);

  if (!_camera_mask.has_bits_in_common(trav->get_camera_mask())) {
    // Don't need to compute lighting for this camera.
    return true;
  }

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
 *
 */
INLINE static PN_stdfloat
quantize(PN_stdfloat value, PN_stdfloat amount) {
  return cfloor(value / amount + 0.5) * amount;
}

/**
 * Computes the lighting state and applies it to the running render state.
 */
void MapLightingEffect::
do_cull_callback(CullTraverser *trav, CullTraverserData &data,
                 CPT(TransformState) &node_transform,
                 CPT(RenderState) &node_state) {
  if (trav->get_type() != MapCullTraverser::get_class_type()) {
    return;
  }
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

  LPoint3 net_pos = net_transform->get_pos();
  double quantize_amt = map_lighting_effect_quantize_amount;
  if (quantize_amt > 0.0) {
    net_pos[0] = quantize(net_pos[0], quantize_amt);
    net_pos[1] = quantize(net_pos[1], quantize_amt);
    net_pos[2] = quantize(net_pos[2], quantize_amt);
  }

  if (!net_pos.almost_equal(_last_pos) || mdata != _last_map_data || _last_update != _next_update) {
    // Node moved or map changed.  We need to recompute its lighting
    // state.
    _last_pos = net_pos;
    _last_map_data = mdata;
    _last_update = _next_update;

    do_compute_lighting(net_transform, mdata, (const GeometricBoundingVolume *)node_reader->get_bounds(),
                        parent_net_transform);
  }

  // Lerp the probe color.
  if (_probe != nullptr) {
    float lerp_ratio = 0.15f;
    lerp_ratio = 1.0f - cpow((1.0f - lerp_ratio), (float)ClockObject::get_global_clock()->get_dt() * 30.0f);
    for (int i = 0; i < 9; ++i) {
      _probe_color[i] = _probe_color[i] * (1.0f - lerp_ratio) + _probe->_color[i] * lerp_ratio;
    }
  }

  // Put the computed map lighting state onto the running render state.
  data._state = data._state->compose(_lighting_state);
}

class LightCandidate {
public:
  PandaNode *_light;
  PN_stdfloat _metric;

  INLINE LightCandidate(PandaNode *light, const LPoint3 &pos, PN_stdfloat metric = -1.0f) :
    _light(light)
  {
    if (metric < 0.0f) {
      calc_metric(pos);
    } else {
      _metric = metric;
    }
  }

  INLINE void calc_metric(const LPoint3 &pos) {
    _metric = (pos - _light->get_transform()->get_pos()).length_squared();
  }

  INLINE bool operator < (const LightCandidate &other) const {
    if (_metric != other._metric) {
      return _metric < other._metric;
    }
    return _light < other._light;
  }
};

/**
 *
 */
void MapLightingEffect::
do_compute_lighting(const TransformState *net_transform, MapData *mdata,
                    const GeometricBoundingVolume *bounds, const TransformState *parent_net_transform) {
  // FIXME: This is most definitely slow.

  PStatTimer timer(map_lighting_coll);

  static TextureStage *cm_ts = TextureStagePool::get_stage(new TextureStage("envmap"));

  // Determine the lighting origin.  This the world-space geometric center
  // of the node's external bounding volume, or, if a lighting origin was
  // specified, the node's current net transform offset by the lighting
  // origin.
  LPoint3 pos;
  if (_has_lighting_origin || _use_position) {
    if (!_has_lighting_origin) {
      pos = net_transform->get_pos();
    } else {
      pos = net_transform->get_mat().xform_point(_lighting_origin);
    }
    pos[2] += 0.1f;

  } else {
    if (!bounds->is_infinite() && !bounds->is_empty()) {
      pos = bounds->get_approx_center();

      // Move it into world-space if not already.
      if (!parent_net_transform->is_identity()) {
        parent_net_transform->get_mat().xform_point_in_place(pos);
      }

    } else {
      pos = net_transform->get_pos();
    }
  }


  int cluster = mdata->get_area_cluster_tree()->get_leaf_value_from_point(pos);

  mdata->check_lighting_pvs();

  // Locate closest cube map texture.
  map_lighting_cubemap_coll.start();
  Texture *closest = nullptr;
  PN_stdfloat closest_dist = 1e24;
  if (_flags & F_cube_map) {
    if (cluster >= 0 && !mdata->_cube_map_pvs[cluster].empty()) {
      for (size_t i = 0; i < mdata->_cube_map_pvs[cluster].size(); i++) {
        const MapCubeMap *mcm = mdata->get_cube_map(mdata->_cube_map_pvs[cluster][i]);
        PN_stdfloat dist = (pos - mcm->_pos).length_squared();
        if (dist < closest_dist) {
          closest = mcm->_texture;
          closest_dist = dist;
        }
      }

    } else {
      for (size_t i = 0; i < mdata->_cube_maps.size(); i++) {
        const MapCubeMap *mcm = mdata->get_cube_map(i);
        PN_stdfloat dist = (pos - mcm->_pos).length_squared();
        if (dist < closest_dist) {
          closest = mcm->_texture;
          closest_dist = dist;
        }
      }
    }
  }
  map_lighting_cubemap_coll.stop();

  RayTraceScene *rt_scene = mdata->get_trace_scene();

  // Located closest ambient probe.
  map_lighting_probe_coll.start();
  closest_dist = 1e24;
  const MapAmbientProbe *closest_probe = nullptr;

  if (_flags & F_probe) {
    //bool closest_probe_visible = false;
    if (cluster >= 0 && !mdata->_probe_pvs[cluster].empty()) {
      for (size_t i = 0; i < mdata->_probe_pvs[cluster].size(); i++) {
        const MapAmbientProbe *map = mdata->get_ambient_probe(mdata->_probe_pvs[cluster][i]);
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
    } else {
      for (size_t i = 0; i < mdata->_ambient_probes.size(); i++) {
        const MapAmbientProbe *map = mdata->get_ambient_probe(i);
        PN_stdfloat dist = (pos - map->_pos).length_squared();
        if (dist < closest_dist) {
          // Check that we can actually trace to the probe.
          //RayTraceHitResult ret;
          //ret = rt_scene->trace_line(pos, map->_pos, 3);
          //if (!ret.hit) {
            // Probe is visible from sample point, we can use it.
            closest_probe = map;
            closest_dist = dist;
          //}
        }
      }
    }
  }
  map_lighting_probe_coll.stop();

  CPT(RenderState) state = _lighting_state;

  if (closest != _cube_map && closest != nullptr) {
    _cube_map = closest;
    CPT(RenderAttrib) tattr = TextureAttrib::make();
    tattr = DCAST(TextureAttrib, tattr)->add_on_stage(cm_ts, closest);
    state = state->set_attrib(tattr);
  }

  if (closest_probe != _probe && closest_probe != nullptr) {
    if (_probe == nullptr) {
      // Apply it immediately if we don't currently have a probe,
      // otherwise it will smoothly lerp to the new probe.
      for (int i = 0; i < 9; i++) {
        _probe_color[i] = closest_probe->_color[i];
      }
    }
    _probe = closest_probe;
    if (!_lighting_state->has_attrib(ShaderAttrib::get_class_slot())) {
      CPT(RenderAttrib) sattr = ShaderAttrib::make();
      sattr = DCAST(ShaderAttrib, sattr)->set_shader_input(ShaderInput("ambientProbe", _probe_color));
      state = state->set_attrib(sattr);
    }
  }

  // We build up our light set then pass it into a single LightAttrib::make()
  // operation, rather than calling add_light() for each light.
  //ov_set<NodePath> lights;
  //lights.reserve(_max_lights);
  //bool lights_changed = false;
  //int num_added_lights = 0;

  // Algorithm:
  // - Build vector of light candiates, includes sun light, static lights, and dynamic lights.
  // - Sort light candidates by distance/importance.
  // - Build light list by iterating through sorted candidate list, and adding each one
  //   that can be traced to until we hit the light budget.

  map_lighting_light_cand_coll.start();
  pvector<LightCandidate> lights;
  lights.reserve(256);

  // First, add the sun, if it's visible from the node position.
  // We test sun visibility by tracing towards the sun, and checking
  // that it hit a skybox face.

  if (!(_flags & F_no_sun) && !mdata->_dir_light.is_empty()) {
    bool sees_sky = false;
    if (_flags & F_force_sun) {
      sees_sky = true;

    } else {
      RayTraceHitResult ret = rt_scene->trace_ray(pos, -mdata->_dir_light_dir, 999999, 3);
      if (ret.hit) {
        RayTraceGeometry *geom = rt_scene->get_geometry(ret.geom_id);
        if ((geom->get_mask() & 2) != 0) {
          // Hit sky, sun is visible.
          sees_sky = true;
        }
      } else {
        // No hit = sky.
        sees_sky = true;
      }
    }

    if (sees_sky) {
      lights.push_back(LightCandidate(mdata->_dir_light.node(), pos, 0.0f));
    }
  }

  // Now add all non-sun static lights in the PVS of the node position.
  if (cluster >= 0 && (_flags & F_static_lights)) {
    for (int light_idx : mdata->_light_pvs[cluster]) {
      lights.push_back(LightCandidate(mdata->_lights[light_idx].node(), pos));
    }
  }

  // Dynamic lights.
  if ((_flags & F_dynamic_lights) && !_dynamic_light_root.is_empty()) {
    PandaNode::Children dyn_lights = _dynamic_light_root.node()->get_children();
    // Add in dynamic light sources.
    for (int i = 0; i < dyn_lights.get_num_children(); ++i) {
      PandaNode *child = dyn_lights.get_child(i);
      Light *light = child->as_light();
      if (light == nullptr) {
        continue;
      }

      PN_stdfloat max_distance;
      if (light->get_light_type() == Light::LT_point) {
        PointLight *pl = DCAST(PointLight, child);
        max_distance = pl->get_max_distance();
      } else if (light->get_light_type() == Light::LT_spot) {
        Spotlight *sl = DCAST(Spotlight, child);
        max_distance = sl->get_max_distance();
      } else {
        // This light type is not supported for dynamic lights.
        continue;
      }

      PN_stdfloat metric = (pos - child->get_transform()->get_pos()).length_squared();
      if (metric >= max_distance*max_distance) {
        continue;
      }
      lights.push_back(LightCandidate(child, pos, metric));
    }
  }

  map_lighting_light_cand_coll.stop();

  // Sort light candidates by increasing metric.
  map_lighting_sort_cand_coll.start();
  std::sort(lights.begin(), lights.end());
  map_lighting_sort_cand_coll.stop();

  if (!lights.empty()) {
    map_lighting_apply_light_coll.start();

    // Apply the most important lights, up to _max_lights lights.
    ov_set<NodePath> light_set;
    light_set.reserve(_max_lights);
    for (int i = 0; i < (int)lights.size() && i < _max_lights; ++i) {
      light_set.push_back(NodePath(lights[i]._light));
    }
    std::sort(light_set.begin(), light_set.end());
    state = state->set_attrib(LightAttrib::make(std::move(light_set)));

    map_lighting_apply_light_coll.stop();
  }

  _lighting_state = state;
}

/**
 *
 */
void MapLightingEffect::
add_to_linked_list() {
  if (_list == nullptr) {
    _list = this;
  } else {
    _next = _list;
    _list = this;
  }
}

/**
 *
 */
void MapLightingEffect::
remove_from_linked_list() {
  const MapLightingEffect *prev = nullptr;
  const MapLightingEffect *mle = _list;
  while (mle != this && mle != nullptr) {
    prev = mle;
    mle = mle->_next;
  }
  if (prev != nullptr) {
    ((MapLightingEffect *)prev)->_next = mle->_next;
  } else {
    _list = mle->_next;
  }
}

/**
 *
 */
void MapLightingEffect::
mark_stale() {
  _next_update++;
}

/**
 *
 */
void MapLightingEffect::
set_dynamic_light_root(NodePath np) {
  _dynamic_light_root = np;
}

/**
 *
 */
void MapLightingEffect::
clear_dynamic_light_root() {
  _dynamic_light_root.clear();
}
