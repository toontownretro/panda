/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTrace.h
 * @author lachbr
 * @date 2020-09-21
 */

#ifndef BSP_RAYTRACE_H
#define BSP_RAYTRACE_H

#include "config_raytrace.h"

/**
 *
 */
class EXPCL_PANDA_RAYTRACE RayTrace {
PUBLISHED:
  static void initialize();
  static void destruct();

public:
  INLINE static RTCDevice get_device();

private:
  static bool _initialized;
  static RTCDevice _device;
};

#include "rayTrace.I"

#endif // BSP_RAYTRACE_H
