/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSystem.h
 * @author brian
 * @date 2021-04-13
 */

#ifndef PHYSSYSTEM_H
#define PHYSSYSTEM_H

#include "pandabase.h"

#include "physXAllocator.h"
#include "physXErrorCallback.h"

// PhysX includes.
#include "PxFoundation.h"
#include "PxPhysics.h"
#include "pvd/PxPvd.h"
#include "pvd/PxPvdTransport.h"
#include "cooking/PxCooking.h"
#include "PxPhysicsVersion.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysSystem {
private:
  PhysSystem();

PUBLISHED:
  static PhysSystem *ptr();

  bool initialize();
  void shutdown();

  INLINE bool is_initialized() const;

  INLINE int get_api_version_major() const;
  INLINE int get_api_version_minor() const;
  INLINE int get_api_version_bugfix() const;
  INLINE int get_api_version() const;
  INLINE std::string get_api() const;
  INLINE std::string get_api_version_string() const;

public:
  INLINE physx::PxFoundation *get_foundation() const;
  INLINE physx::PxPhysics *get_physics() const;
  INLINE physx::PxCooking *get_cooking() const;
  INLINE physx::PxPvd *get_pvd() const;
  INLINE const physx::PxTolerancesScale &get_scale() const;

private:
  static PhysSystem *_ptr;

  bool _initialized;

  physx::PxTolerancesScale _scale;
  physx::PxFoundation *_foundation;
  physx::PxPhysics *_physics;
  physx::PxCooking *_cooking;
  physx::PxPvd *_pvd;

  PhysXAllocator _allocator;
  PhysXErrorCallback _error_callback;
};

#include "physSystem.I"

#endif // PHYSSYSTEM_H
