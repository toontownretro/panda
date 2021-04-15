/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSystem.cxx
 * @author brian
 * @date 2021-04-13
 */

#include "config_pphysics.h"
#include "physSystem.h"

PhysSystem *PhysSystem::_ptr = nullptr;

/**
 *
 */
PhysSystem::
PhysSystem() {
  _foundation = nullptr;
  _physics = nullptr;
  _cooking = nullptr;
  _pvd = nullptr;
  _cpu_dispatcher = nullptr;
  _initialized = false;
}

/**
 *
 */
PhysSystem *PhysSystem::
ptr() {
  if (_ptr == nullptr) {
    _ptr = new PhysSystem;
  }

  return _ptr;
}

/**
 * Initializes the physics system.
 *
 * Returns true if everything was successfully initialized, false otherwise.
 */
bool PhysSystem::
initialize() {
  if (_initialized) {
    // Already initialized.  No big deal.
    return true;
  }

  _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocator, _error_callback);
  if (_foundation == nullptr) {
    pphysics_cat.error()
      << "Failed to initialize PxFoundation!\n";
    return false;
  }

  if (phys_enable_pvd) {
    _pvd = physx::PxCreatePvd(*_foundation);
    if (_pvd == nullptr) {
      pphysics_cat.warning()
        << "PVD was requested, but it could not be initialized.\n";
    } else {
      physx::PxPvdTransport *transport = physx::PxDefaultPvdSocketTransportCreate
        (phys_pvd_host.get_value().c_str(), phys_pvd_port, 10);
      if (!_pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL)) {
        pphysics_cat.warning()
          << "Unable to connect to PVD host.\n";
      }
    }
  }

  // Panda uses feet as its unit of measurement, so adjust the tolerance scales
  // accordingly.
  _scale.length = phys_tolerance_scale.get_value();
  _scale.speed = _scale.length * 10;

  _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation,
                             _scale, phys_track_allocations.get_value(), _pvd);
  if (_physics == nullptr) {
    pphysics_cat.error()
      << "Failed to initialize PxPhysics!\n";
    return false;
  }

  _cooking = PxCreateCooking(PX_PHYSICS_VERSION, *_foundation,
                             physx::PxCookingParams(_scale));
  if (_cooking == nullptr) {
    pphysics_cat.error()
      << "Failed to initialize PxCooking!\n";
    return false;
  }

  if (!PxInitExtensions(*_physics, _pvd)) {
    pphysics_cat.error()
      << "Failed to initialize PxExtensions!\n";
    return false;
  }

  _cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
  if (_cpu_dispatcher == nullptr) {
    pphysics_cat.error()
      << "Failed to initialize PxCpuDispatcher!\n";
    return false;
  }

  _initialized = true;

  return true;
}

/**
 *
 */
void PhysSystem::
shutdown() {
  if (!_initialized) {
    return;
  }

  if (_pvd != nullptr) {
    if (_pvd->isConnected()) {
      _pvd->disconnect();
    }
    _pvd->release();
    _pvd = nullptr;
  }

  PxCloseExtensions();

  if (_cooking != nullptr) {
    _cooking->release();
    _cooking = nullptr;
  }

  if (_physics != nullptr) {
    _physics->release();
    _physics = nullptr;
  }

  if (_foundation != nullptr) {
    _foundation->release();
    _foundation = nullptr;
  }

  _initialized = false;
}
