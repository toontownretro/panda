/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pvsCullEffect.cxx
 * @author brian
 * @date 2021-11-29
 */

#include "pvsCullEffect.h"
#include "config_pgraph.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "bitArray.h"
#include "boundingBox.h"
#include "boundingSphere.h"
#include "sceneVisibility.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector pvs_cull_coll("Cull:PVSTest");

TypeHandle PVSCullEffect::_type_handle;

PVSCullEffect::
PVSCullEffect() :
  _num_sectors(0),
  _bounds(nullptr)
{
}

/**
 *
 */
CPT(RenderEffect) PVSCullEffect::
make() {
  PVSCullEffect *effect = new PVSCullEffect;
  return return_new(effect);
}

/**
 *
 */
bool PVSCullEffect::
has_cull_callback() const {
  return true;
}

/**
 *
 */
bool PVSCullEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  return ((PVSCullEffect *)this)->do_cull_callback(trav, data, node_transform, node_state);
}

/**
 *
 */
bool PVSCullEffect::
do_cull_callback(CullTraverser *trav, CullTraverserData &data,
                 CPT(TransformState) &node_transform,
                 CPT(RenderState) &node_state) {

  PStatTimer timer(pvs_cull_coll);

  if (trav->_view_sector < 0) {
    // Camera is in an invalid vis sector.  Forget it.
    return true;
  }

  const TransformState *parent_net_transform = data._net_transform;
  const GeometricBoundingVolume *bounds = (const GeometricBoundingVolume *)data._node_reader.get_bounds();
  const BitArray *pvs = trav->_pvs;
  const SceneVisibility *scene_vis = trav->_vis_info;

  // If the transform cache is in use, we can just compare the transforms by
  // pointer, as identical transforms are guaranteed to have the same pointer.
  // Otherwise, we need to compare the actual transforms.
  //static bool using_transform_cache = transform_cache;

  bool transform_changed;
  //vis_compare_transforms.start();
  if (true) {
    transform_changed = _parent_net_transform != parent_net_transform;

  } else if (_parent_net_transform != nullptr) {
    transform_changed = *_parent_net_transform != *parent_net_transform;

  } else {
    // It's fresh.
    transform_changed = true;
  }
  //vis_compare_transforms.stop();

  if (transform_changed || _bounds != bounds) {
    _num_sectors = 0;
    _bounds = bounds;
    _parent_net_transform = parent_net_transform;

    if (bounds->is_infinite()) {
      _num_sectors = -1;

    } else if (bounds->is_exact_type(BoundingBox::get_class_type())) {
      const BoundingBox *bbox = (const BoundingBox *)bounds;

      LPoint3 mins = bbox->get_minq();
      LPoint3 maxs = bbox->get_maxq();

      if (!parent_net_transform->is_identity()) {
        // The net transform is non-identity.  We need to transform the
        // box into world coordinates for the K-D tree query.
        const LMatrix4 &mat = parent_net_transform->get_mat();

        LPoint3 x = bbox->get_point(0) * mat;
        LPoint3 n = x;
        for (int i = 1; i < 8; ++i) {
          LPoint3 p = bbox->get_point(i) * mat;
          n.set(std::min(n[0], p[0]), std::min(n[1], p[1]), std::min(n[2], p[2]));
          x.set(std::max(x[0], p[0]), std::max(x[1], p[1]), std::max(x[2], p[2]));
        }
        maxs = x;
        mins = n;
      }

      scene_vis->get_box_sectors(mins, maxs, _sectors, _num_sectors);

    } else if (bounds->is_exact_type(BoundingSphere::get_class_type())) {
      const BoundingSphere *bsphere = (const BoundingSphere *)bounds;

      PN_stdfloat radius = bsphere->get_radius();
      LPoint3 center = bsphere->get_center();

      if (!parent_net_transform->is_identity()) {
        // The net transform is non-identity.  We need to transform the
        // sphere into world coordinates for the K-D tree query.
        const LMatrix4 &mat = parent_net_transform->get_mat();

        // First, determine the longest axis of the matrix, in case it contains a
        // non-uniform scale.

        LVecBase3 x, y, z;
        mat.get_row3(x, 0);
        mat.get_row3(y, 1);
        mat.get_row3(z, 2);

        PN_stdfloat xd = dot(x, x);
        PN_stdfloat yd = dot(y, y);
        PN_stdfloat zd = dot(z, z);

        PN_stdfloat scale = std::max(xd, yd);
        scale = std::max(scale, zd);
        scale = csqrt(scale);

        // Transform the radius
        radius *= scale;

        // Transform the center
        center = center * mat;
      }

      scene_vis->get_sphere_sectors(center, radius, _sectors, _num_sectors);

    } else {
      // If for some reason the node has a bounding volume that isn't a box
      // or sphere, forget it and just say it's in the PVS.  I want to avoid
      // the overhead of the BoundingVolume interface for this check and it
      // would be a pain to implement a K-D tree query for each bounding volume
      // type.
      _num_sectors = -1;
    }
  }

  if (_num_sectors < 0) {
    // Infinite bounds or something.  Always in PVS.
    return true;
  }

  // Otherwise check that at least one sector of the node is in the PVS.
  for (int i = 0; i < _num_sectors; ++i) {
    if (trav->_pvs->get_bit(_sectors[i])) {
      // We're in the PVS!
      return true;
    }
  }

  return false;
}

/**
 *
 */
int PVSCullEffect::
compare_to_impl(const RenderEffect *other) const {
  // We only compare them by pointer.  There's nothing to uniquify since each
  // PVSCullEffect stores its own cache.
  if (this != other) {
    return this < other ? -1 : 1;
  }

  return 0;
}
