/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullLights.cxx
 * @author lachbr
 * @date 2020-12-10
 */

#include "cullLights.h"
#include "lightAttrib.h"
#include "cullTraverserData.h"
#include "light.h"

/**
 * Returns a pointer to an empty CullLights object.
 */
CPT(CullLights) CullLights::
make_empty() {
  static CPT(CullLights) empty;
  if (empty == nullptr) {
    empty = new CullLights;
    // Artificially tick the reference count, just to ensure we won't
    // accidentally modify this object in any of the copy-on-write operations
    // below.
    empty->ref();
  }
  return empty;
}

/**
 * Returns a pointer to a new CullLights object that is the same as this one,
 * but with the lights modified by the indicated transform.
 */
CPT(CullLights) CullLights::
xform(const LMatrix4 &mat) const {
  PT(CullLights) new_lights;
  if (get_ref_count() == 1) {
    new_lights = (CullLights *)this;
  } else {
    new_lights = new CullLights(*this);
  }

  for (Lights::iterator li = new_lights->_lights.begin();
       li != new_lights->_lights.end();
       ++li) {
    if ((*li).second->get_ref_count() != 1) {
      (*li).second = DCAST(GeometricBoundingVolume, (*li).second->make_copy());
    }
    (*li).second->xform(mat);
  }

  return new_lights;
}

/**
 * Returns a pointer to a new CullLights object that is the same as this one,
 * but with the indicated attributes applied to the state.
 *
 * In particular, any new Lights given in net_attrib, if it is not NULL and not
 * off, will be added to the state.
 */
CPT(CullLights) CullLights::
apply_state(const CullTraverser *trav, const CullTraverserData* data,
            const LightAttrib *net_attrib) const {
  if (net_attrib == nullptr) {
    return this;
  }

  PT(CullLights) new_lights;
  if (get_ref_count() == 1) {
    new_lights = (CullLights *)this;
  } else {
    new_lights = new CullLights(*this);
  }

  CPT(TransformState) net_transform = nullptr;

  if (net_attrib != nullptr) {
    int num_on_lights = net_attrib->get_num_on_lights();
    for (int i = 0; i < num_on_lights; i++) {
      NodePath light = net_attrib->get_on_light(i);
      Lights::const_iterator li = new_lights->_lights.find(light);
      if (li == new_lights->_lights.end()) {
        if (!net_attrib->has_off_light(light)) {
          // Here's a new light; add it to the list.  For this we need the net
          // transform to this node.
          if (net_transform == nullptr) {
            net_transform = data->get_net_transform(trav);
          }

          Light *light_node = light.node()->as_light();
          CPT(TransformState) new_transform =
            net_transform->invert_compose(light.get_net_transform());

          PT(GeometricBoundingVolume) gbv = light_node->make_light_bounds();
          gbv->xform(new_transform->get_mat());
          new_lights->_lights[light] = gbv;
        }
      }
    }
  }

  return new_lights;
}

/**
 * Tests the indicated bounding volume against all of the lights in this
 * object.  Sets result to an appropriate union of
 * BoundingVolume::IntersectionFlags, similar to the result of
 * BoundingVolume::contains().
 *
 * Also, if the bounding volume is outside of any of the lights,
 * removes those lights both from this object and from the indicated
 * state, returning a new CullLights object in that case.
 */
CPT(CullLights) CullLights::
do_cull(int &result, CPT(RenderState) &state,
        const GeometricBoundingVolume *node_gbv) const {
  result =
    BoundingVolume::IF_all | BoundingVolume::IF_possible | BoundingVolume::IF_some;

  CPT(CullLights) new_lights = this;

  const LightAttrib *orig_la;
  if (!state->get_attrib(orig_la)) {
    // If there are no lights in this state, the node is completely inside all
    // zero of the lights.  (This can happen if someone directly changes the
    // state during the traversal.)
    CullLights *lights = new CullLights;
    new_lights = lights;

  } else {
    CPT(LightAttrib) new_la = orig_la;

    Lights::const_iterator li;
    for (li = _lights.begin(); li != _lights.end(); ++li) {
      int light_result = (*li).second->contains(node_gbv);
      if (light_result == BoundingVolume::IF_no_intersection) {
        // The node is completely outside of the light's bounding volume, and
        // the light gets culled.  We don't need to consider this light ever
        // again for any descendents of this node.
        new_lights = new_lights->remove_light((*li).first);
        nassertr(new_lights != this, new_lights);
        new_la = DCAST(LightAttrib, new_la->remove_on_light((*li).first));
      } else if ((light_result & BoundingVolume::IF_all) != 0) {
        // The node is completely inside the light's bounding volume.  The
        // light is kept on the state, and we don't need to consider this light
        // ever again for any descendents of this node.
        new_lights = new_lights->remove_light((*li).first);
        nassertr(new_lights != this, new_lights);
      }

      result &= light_result;
    }

    if (new_la != orig_la) {
      if (new_la->is_identity()) {
        state = state->remove_attrib(LightAttrib::get_class_slot());
      } else {
        state = state->add_attrib(new_la);
      }
    }
  }

  return new_lights;
}

/**
 * Returns a pointer to a new CullLights object that is the same as this one,
 * but with the indicated light removed.
 */
CPT(CullLights) CullLights::
remove_light(const NodePath &light) const {
  PT(CullLights) new_lights;
  if (get_ref_count() == 1) {
    new_lights = (CullLights *)this;
  } else {
    new_lights = new CullLights(*this);
  }

  Lights::iterator li = new_lights->_lights.find(light);
  nassertr(li != new_lights->_lights.end(), new_lights);
  new_lights->_lights.erase(li);

  return new_lights;
}

/**
 *
 */
void CullLights::
write(std::ostream &out) const {
  out << "CullLights (" << _lights.size() << " lights)\n";
  Lights::const_iterator li;
  for (li = _lights.begin(); li != _lights.end(); ++li) {
    out << "  " << (*li).first << " : " << *(*li).second << "\n";
  }
}
