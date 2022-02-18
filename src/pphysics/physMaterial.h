/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physMaterial.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSMATERIAL_H
#define PHYSMATERIAL_H

#include "pandabase.h"
#include "numeric_types.h"
#include "referenceCount.h"

#include "physx_includes.h"

/**
 * Defines the material properties of a physics shape or body.
 * Static friction, dynamic friction, and restitution are properties built
 * into the PhysX system, but the other properties, such as density and
 * audio reflectivity, are specific to Panda.
 */
class EXPCL_PANDA_PPHYSICS PhysMaterial final : public ReferenceCount {
PUBLISHED:
  PhysMaterial(PN_stdfloat static_friction, PN_stdfloat dynamic_friction,
               PN_stdfloat restitution);
  virtual ~PhysMaterial();

  INLINE void set_static_friction(PN_stdfloat friction);
  INLINE PN_stdfloat get_static_friction() const;
  MAKE_PROPERTY(static_friction, get_static_friction, set_static_friction);

  INLINE void set_dynamic_friction(PN_stdfloat friction);
  INLINE PN_stdfloat get_dynamic_friction() const;
  MAKE_PROPERTY(dynamic_friction, get_dynamic_friction, set_dynamic_friction);

  INLINE void set_restitution(PN_stdfloat restitution);
  INLINE PN_stdfloat get_restitution() const;
  MAKE_PROPERTY(restitution, get_restitution, set_restitution);

  //INLINE void set_density(PN_stdfloat density);
  //INLINE PN_stdfloat get_density() const;
  //MAKE_PROPERTY(density, get_density, set_density);

  //INLINE void set_thickness(PN_stdfloat thickness);
  //INLINE PN_stdfloat get_thickness() const;
  //MAKE_PROPERTY(thickness, get_thickness, set_thickness);

public:
  PhysMaterial(physx::PxMaterial *material);

  INLINE physx::PxMaterial *get_material() const;

private:
  PN_stdfloat _density;
  PN_stdfloat _thickness;
  PN_stdfloat _audio_reflectivity;
  PN_stdfloat _audio_hardness_factor;
  PN_stdfloat _audio_roughness_factor;
  physx::PxMaterial *_material;
};

#include "physMaterial.I"

#endif // PHYSMATERIAL_H
