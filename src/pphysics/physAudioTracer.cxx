/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physAudioTracer.cxx
 * @author brian
 * @date 2023-01-27
 */

#include "physAudioTracer.h"
#include "physRayCastResult.h"

/**
 *
 */
PhysAudioTracer::
PhysAudioTracer(PhysScene *scene, CollideMask ray_mask) :
  _scene(scene),
  _ray_mask(ray_mask)
{
}

/**
 *
 */
bool PhysAudioTracer::
trace_ray(const LPoint3 &origin, const LVector3 &direction, PN_stdfloat distance) {
  // NOTE: origin and distance are in game units.
  PhysRayCastResult result;
  _scene->raycast(result, origin, direction, distance, _ray_mask);
  return result.has_block();
}
