/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winding.h
 * @author brian
 * @date 2021-07-07
 */

#ifndef WINDING_H
#define WINDING_H

#include "pandabase.h"
#include "pvector.h"
#include "luse.h"
#include "plane.h"
#include "referenceCount.h"
#include "thread.h"
#include "vector_stdfloat.h"
#include "vector_int.h"
#include "mathutil_misc.h"
#include "config_mapbuilder.h"
#include "threadManager.h"

enum PlaneSide {
  PS_cross = -2, // Crosses the plane.
  PS_front = 0, // Completely in front of the plane.
  PS_back, // Completely behind the plane.
  PS_on, // On the plane exactly.
};

#define MAX_WINDING_POINTS 64

/**
 * A collection of coplanar points that form a polygon.
 */
template<int MaxPoints>
class BaseWinding {
PUBLISHED:
  //ALLOC_DELETED_CHAIN(BaseWinding);

  INLINE BaseWinding(const LPlane &plane);
  INLINE BaseWinding(const BaseWinding &copy);
  INLINE BaseWinding(BaseWinding &&other);
  INLINE BaseWinding();

  INLINE void operator = (const BaseWinding &copy);
  INLINE void operator = (BaseWinding &&other);

  INLINE PN_stdfloat get_area() const;
  INLINE LPoint3 get_center() const;
  INLINE LPlane get_plane() const;

  INLINE void get_bounds(LPoint3 &mins, LPoint3 &maxs) const;

  INLINE PN_stdfloat get_area_and_balance_point(LPoint3 &balance_point) const;

  INLINE BaseWinding<MaxPoints> chop(const LPlane &plane) const;

  INLINE bool contains_point(const LPoint3 &point) const;

  INLINE void translate(const LVecBase3 &offset);

  INLINE void reverse();

  INLINE void remove_colinear_points();

  INLINE PlaneSide get_plane_side(const LPlane &plane) const;

  INLINE void add_point(const LPoint3 &point);
  INLINE int get_num_points() const;
  INLINE const LPoint3 &get_point(int n) const;

  INLINE bool is_empty() const;
  INLINE void clear();

  INLINE void clip_epsilon(const LPlane &plane, PN_stdfloat epsilon,
                    BaseWinding &front, BaseWinding &back) const;
  INLINE void clip_epsilon_offset(const LPlane &plane, PN_stdfloat epsilon, BaseWinding &front, BaseWinding &back,
                           const LVecBase3 &offset);

public:
  INLINE const LPoint3 *get_points() const;

private:
  LPoint3 _points[MaxPoints];
  int _num_points;
};

#include "winding.I"

BEGIN_PUBLISH
typedef BaseWinding<MAX_WINDING_POINTS> Winding;
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MAPBUILDER, EXPTP_PANDA_MAPBUILDER, BaseWinding<MAX_WINDING_POINTS>);
END_PUBLISH


#endif // WINDING_H
