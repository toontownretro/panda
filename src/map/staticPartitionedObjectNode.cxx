/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file staticPartitionedObjectNode.cxx
 * @author brian
 * @date 2022-03-10
 */

#include "staticPartitionedObjectNode.h"
#include "spatialPartition.h"
#include "ordered_vector.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "mapCullTraverser.h"
#include "omniBoundingVolume.h"

IMPLEMENT_CLASS(StaticPartitionedObjectNode);

/**
 *
 */
StaticPartitionedObjectNode::
StaticPartitionedObjectNode(const std::string &name) :
  PandaNode(name),
  _trav_counter(-1)
{
  set_renderable();
  set_bounds(new OmniBoundingVolume);
}

/**
 *
 */
void StaticPartitionedObjectNode::
add_object(GeomNode *node) {
  Object obj;
  obj._bounds = node->get_bounds();
  obj._last_trav_counter = -1;
  for (int i = 0; i < node->get_num_geoms(); ++i) {
    GeomEntry geom;
    geom._geom = node->get_geom(i);
    geom._state = node->get_geom_state(i);
    obj._geoms.push_back(std::move(geom));
  }
  _objects.push_back(std::move(obj));
}

/**
 * Places each object into spatial partition leaves.
 */
void StaticPartitionedObjectNode::
partition_objects(int num_leaves, const SpatialPartition *tree) {
  _leaf_objects.resize(num_leaves);

  for (size_t i = 0; i < _objects.size(); ++i) {
    ov_set<int> leaves;
    Object *obj = &_objects[i];
    if (obj->_bounds->is_of_type(BoundingSphere::get_class_type())) {
      const BoundingSphere *bsphere = (const BoundingSphere *)obj->_bounds.p();
      tree->get_leaf_values_containing_sphere(bsphere->get_center(), bsphere->get_radius(), leaves);

    } else if (obj->_bounds->is_of_type(BoundingBox::get_class_type())) {
      const BoundingBox *bbox = (const BoundingBox *)obj->_bounds.p();
      tree->get_leaf_values_containing_box(bbox->get_minq(), bbox->get_maxq(), leaves);
    }

    for (int leaf : leaves) {
      _leaf_objects[leaf].push_back(obj);
    }
  }
}

/**
 * Adds the node's contents to the CullResult we are building up during the
 * cull traversal, so that it will be drawn at render time.  For most nodes
 * other than GeomNodes, this is a do-nothing operation.
 */
void StaticPartitionedObjectNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  MapCullTraverser *mtrav = (MapCullTraverser *)trav;

  if (mtrav->_data == nullptr || mtrav->_view_cluster < 0) {
    // No map, invalid view cluster, or culling disabled.

    for (const Object &obj : _objects) {
      add_object_for_draw(trav, data, &obj);
    }

    return;
  }

  ++_trav_counter;

  const BitArray &pvs = mtrav->_pvs;
  int num_visgroups = (int)_leaf_objects.size();

  for (int i = 0; i < num_visgroups; ++i) {
    if (!pvs.get_bit(i)) {
      // Not in PVS.
      continue;
    }

    const pvector<Object *> &objects = _leaf_objects[i];

    for (Object *obj : objects) {
      if (obj->_last_trav_counter != _trav_counter) {
        obj->_last_trav_counter = _trav_counter;
        add_object_for_draw(trav, data, obj);
      }
    }
  }
}

/**
 *
 */
void StaticPartitionedObjectNode::
add_object_for_draw(CullTraverser *trav, CullTraverserData &data, const Object *obj) {
  if (data._view_frustum != nullptr) {
    if (!obj->_bounds->contains(data._view_frustum)) {
      return;
    }
  }

  Thread *current_thread = trav->get_current_thread();

  const TransformState *trans = trav->get_scene()->get_cs_world_transform();

  for (const GeomEntry &geom : obj->_geoms) {
    if (data._state->is_empty()) {
      CullableObject cobj(geom._geom, geom._state, trans, current_thread);
      trav->get_cull_handler()->record_object(cobj, trav);

    } else if (geom._state->is_empty()) {
      CullableObject cobj(geom._geom, data._state, trans, current_thread);
      trav->get_cull_handler()->record_object(cobj, trav);

    } else {
      CPT(RenderState) state = data._state->compose(geom._state);
      CullableObject cobj(geom._geom, std::move(state), trans, current_thread);
      trav->get_cull_handler()->record_object(cobj, trav);
    }

  }
}
