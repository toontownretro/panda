/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspClusterVisibility.I
 * @author brian
 * @date 2021-07-04
 */

/**
 *
 */
INLINE BSPClusterVisibility::
BSPClusterVisibility() :
  _cluster_index(-1),
  _is_all_visible(false),
  _is_all_audible(false)
{
}

/**
 * Returns the index of the cluster that this object contains visibility
 * information for.
 */
INLINE int BSPClusterVisibility::
get_cluster_index() const {
  return _cluster_index;
}

/**
 * Returns true if the cluster can see all other clusters, false
 * otherwise.
 */
INLINE bool BSPClusterVisibility::
is_all_visible() const {
  return _is_all_visible;
}

/**
 * Returns true if the cluster can hear all other clusters, false otherwise.
 */
INLINE bool BSPClusterVisibility::
is_all_audible() const {
  return _is_all_audible;
}

/**
 * Returns the number of clusters visible from this cluster.
 */
INLINE int BSPClusterVisibility::
get_num_visible_clusters() const {
  return (int)_visible_clusters.size();
}

/**
 * Returns the index of the nth visible cluster from this cluster.
 */
INLINE int BSPClusterVisibility::
get_visible_cluster(int n) const {
  nassertr(n >= 0 && n < (int)_visible_clusters.size(), -1);
  return _visible_clusters[n];
}

/**
 * Returns the number of clusters audible from this cluster.
 */
INLINE int BSPClusterVisibility::
get_num_audible_clusters() const {
  return (int)_audible_clusters.size();
}

/**
 * Returns the index of the nth audible cluster from this cluster.
 */
INLINE int BSPClusterVisibility::
get_audible_cluster(int n) const {
  nassertr(n >= 0 && n < (int)_audible_clusters.size(), -1);
  return _audible_clusters[n];
}
