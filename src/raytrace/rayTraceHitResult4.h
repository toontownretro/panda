/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceHitResult4.h
 * @author lachbr
 */

#ifndef RAYTRACEHITRESULT4_H
#define RAYTRACEHITRESULT4_H

#include "config_raytrace.h"
#include "ssemath.h"

class ALIGN_16BYTE EXPCL_PANDA_RAYTRACE RayTraceHitResult4
{
public:
        FourVectors hit_normal;
        FourVectors hit_uv;
        u32x4 prim_id;
        u32x4 geom_id;
        fltx4 hit_fraction;
        u32x4 hit;
};

#endif // RAYTRACEHITRESULT4_h
