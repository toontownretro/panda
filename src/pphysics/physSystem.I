/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSystem.I
 * @author brian
 * @date 2021-04-13
 */

/**
 * Returns true if the physics system has been sucessfully initialized, false
 * otherwise.
 */
INLINE bool PhysSystem::
is_initialized() const {
  return _initialized;
}

/**
 * Returns the major version of the underlying physics engine.
 */
INLINE int PhysSystem::
get_api_version_major() const {
  return PX_PHYSICS_VERSION_MAJOR;
}

/**
 * Returns the minor version of the underlying physics engine.
 */
INLINE int PhysSystem::
get_api_version_minor() const {
  return PX_PHYSICS_VERSION_MINOR;
}

/**
 * Returns the bugfix version of the underlying physics engine.
 */
INLINE int PhysSystem::
get_api_version_bugfix() const {
  return PX_PHYSICS_VERSION_BUGFIX;
}

/**
 * Returns the full version of the underlying physics engine as an integer.
 */
INLINE int PhysSystem::
get_api_version() const {
  return PX_PHYSICS_VERSION;
}

/**
 * Returns the full version string of the underlying physics engine.
 */
INLINE std::string PhysSystem::
get_api_version_string() const {
  std::ostringstream ss;
  ss << get_api_version_major() << "." << get_api_version_minor()
     << "." << get_api_version_bugfix();
  return ss.str();
}

/**
 * Returns the name of the underlying physics engine.
 */
INLINE std::string PhysSystem::
get_api() const {
  return "NVIDIA PhysX";
}

/**
 *
 */
INLINE physx::PxFoundation *PhysSystem::
get_foundation() const {
  return _foundation;
}

/**
 *
 */
INLINE physx::PxPhysics *PhysSystem::
get_physics() const {
  return _physics;
}

/**
 *
 */
INLINE physx::PxCooking *PhysSystem::
get_cooking() const {
  return _cooking;
}

/**
 *
 */
INLINE physx::PxPvd *PhysSystem::
get_pvd() const {
  return _pvd;
}

/**
 *
 */
INLINE physx::PxCpuDispatcher *PhysSystem::
get_cpu_dispatcher() const {
  return _cpu_dispatcher;
}

/**
 *
 */
INLINE const physx::PxTolerancesScale &PhysSystem::
get_scale() const {
  return _scale;
}
