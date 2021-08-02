/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspClusterVisibility.h
 * @author brian
 * @date 2021-07-04
 */

#ifndef BSPCLUSTERVISIBILITY_H
#define BSPCLUSTERVISIBILITY_H

#include "pandabase.h"
#include "pvector.h"
#include "vector_ushort.h"
#include "pnotify.h"

/**
 * Visibility data for a single cluster in the BSP file.
 */
class EXPCL_PANDA_BSP BSPClusterVisibility {
public:
  INLINE BSPClusterVisibility();

PUBLISHED:
  INLINE int get_cluster_index() const;

  INLINE bool is_all_visible() const;
  INLINE bool is_all_audible() const;

  INLINE int get_num_visible_clusters() const;
  INLINE int get_visible_cluster(int n) const;

  INLINE int get_num_audible_clusters() const;
  INLINE int get_audible_cluster(int n) const;

private:
  int _cluster_index;
  bool _is_all_visible;
  bool _is_all_audible;
  vector_ushort _visible_clusters;
  vector_ushort _audible_clusters;

  friend class BSPData;
};

#include "bspClusterVisibility.I"

#endif // BSPCLUSTERVISIBILITY_H
