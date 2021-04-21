/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXSimulationEventCallback.h
 * @author brian
 * @date 2021-04-20
 */

#ifndef PHYSXSIMULATIONEVENTCALLBACK_H
#define PHYSXSIMULATIONEVENTCALLBACK_H

#include "pandabase.h"
#include "physx_includes.h"

class PhysScene;

/**
 * Implementation of a PhysX simulation event callback.
 */
class PhysXSimulationEventCallback : public physx::PxSimulationEventCallback {
public:
  INLINE PhysXSimulationEventCallback(PhysScene *scene);

  virtual void onConstraintBreak(physx::PxConstraintInfo *constraints, physx::PxU32 count) override;
  virtual void onWake(physx::PxActor **actors, physx::PxU32 count) override;
  virtual void onSleep(physx::PxActor **actors, physx::PxU32 count) override;
  virtual void onContact(const physx::PxContactPairHeader &pair_header,
                         const physx::PxContactPair *pairs, physx::PxU32 num_pairs) override;
  virtual void onTrigger(physx::PxTriggerPair *pairs, physx::PxU32 count) override;
  virtual void onAdvance(const physx::PxRigidBody *const *bodyBuffer,
                         const physx::PxTransform *poseBuffer, const physx::PxU32 count) override;

private:
  PhysScene *_scene;
};

#include "physXSimulationEventCallback.I"

#endif // PHYSXSIMULATIONEVENTCALLBACK_H
