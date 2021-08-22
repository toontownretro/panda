/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEvalContext.h
 * @author brian
 * @date 2021-08-04
 */

#ifndef ANIMEVALCONTEXT_H
#define ANIMEVALCONTEXT_H

#include "pandabase.h"
#include "bitArray.h"
#include "luse.h"
#include "animInterface.h"

class Character;
class CharacterJoint;

// NOTE: Avoid heap allocations during the AnimChannel evaluation.

/**
 * Defines the context of an AnimChannel hierarchy evaluation.  This data
 * remains constant throughout the entire evaluation.
 */
class AnimEvalContext final {
public:
  AnimEvalContext(const AnimEvalContext &copy) = delete;
  AnimEvalContext() = default;

  // The bit mask of joints we actually care about animating.  Joints that are
  // joint merged or have a forced value don't need to be evaluated, so their
  // bits would not be set.
  BitArray _joint_mask;

  // The number of joints in the character.
  int _num_joints;

  // Should AnimChannelTables do inter-frame blending?
  bool _frame_blend;

  // The character we are evaluating a pose for.
  Character *_character;

  // The character's array of joints.
  CharacterJoint *_joints;

  // The frame time of the evaluation.
  PN_stdfloat _time;

  int _play_mode;
  PN_stdfloat _start_cycle;
  PN_stdfloat _play_cycles;
  PN_stdfloat _play_rate;
};

static constexpr int max_character_joints = 256;

/**
 * Contains the data for evaluating an AnimChannel at a particular level of
 * the hierarchy.
 */
class AnimEvalData final {
public:
  INLINE AnimEvalData();
  INLINE AnimEvalData(const AnimEvalData &copy);
  INLINE AnimEvalData(AnimEvalData &&other);

  INLINE void copy_joints(const AnimEvalData &other);
  INLINE void steal_joints(AnimEvalData &other);

  // Poses of all joints.
  LPoint3 _position[max_character_joints];
  LQuaternion _rotation[max_character_joints];
  LVecBase3 _scale[max_character_joints];

  PN_stdfloat _weight;

  PN_stdfloat _cycle;
};

#include "animEvalContext.I"

#endif // ANIMEVALCONTEXT_H
