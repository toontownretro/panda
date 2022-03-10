/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file staticPartitionedObjectNode.h
 * @author brian
 * @date 2022-03-10
 */

#ifndef STATICPARTITIONEDOBJECTNODE_H
#define STATICPARTITIONEDOBJECTNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "pvector.h"

class SpatialPartition;

/**
 * This is a special kind of node optimized for the specific case of
 * static props in TF2.  It contains a list of "objects", where each object
 * is simply of list of Geoms and their associated RenderStates.  Each
 * object is placed into the spatial partition and only rendered if it's
 * in the PVS at Cull time.
 *
 * Since static props are essentially just lists of static Geoms, this node
 * optimizes it by allowing all the static props to be placed in a single
 * node with special code for rendering the props.
 */
class EXPCL_PANDA_MAP StaticPartitionedObjectNode : public PandaNode {
  DECLARE_CLASS(StaticPartitionedObjectNode, PandaNode);
PUBLISHED:
  StaticPartitionedObjectNode(const std::string &name);
  virtual ~StaticPartitionedObjectNode() = default;

  void add_object(GeomNode *node);

  void partition_objects(int num_leaves, const SpatialPartition *tree);

public:
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data) override;

private:
  class GeomEntry {
  public:
    CPT(Geom) _geom;
    CPT(RenderState) _state;
  };

  class Object {
  public:
    pvector<GeomEntry> _geoms;
    CPT(BoundingVolume) _bounds;
    int _last_trav_counter;
  };

  pvector<Object> _objects;

  // List of objects per visgroup/leaf.
  pvector<pvector<Object *>> _leaf_objects;

  int _trav_counter;

  void add_object_for_draw(CullTraverser *trav, CullTraverserData &data, const Object *object);
};

#include "staticPartitionedObjectNode.I"

#endif // STATICPARTITIONEDOBJECTNODE_H
