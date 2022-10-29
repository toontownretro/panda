/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physBoxController.h
 * @author brian
 * @date 2021-04-28
 */

#ifndef PHYSBOXCONTROLLER_H
#define PHYSBOXCONTROLLER_H

#include "pandabase.h"
#include "physController.h"
#include "luse.h"

class PhysScene;
class PhysMaterial;

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysBoxController final : public PhysController {
PUBLISHED:
  PhysBoxController(PhysScene *scene, NodePath node,
                    const LVector3 &half_extents,
                    PhysMaterial *material);
  virtual ~PhysBoxController() override;

  INLINE void set_half_extents(const LVector3 &half_extents);
  INLINE LVector3 get_half_extents() const;

  virtual void destroy() override final;

protected:
  virtual physx::PxController *get_controller() const override;

private:
  physx::PxBoxController *_controller;
};

#include "physBoxController.I"

#endif // PHYSBOXCONTROLLER_H
