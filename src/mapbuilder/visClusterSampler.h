/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visClusterSampler.h
 * @author brian
 * @date 2021-12-19
 */

#ifndef VISCLUSTERSAMPLER_H
#define VISCLUSTERSAMPLER_H

#include "pandabase.h"
#include "luse.h"
#include "randomizer.h"

class MapData;
class AreaClusterPVS;

/**
 * This class is reponsible for generating sample positions within a vis
 * cluster.  It is currently used to generate ambient probe sample positions
 * and audio reflection probe positions.  The density of sample positions is
 * configurable.
 */
class VisClusterSampler {
public:
  VisClusterSampler(MapData *data);

  void generate_samples(int cluster_id, const LVecBase3 &density, int max_samples,
                        int min_samples, pset<LPoint3> &sample_positions);
  LPoint3 generate_sample_position(const AreaClusterPVS *cluster, const LPoint3 &mins, const LPoint3 &maxs);

private:
  MapData *_data;
  Randomizer _random;
};

#include "visClusterSampler.I"

#endif // VISCLUSTERSAMPLER_H
