/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSleepStateCallbackData.I
 * @author brian
 * @date 2021-04-21
 */

/**
 *
 */
INLINE PhysSleepStateCallbackData::
PhysSleepStateCallbackData(physx::PxActor *actor, State state) :
  _actor(actor),
  _state(state),
  _node((PhysRigidActorNode *)actor->userData)
{
}

/**
 * Returns the state (awake or asleep).
 */
INLINE PhysSleepStateCallbackData::State PhysSleepStateCallbackData::
get_state() const {
  return _state;
}

/**
 *
 */
INLINE bool PhysSleepStateCallbackData::
is_awake() const {
  return _state == S_awake;
}

/**
 *
 */
INLINE bool PhysSleepStateCallbackData::
is_asleep() const {
  return _state == S_asleep;
}

/**
 * Returns the actor node that woke up or went to sleep.
 */
INLINE PT(PhysRigidActorNode) PhysSleepStateCallbackData::
get_node() const {
  return _node.lock();
}
