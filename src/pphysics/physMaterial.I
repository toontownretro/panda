/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physMaterial.I
 * @author brian
 * @date 2021-04-14
 */

/**
 *
 */
INLINE void PhysMaterial::
set_static_friction(PN_stdfloat friction) {
  nassertv(_material != nullptr);
  _material->setStaticFriction((physx::PxReal)friction);
}

/**
 *
 */
INLINE PN_stdfloat PhysMaterial::
get_static_friction() const {
  nassertr(_material != nullptr, 0.0f);
  return (PN_stdfloat)_material->getStaticFriction();
}

/**
 *
 */
INLINE void PhysMaterial::
set_dynamic_friction(PN_stdfloat friction) {
  nassertv(_material != nullptr);
  _material->setDynamicFriction((physx::PxReal)friction);
}

/**
 *
 */
INLINE PN_stdfloat PhysMaterial::
get_dynamic_friction() const {
  nassertr(_material != nullptr, 0.0f);
  return (PN_stdfloat)_material->getDynamicFriction();
}

/**
 *
 */
INLINE void PhysMaterial::
set_restitution(PN_stdfloat restitution) {
  nassertv(_material != nullptr);
  _material->setRestitution((physx::PxReal)restitution);
}

/**
 *
 */
INLINE PN_stdfloat PhysMaterial::
get_restitution() const {
  nassertr(_material != nullptr, 0.0f);
  return (PN_stdfloat)_material->getRestitution();
}

/**
 *
 */
INLINE physx::PxMaterial *PhysMaterial::
get_material() const {
  return _material;
}
