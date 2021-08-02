/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file portal.h
 * @author brian
 * @date 2021-07-13
 */

#ifndef PORTAL_H
#define PORTAL_H

#include "pandabase.h"
#include "referenceCount.h"
#include "luse.h"
#include "plane.h"
#include "vector_uchar.h"
#include "winding.h"
#include "atomicAdjust.h"

#ifndef CPPPARSER
#include <array>
#endif

class Area;

typedef BaseWinding<12> PortalWinding;

/**
 *
 */
class EXPCL_PANDA_MAPBUILDER Portal : public ReferenceCount {
public:
  Portal() = default;
  ~Portal();

  enum Status {
    S_none,
    S_working,
    S_done,
  };

#ifndef CPPPARSER
  std::array<LPoint3, 4> get_quad(const LVecBase3 &voxel_size, const LPoint3 &scene_min) const;
#endif

  void calc_radius();

  Area *_from_area;
  Area *_to_area;
  LPoint3i _min_voxel;
  LPoint3i _max_voxel;
  LPlane _plane;
  LPoint3 _origin;
  PN_stdfloat _radius;

  // Specific to PVS pass.
  unsigned char *_portal_front = nullptr;
  unsigned char *_portal_flood = nullptr;
  unsigned char *_portal_vis = nullptr;
  int _num_might_see = 0;
  AtomicAdjust::Integer _status = S_none;

  int _id;

  PortalWinding _winding;
};
#include "portal.I"

#endif // PORTAL_H
