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
#include "mapCullTraverser.h"

static PStatCollector world_geometry_coll("Cull:MapRoot");

IMPLEMENT_CLASS(MapRoot);

/**
 *
 */
MapRoot::
MapRoot(MapData *data) :
  PandaNode("map-root"),
  _data(data),
  _pvs_cull(true),
  _built_mesh_groups(false)
{
  set_cull_callback();
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

  if (!_built_mesh_groups) {
    _cluster_mesh_groups.resize(_data->get_num_clusters());
    for (int i = 0; i < _data->get_num_clusters(); ++i) {
      const AreaClusterPVS *pvs = _data->get_cluster_pvs(i);
      for (int j = 0; j < pvs->get_num_visible_clusters(); ++j) {
        BitArray mesh_groups = _data->get_cluster_pvs(pvs->get_visible_cluster(j))->_mesh_groups;
        int index = mesh_groups.get_lowest_on_bit();
        while (index >= 0) {
          _cluster_mesh_groups[i].insert(index);
          mesh_groups.clear_bit(index);
          index = mesh_groups.get_lowest_on_bit();
        }
      }
    }
    _built_mesh_groups = true;
  }

  int cluster;
  if (trav->is_exact_type(MapCullTraverser::get_class_type())) {
    // We can use the cluster that was computed by the MapCullTraverser.
    cluster = ((MapCullTraverser *)trav)->_view_cluster;
  } else {
    // Otherwise look it up in our tree.
    // Isn't this line a beauty.
    cluster = _data->get_area_cluster_tree()->get_leaf_value_from_point(trav->get_scene()->get_camera_path().get_net_transform()->get_pos());
  }

  if (cluster < 0) {
    // Invalid cluster.  Don't render anything.
    return false;
  }

  // Very quickly iterate through all the mesh groups in the PVS and traverse
  // them.
  for (auto it = _cluster_mesh_groups[cluster].begin(); it != _cluster_mesh_groups[cluster].end(); ++it) {
    trav->traverse_down(data, data._node_reader.get_child_connection(*it));
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
  _pvs_cull(true),
  _built_mesh_groups(false)
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
  _pvs_cull(copy._pvs_cull),
  _built_mesh_groups(copy._built_mesh_groups),
  _cluster_mesh_groups(copy._cluster_mesh_groups)
{
  set_cull_callback();
}
