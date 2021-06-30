/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physBox.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSBOX_H
#define PHYSBOX_H

#include "pandabase.h"
#include "numeric_types.h"
#include "luse.h"
#include "physGeometry.h"

#include "physx_includes.h"
#include "physx_utils.h"

/**
 * A box physics shape.
 */
class EXPCL_PANDA_PPHYSICS PhysBox final : public PhysGeometry {
PUBLISHED:
  PhysBox(const LVector3 &half_extents);
  PhysBox(PN_stdfloat hx, PN_stdfloat hy, PN_stdfloat hz);
  ~PhysBox() = default;

  INLINE void set_half_extents(const LVector3 &half_extents);
  INLINE void set_half_extents(PN_stdfloat hx, PN_stdfloat hy, PN_stdfloat hz);
  INLINE LVector3 get_half_extents() const;

  INLINE bool is_valid() const;

public:
  virtual physx::PxGeometry *get_geometry() override;

private:
  physx::PxBoxGeometry _geom;
};

#include "physBox.I"

#endif // PHYSBOX_H
