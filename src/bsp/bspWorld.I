/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspWorld.I
 * @author brian
 * @date 2021-07-04
 */

/**
 *
 */
INLINE int BSPWorld::
get_cluster_count() const {
  return (int)_cluster_geom_nodes.size();
}

/**
 *
 */
INLINE void BSPWorld::
set_cluster_geom_node(int cluster, GeomNode *geom_node) {
  nassertv(cluster >= 0 && cluster < (int)_cluster_geom_nodes.size());
  _cluster_geom_nodes[cluster] = geom_node;
}

/**
 *
 */
INLINE GeomNode *BSPWorld::
get_cluster_geom_node(int cluster) const {
  nassertr(cluster >= 0 && cluster < (int)_cluster_geom_nodes.size(), nullptr);
  return _cluster_geom_nodes[cluster];
}
