/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSleepStateCallbackData.h
 * @author brian
 * @date 2021-04-21
 */

#ifndef PHYSSLEEPSTATECALLBACKDATA_H
#define PHYSSLEEPSTATECALLBACKDATA_H

#include "pandabase.h"
#include "refCallbackData.h"

#include "physx_includes.h"

class PhysRigidActorNode;

/**
 * Callback data for when an wakes up or goes to sleep.
 */
class EXPCL_PANDA_PPHYSICS PhysSleepStateCallbackData : public RefCallbackData {
PUBLISHED:
  enum State {
    S_awake,
    S_asleep,
  };

  INLINE State get_state() const;

  INLINE bool is_awake() const;
  INLINE bool is_asleep() const;

  INLINE PhysRigidActorNode *get_node() const;

public:
  INLINE PhysSleepStateCallbackData(physx::PxActor *actor, State state);

private:
  physx::PxActor *_actor;
  State _state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RefCallbackData::init_type();
    register_type(_type_handle, "PhysSleepStateCallbackData",
                  RefCallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physSleepStateCallbackData.I"

#endif // PHYSSLEEPSTATECALLBACKDATA_H
