/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTrace.cxx
 * @author brian
 * @date 2020-09-21
 */

#include "rayTrace.h"

#include "geomVertexReader.h"

#include "rtcore.h"

bool RayTrace::_initialized = false;
RTCDevice RayTrace::_device = nullptr;

/**
 * Initializes the ray trace device.
 */
void RayTrace::
initialize() {
  if ( _initialized ) {
    return;
  }

  _device = rtcNewDevice( "" );

  _initialized = true;
}

/**
 * Destructs the ray trace device.
 */
void RayTrace::
destruct() {
  _initialized = false;
  if ( _device )
    rtcReleaseDevice( _device );
  _device = nullptr;
}
