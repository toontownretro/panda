/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioProperties.cxx
 * @author brian
 * @date 2021-12-15
 */

#include "steamAudioProperties.h"

/**
 *
 */
SteamAudioProperties::
SteamAudioProperties() :
  _enable_distance_atten(true),
  _enable_air_absorption(true),
  _enable_directivity(false),
  _enable_occlusion(false),
  _enable_transmission(false),
  _enable_reflections(false),
  _enable_pathing(false),
  _directivity_dipole_power(1.0f),
  _directivity_dipole_weight(0.5f),
  _binaural_reflections(true),
  _binaural_pathing(true),
  _bilinear_hrtf(true),
  _volumetric_occlusion(false),
  _volumetric_occlusion_radius(1.0f)
{
}
