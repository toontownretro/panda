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

/**
 *
 */
VisClusterSampler::
VisClusterSampler(MapData *data) :
  _data(data)
{
}

/**
 * Generates random sample positions within the indicated vis cluster using
 * the given density.  Generates no more than `max_samples` positions and no
 * fewer than `min_samples` positions.
 */
void VisClusterSampler::
generate_samples(int cluster_id, const LVecBase3 &density, int max_samples,
                 int min_samples, pset<LPoint3> &sample_positions) {
  const AreaClusterPVS *cluster = _data->get_cluster_pvs(cluster_id);
  nassertv(cluster != nullptr);

  LPoint3 mins(1e+9);
  LPoint3 maxs(1e-9);
  // Calculate the union of all boxes in the cluster.  We will attempt
  // to generate sample positions within this volume, but they have to
  // be within one of the cluster boxes.
  for (int i = 0; i < cluster->get_num_boxes(); i++) {
    LPoint3 bmins, bmaxs;
    cluster->get_box_bounds(i, bmins, bmaxs);
    mins[0] = std::min(mins[0], bmins[0]);
    mins[1] = std::min(mins[1], bmins[1]);
    mins[2] = std::min(mins[2], bmins[2]);
    maxs[0] = std::max(maxs[0], bmaxs[0]);
    maxs[1] = std::max(maxs[1], bmaxs[1]);
    maxs[2] = std::max(maxs[2], bmaxs[2]);
  }

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
    sample_positions.insert(generate_sample_position(cluster, mins, maxs));
  }
}

/**
 * Generates a random sample position within the boxes of the given vis
 * cluster.
 */
LPoint3 VisClusterSampler::
generate_sample_position(const AreaClusterPVS *cluster, const LPoint3 &mins, const LPoint3 &maxs) {
  PN_stdfloat dx = maxs[0] - mins[0];
  PN_stdfloat dy = maxs[1] - mins[1];
  PN_stdfloat dz = maxs[2] - mins[2];

  for (int i = 0; i < 1000; i++) {
    LPoint3 sample_pos;
    sample_pos[0] = mins[0] + _random.random_real(dx);
    sample_pos[1] = mins[1] + _random.random_real(dy);
    sample_pos[2] = mins[2] + _random.random_real(dz);
    // Must be in a cluster box.
    for (int j = 0; j < cluster->get_num_boxes(); j++) {
      LPoint3 bmins, bmaxs;
      cluster->get_box_bounds(j, bmins, bmaxs);
      if (sample_pos[0] >= bmins[0] && sample_pos[0] <= bmaxs[0] &&
          sample_pos[1] >= bmins[1] && sample_pos[1] <= bmaxs[1] &&
          sample_pos[2] >= bmins[2] && sample_pos[2] <= bmaxs[2]) {
        // Inside this box.  It's a valid sample position.
        return sample_pos;
      }
    }
  }

  // Didn't generate a valid position.  Just use the center of the cluster
  // volume.
  return (mins + maxs) * 0.5f;
}
