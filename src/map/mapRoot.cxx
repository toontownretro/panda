/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRoot.cxx
 * @author brian
 * @date 2021-07-09
 */

#include "mapRoot.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector world_geometry_coll("Cull:MapRoot");

IMPLEMENT_CLASS(MapRoot);

/**
 *
 */
MapRoot::
MapRoot(MapData *data) :
  PandaNode("map-root"),
  _data(data),
  _pvs_cull(true)
{
  set_cull_callback();
  set_renderable();
}

/**
 *
 */
bool MapRoot::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  PStatTimer timer(world_geometry_coll);

  if (_data == nullptr || !_pvs_cull || _data->get_num_clusters() == 0) {
    return true;
  }

  // Query the cluster that the camera/cull target is currently in.
  int cluster = trav->_view_sector;

  if (cluster < 0) {
    // Invalid cluster.  Don't render anything.
    return false;
  }

  // Build up a bit array of all the mesh groups we will render.
  const AreaClusterPVS *pvs = _data->get_cluster_pvs(cluster);
  if (pvs == nullptr) {
    return false;
  }

  Children children = get_children();

  BitArray visible_mesh_groups;
  for (size_t i = 0; i < pvs->get_num_visible_clusters(); i++) {
    visible_mesh_groups |= _data->get_cluster_pvs(pvs->get_visible_cluster(i))->_mesh_groups;
  }
  int index = visible_mesh_groups.get_lowest_on_bit();
  while (index >= 0) {
    trav->traverse_down(data, children.get_child_connection(index));

    visible_mesh_groups.clear_bit(index);
    index = visible_mesh_groups.get_lowest_on_bit();
  }

  // We've taken care of the traversal for this subgraph.
  return false;
}

/**
 *
 */
void MapRoot::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MapRoot::
write_datagram(BamWriter *manager, Datagram &me) {
  PandaNode::write_datagram(manager, me);

  manager->write_pointer(me, _data);
}

/**
 *
 */
void MapRoot::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);
}

/**
 *
 */
int MapRoot::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);
  _data = DCAST(MapData, p_list[pi++]);
  return pi;
}

/**
 *
 */
TypedWritable *MapRoot::
make_from_bam(const FactoryParams &params) {
  MapRoot *node = new MapRoot;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  node->fillin(scan, manager);
  return node;
}

/**
 *
 */
PandaNode *MapRoot::
make_copy() const {
  return new MapRoot(*this);
}

/**
 *
 */
MapRoot::
MapRoot() :
  PandaNode("map-root"),
  _data(nullptr),
  _pvs_cull(true)
{
  set_cull_callback();
}

/**
 *
 */
MapRoot::
MapRoot(const MapRoot &copy) :
  PandaNode(copy),
  _data(copy._data),
  _pvs_cull(copy._pvs_cull)
{
  set_cull_callback();
}
