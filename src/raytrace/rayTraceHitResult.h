/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceHitResult.h
 * @author brian
 */

#ifndef RAYTRACEHITRESULT_H
#define RAYTRACEHITRESULT_H

#include "config_raytrace.h"
#include "luse.h"

class EXPCL_PANDA_RAYTRACE RayTraceHitResult {
public:
  LVector3 hit_normal;
  LVector2 hit_uv;
  unsigned int prim_id;
  unsigned int geom_id;
  float hit_fraction;
  bool hit;

PUBLISHED:
  INLINE RayTraceHitResult()
  {
    prim_id = 0;
    geom_id = 0;
    hit_fraction = 0;
    hit = false;
  }

  INLINE bool has_hit() const
  {
    return hit;
  }
  INLINE LVector3 get_hit_normal() const
  {
    return hit_normal;
  }
  INLINE LVector2 get_uv() const
  {
    return hit_uv;
  }
  INLINE unsigned int get_prim_id() const
  {
    return prim_id;
  }
  INLINE unsigned int get_geom_id() const
  {
    return geom_id;
  }
  INLINE float get_hit_fraction() const
  {
    return hit_fraction;
  }
};

#endif // RAYTRACEHITRESULT_H
