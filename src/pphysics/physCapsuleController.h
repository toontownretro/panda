/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physCapsuleController.h
 * @author brian
 * @date 2023-01-04
 */

#ifndef PHYSCAPSULECONTROLLER_H
#define PHYSCAPSULECONTROLLER_H

#include "pandabase.h"
#include "physController.h"

class PhysScene;
class PhysMaterial;

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysCapsuleController : public PhysController {
PUBLISHED:
  PhysCapsuleController(PhysScene *scene, NodePath node,
                        PN_stdfloat radius, PN_stdfloat height,
                        PhysMaterial *material);
  virtual ~PhysCapsuleController() override;

  INLINE void set_size(PN_stdfloat radius, PN_stdfloat height);
  INLINE PN_stdfloat get_radius() const;
  INLINE PN_stdfloat get_height() const;

  virtual void resize(PN_stdfloat height) override final;

  virtual void destroy() override final;

protected:
  virtual physx::PxController *get_controller() const override;

private:
  physx::PxCapsuleController *_controller;
};

#include "physCapsuleController.I"

#endif // PHYSCAPSULECONTROLLER_H
