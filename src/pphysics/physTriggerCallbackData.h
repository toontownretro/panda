/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriggerCallbackData.h
 * @author brian
 * @date 2021-04-21
 */

#ifndef PHYSTRIGGERCALLBACKDATA_H
#define PHYSTRIGGERCALLBACKDATA_H

#include "pandabase.h"
#include "refCallbackData.h"

#include "physx_includes.h"

class PhysRigidActorNode;
class PhysShape;

/**
 * Trigger event callback data.
 */
class EXPCL_PANDA_PPHYSICS PhysTriggerCallbackData : public RefCallbackData {
PUBLISHED:
  enum Touch {
    T_none,
    // The other actor has entered the trigger volume.
    T_enter,
    // The other actor has exited the trigger volume.
    T_exit,
  };

  INLINE PhysTriggerCallbackData(const physx::PxTriggerPair &pair);

  INLINE Touch get_touch_type() const;

  INLINE PhysRigidActorNode *get_trigger_node() const;
  INLINE PhysShape *get_trigger_shape() const;

  INLINE PhysRigidActorNode *get_other_node() const;
  INLINE PhysShape *get_other_shape() const;

private:
  physx::PxTriggerPair _pair;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RefCallbackData::init_type();
    register_type(_type_handle, "PhysTriggerCallbackData",
                  RefCallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physTriggerCallbackData.I"

#endif // PHYSTRIGGERCALLBACKDATA_H
