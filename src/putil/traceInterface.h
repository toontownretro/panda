/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file traceInterface.h
 * @author brian
 * @date 2023-12-24
 */

#ifndef TRACEINTERFACE_H
#define TRACEINTERFACE_H

#include "pandabase.h"
#include "collideMask.h"
#include "luse.h"

/**
 * Generic interface for doing intersection tests.  Used by other systems that
 * are unaware of the underlying physics/collision/ray-tracing system, and just
 * need to be able to trace against it.
 */
class EXPCL_PANDA_PUTIL TraceInterface {
PUBLISHED:
  class TraceResult {
  PUBLISHED:
    INLINE PN_stdfloat get_starts_solid() const { return _frac <= 0.0f; }
    INLINE PN_stdfloat has_hit() const { return _frac < 1.0f; }
    INLINE PN_stdfloat get_frac() const { return _frac; }
    INLINE LVector3 get_surface_normal() const { return _surface_normal; }
    INLINE LPoint3 get_surface_pos() const { return _hit_pos; }
    INLINE LPoint3 get_trace_start() const { return _trace_start; }
    INLINE LPoint3 get_trace_end() const { return _trace_end; }
    INLINE LVector3 get_trace_dir() const { return _trace_dir; }
    INLINE PN_stdfloat get_trace_length() const { return (_trace_end - _trace_start).length(); }
    INLINE LPoint3 get_trace_hit_pos() const { return _trace_start + (_trace_end - _trace_start) * _frac; }

  public:
    PN_stdfloat _frac;
    LVector3 _surface_normal;
    LPoint3 _hit_pos;
    LPoint3 _trace_start;
    LPoint3 _trace_end;
    LVector3 _trace_dir;
  };

  virtual TraceResult trace_line(const LPoint3 &start, const LPoint3 &end, CollideMask mask = CollideMask::all_on(),
                                 int collision_group = 0)=0;
  virtual TraceResult trace_sphere(const LPoint3 &start, const LPoint3 &end, PN_stdfloat radius,
                                   CollideMask mask = CollideMask::all_on(), int collision_group = 0)=0;
  virtual TraceResult trace_box(const LPoint3 &start, const LPoint3 &end, const LPoint3 &min_point,
                                const LPoint3 &max_point, const LVecBase3 &hpr = LVecBase3(0.0f, 0.0f, 0.0f),
                                CollideMask mask = CollideMask::all_on(), int collision_group = 0)=0;
};

#endif // TRACEINTERFACE_H
