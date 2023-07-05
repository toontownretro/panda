/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visClusterSampler.cxx
 * @author brian
 * @date 2021-12-19
 */

#include "visClusterSampler.h"
#include "mapData.h"
#include "randomizer.h"
#include "bspTree.h"
#include "winding.h"

/**
 *
 */
VisClusterSampler::
VisClusterSampler(MapData *data) :
  _data(data),
  _trace(data)
{
  _trace.build(MapTraceScene::M_all);
}

/**
 * Generates random sample positions within the indicated vis cluster using
 * the given density.  Generates no more than `max_samples` positions and no
 * fewer than `min_samples` positions.
 */
void VisClusterSampler::
generate_samples(int cluster_id, const LVecBase3 &density, int max_samples,
                 int min_samples, pset<LPoint3> &sample_positions) {

  const BSPTree *tree = (const BSPTree *)_data->get_area_cluster_tree();
  const BSPTree::Leaf *leaf = tree->get_leaf(cluster_id);
  nassertv(leaf != nullptr);
  nassertv(!leaf->is_solid());
  nassertv(leaf->get_value() >= 0);

  // Gather the planes of all parent nodes of the leaf.
  pvector<LPlane> boundary_planes;
  pvector<Winding> boundary_windings;
  int node_idx = tree->get_leaf_parent(cluster_id);
  int child = ~((int)cluster_id);
  while (node_idx >= 0) {
    LPlane plane = tree->_nodes[node_idx].plane;
    if (tree->_nodes[node_idx].children[BACK_CHILD] == child) {
      // Back side.
      plane.flip();
    }
    boundary_planes.push_back(plane);
    boundary_windings.push_back(Winding(plane));
    child = node_idx;
    node_idx = tree->_node_parents[node_idx];
  }

  // Intersect all planes to get windings for the leaf.
  for (size_t j = 0; j < boundary_windings.size(); ++j) {
    for (size_t k = 0; k < boundary_planes.size(); ++k) {
      if (k == j) {
        continue;
      }
      // Flip the plane because we want to keep the back-side.
      boundary_windings[j] = boundary_windings[j].chop(boundary_planes[k]);
    }
  }

  LPoint3 mins(1e+9);
  LPoint3 maxs(1e-9);

  // Average all winding vertex positions to get leaf center.
  LPoint3 leaf_center(0.0f);
  int total_points = 0;
  for (size_t j = 0; j < boundary_windings.size(); ++j) {
    for (int k = 0; k < boundary_windings[j].get_num_points(); ++k) {
      mins = mins.fmin(boundary_windings[j].get_point(k));
      maxs = maxs.fmax(boundary_windings[j].get_point(k));
      leaf_center += boundary_windings[j].get_point(k);
      total_points++;
    }
  }
  leaf_center /= total_points;

  // Now generate sample positions.  Determine how many samples we want.
  int x_samples = (maxs[0] - mins[0]) / density[0];
  int y_samples = (maxs[1] - mins[1]) / density[1];
  int z_samples = (maxs[2] - mins[2]) / density[2];

  x_samples = std::max(x_samples, min_samples);
  y_samples = std::max(y_samples, min_samples);
  z_samples = std::max(z_samples, min_samples);

  int volume_count = x_samples * y_samples * z_samples;
  // Don't do any more than the user specified sample count.
  int sample_count = std::clamp(volume_count, min_samples, max_samples);
  for (int i = 0; i < sample_count; i++) {
    sample_positions.insert(generate_sample_position(boundary_planes, leaf_center, mins, maxs));
  }
}

/**
 * Generates a random sample position within the boxes of the given vis
 * cluster.
 */
LPoint3 VisClusterSampler::
generate_sample_position(const pvector<LPlane> &planes, const LPoint3 &center, const LPoint3 &mins, const LPoint3 &maxs) {
  PN_stdfloat dx = maxs[0] - mins[0];
  PN_stdfloat dy = maxs[1] - mins[1];
  PN_stdfloat dz = maxs[2] - mins[2];

  for (int i = 0; i < 1000; i++) {
    LPoint3 sample_pos;
    sample_pos[0] = mins[0] + _random.random_real(dx);
    sample_pos[1] = mins[1] + _random.random_real(dy);
    sample_pos[2] = mins[2] + _random.random_real(dz);

    bool in_all_planes = true;
    // Must be within the leaf.
    for (const LPlane &plane : planes) {
      if (plane.dist_to_plane(sample_pos) <= 0.0f) {
        in_all_planes = false;
        break;
      }
    }

    if (!in_all_planes) {
      continue;
    }

    // Not inside an object.
    if (_trace.hits_backface(sample_pos, center)) {
      continue;
    }

    // This one is good.
    return sample_pos;
  }

  // Didn't generate a valid position.  Just use the center of the cluster
  // volume.
  return center;
}
