/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioTracer.h
 * @author brian
 * @date 2023-01-27
 */

#ifndef AUDIOTRACER_H
#define AUDIOTRACER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "luse.h"

/**
 * This is an abstract class that provides an interface for the audio
 * system to trace rays against a scene.
 */
class EXPCL_PANDA_AUDIO AudioTracer : public ReferenceCount {
PUBLISHED:
  virtual bool trace_ray(const LPoint3 &start, const LVector3 &direction, PN_stdfloat distance)=0;
};

#include "audioTracer.I"

#endif // AUDIOTRACER_H
