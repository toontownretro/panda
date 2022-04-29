/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animTimer.h
 * @author brian
 * @date 2022-04-27
 */

#ifndef ANIMTIMER_H
#define ANIMTIMER_H

#include "pandabase.h"
#include "numeric_types.h"
#include "cmath.h"

/**
 * A utility class that provides methods for computing animation cycle values
 * and frame numbers based on time.
 *
 * Can work with frame numbers or normalized cycle values.
 */
class AnimTimer {
PUBLISHED:
  enum PlayMode {
    PM_none,
    PM_pose,
    PM_play,
    PM_loop,
    PM_pingpong,
  };

  INLINE static PN_stdfloat adjust_value(PN_stdfloat val, PN_stdfloat start, PN_stdfloat range, PlayMode mode);
  INLINE static PN_stdfloat clamp_value(PN_stdfloat val, PN_stdfloat start,
                                        PN_stdfloat range, PlayMode mode);

  INLINE static int cycle_to_frame(PN_stdfloat cycle, int num_frames);
  INLINE static PN_stdfloat cycle_to_fframe(PN_stdfloat cycle, int num_frames);

  INLINE static PN_stdfloat frame_to_cycle(PN_stdfloat frame, int num_frames);
};

#include "animTimer.I"

#endif // ANIMTIMER_H
