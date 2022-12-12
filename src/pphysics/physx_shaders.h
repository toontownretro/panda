/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physx_shaders.h
 * @author brian
 * @date 2021-04-18
 */

#ifndef PHYSX_SHADERS_H
#define PHYSX_SHADERS_H

#include "pandabase.h"
#include "physx_includes.h"

/**
 * Implementation for collision filtering during the PhysX simulation.
 *
 * Actors are assigned to one or more collision groups through a bitmask.
 * Each bit in the mask represents a collision group.  Collisions between
 * groups can be enabled or disabled.
 */
class EXPCL_PANDA_PPHYSICS PandaSimulationFilterShader {
public:
  static physx::PxFilterFlags filter(
    physx::PxFilterObjectAttributes attributes0,
    physx::PxFilterData filter_data0,
    physx::PxFilterObjectAttributes attributes1,
    physx::PxFilterData filter_data1,
    physx::PxPairFlags &pair_flags,
    const void *constant_block,
    physx::PxU32 constant_block_size);
};

/**
 * This callback allows for disabling collisions between specific pairs of
 * actors.
 */
class EXPCL_PANDA_PPHYSICS PandaSimulationFilterCallback : public physx::PxSimulationFilterCallback {
public:
  virtual physx::PxFilterFlags	pairFound(	physx::PxU32 pairID,
      physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, const physx::PxActor* a0, const physx::PxShape* s0,
      physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1,
      physx::PxPairFlags& pairFlags) override;

  virtual		void			pairLost(	physx::PxU32 pairID,
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		bool objectRemoved) override;

  virtual		bool			statusChange(physx::PxU32& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags) override;

  static PandaSimulationFilterCallback *ptr();

private:
  static PandaSimulationFilterCallback *_ptr;

};

#include "physx_shaders.I"

#endif // PHYSX_SHADERS_H
