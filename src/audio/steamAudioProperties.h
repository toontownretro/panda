/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioProperties.h
 * @author brian
 * @date 2021-12-15
 */

#ifndef STEAMAUDIOPROPERTIES_H
#define STEAMAUDIOPROPERTIES_H

#include "pandabase.h"
#include "numeric_types.h"

/**
 * This class contains properties for configuring the Steam Audio
 * simulation and spatialization of a particular sound.
 */
class EXPCL_PANDA_AUDIO SteamAudioProperties {
PUBLISHED:
  SteamAudioProperties();

  bool _enable_distance_atten;
  bool _enable_air_absorption;
  bool _enable_directivity;
  bool _enable_occlusion;
  bool _enable_transmission;
  bool _enable_reflections;
  bool _enable_pathing;

  PN_stdfloat _directivity_dipole_weight;
  PN_stdfloat _directivity_dipole_power;

  bool _binaural_reflections;
  bool _binaural_pathing;

  bool _bilinear_hrtf;

  bool _volumetric_occlusion;
  PN_stdfloat _volumetric_occlusion_radius;
};

#endif
