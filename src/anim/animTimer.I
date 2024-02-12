/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animTimer.I
 * @author brian
 * @date 2022-04-27
 */

/**
 *
 */
INLINE PN_stdfloat AnimTimer::
adjust_value(PN_stdfloat unclamped, PN_stdfloat start, PN_stdfloat range, PlayMode mode) {
  switch (mode) {
  case PM_pose:
    return start;

  case PM_play:
    return std::min(std::max(unclamped, 0.0f), range) + start;

  case PM_loop:
    nassertr(range >= 0.0f, 0.0f);
    return cmod(unclamped, range) + start;

  case PM_pingpong:
    {
      nassertr(range >= 0.0f, 0.0f);
      PN_stdfloat c = cmod(unclamped, range * 2.0f);
      if (c > range) {
        return (range * 2.0f - c) + start;
      } else {
        return c + start;
      }
    }

  default:
    return unclamped;
  }
}

/**
 *
 */
INLINE PN_stdfloat AnimTimer::
clamp_value(PN_stdfloat unclamped, PN_stdfloat start, PN_stdfloat range, PlayMode mode) {
  PN_stdfloat temp = start + range;
  start = std::min(start, temp);

  switch (mode) {
  case PM_pose:
    return start;

  case PM_play:
    return std::clamp(unclamped, start, start + range);

  case PM_loop:
    nassertr(range >= 0.0f, 0.0f);
    return unclamped;
    //if (unclamped < start) {
     // unclamped += range * ((start - unclamped) / )
    //}

  case PM_pingpong:
    {
      nassertr(range >= 0.0f, 0.0f);
      PN_stdfloat c = cmod(unclamped, range * 2.0f);
      if (c > range) {
        return (range * 2.0f - c) + start;
      } else {
        return c + start;
      }
    }

  default:
    return unclamped;
  }
}

/**
 * Converts a cycle value to an integer frame number.
 *
 * NOTE: This will return `num_frames` if the cycle is 1.0.
 */
INLINE int AnimTimer::
cycle_to_frame(PN_stdfloat cycle, int num_frames) {
  return (int)cfloor(cycle * num_frames);
}

/**
 * Converts a cycle value to a floating-point frame number.
 *
 * NOTE: This will return `num_frames` if the cycle is 1.0.
 */
INLINE PN_stdfloat AnimTimer::
cycle_to_fframe(PN_stdfloat cycle, int num_frames) {
  return cycle * num_frames;
}

/**
 * Converts a floating-point frame number to a normalized 0..1 cycle.
 */
INLINE PN_stdfloat AnimTimer::
frame_to_cycle(PN_stdfloat frame, int num_frames) {
  nassertr(num_frames > 0, 0.0f);
  return frame / (PN_stdfloat)num_frames;
}
