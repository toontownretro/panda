/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physScene.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSSCENE_H
#define PHYSSCENE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "luse.h"

#include "physx_includes.h"

/**
 * A scene is a collection of bodies and constraints which can interact.
 * The scene simulates the behavior of these objects over time.  Several scenes
 * may exis at the same time, but each body or constraint is specific to a
 * scene -- they may not be shared.
 */
class EXPCL_PANDA_PPHYSICS PhysScene : public ReferenceCount {
PUBLISHED:
  PhysScene();
  ~PhysScene();

  int simulate(double dt);

  INLINE void set_gravity(const LVector3 &gravity);
  INLINE LVector3 get_gravity() const;

  INLINE void shift_origin(const LVector3 &shift);

  INLINE void set_fixed_timestep(double step);
  INLINE double get_fixed_timestep() const;

  INLINE void set_max_substeps(int count);
  INLINE int get_max_substeps() const;

public:
  INLINE physx::PxScene *get_scene() const;

private:
  double _local_time;
  int _max_substeps;
  double _fixed_timestep;

  physx::PxScene *_scene;

  bool _debug_vis_enabled;
};

#include "physScene.I"

#endif // PHYSSCENE_H
