/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visTile.h
 * @author brian
 * @date 2021-07-13
 */

#ifndef VISTILE_H
#define VISTILE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "area.h"
#include "pointerTo.h"

/**
 *
 */
class EXPCL_PANDA_MAPBUILDER VisTile : public ReferenceCount {
public:
  VisTile() = default;

  INLINE int get_num_voxels() const;
  INLINE bool contains_voxel(const LPoint3i &voxel) const;

  LPoint3i _min_voxel;
  LPoint3i _max_voxel;
  size_t _head_node;
  pvector<PT(Area)> _areas;
  int _num_solid_voxels;
};

#include "visTile.I"

#endif // VISTILE_H
