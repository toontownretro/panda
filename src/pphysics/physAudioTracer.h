/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physAudioTracer.h
 * @author brian
 * @date 2023-01-27
 */

#ifndef PHYSAUDIOTRACER_H
#define PHYSAUDIOTRACER_H

#include "pandabase.h"
#include "audioTracer.h"
#include "collideMask.h"
#include "physScene.h"

/**
 * Provides a mechanism for the audio system to trace rays into the
 * physics scene.
 */
class EXPCL_PANDA_PPHYSICS PhysAudioTracer : public AudioTracer {
PUBLISHED:
  PhysAudioTracer(PhysScene *scene, CollideMask ray_mask);

  virtual bool trace_ray(const LPoint3 &origin, const LVector3 &direction, PN_stdfloat distance) override;

private:
  CollideMask _ray_mask;
  PT(PhysScene) _scene;
};

#include "physAudioTracer.I"

#endif // PHYSAUDIOTRACER_H
