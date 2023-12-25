/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleManager2.h
 * @author brian
 * @date 2022-07-31
 */

#ifndef PARTICLEMANAGER2_H
#define PARTICLEMANAGER2_H

#include "pandabase.h"
#include "pvector.h"
#include "particleSystem2.h"
#include "pointerTo.h"
#include "lightMutex.h"
#include "traceInterface.h"

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleManager2 {
PUBLISHED:
  INLINE static ParticleManager2 *get_global_ptr();

  INLINE void set_tracer(TraceInterface *tracer) { _tracer = tracer; }
  INLINE void set_trace_mask(CollideMask mask) { _trace_mask = mask; }
  INLINE TraceInterface *get_tracer() const { return _tracer; }
  INLINE CollideMask get_trace_mask() const { return _trace_mask; }
  INLINE void clear_tracer() { _tracer = nullptr; }

  INLINE void set_enable_fixed_timestep(bool flag) { _want_fixed_timestep = flag; }
  INLINE bool get_enable_fixed_timestep() const { return _want_fixed_timestep; }

  INLINE void set_fixed_timestep(double step) { _fixed_timestep = step; }
  INLINE double get_fixed_timestep() const { return _fixed_timestep; }

  INLINE void set_max_substeps(int steps) { _max_substeps = steps; }
  INLINE int get_max_substeps() const { return _max_substeps; }

  INLINE double get_local_time() const { return _local_time; }
  INLINE double get_last_frame_time() const { return _last_frame_time; }

  INLINE int get_tick_count() const { return _tick_count; }

  INLINE double get_frame_time() const { return _frame_time; }

  void update(double frame_time);

  void run_sim_step(double dt);

  void stop_and_remove_all_systems();

public:
  INLINE ParticleManager2();

  void add_system(ParticleSystem2 *system);
  void remove_system(ParticleSystem2 *system);

private:
  typedef pvector<PT(ParticleSystem2)> Systems;
  // All active systems.
  Systems _systems;

  LightMutex _removed_systems_lock;

  // Default for all systems, unless an explicit one was provided to
  // a system.
  TraceInterface *_tracer;
  CollideMask _trace_mask;

  bool _want_fixed_timestep;
  double _fixed_timestep;
  double _last_frame_time;
  double _local_time;
  int _max_substeps;
  int _tick_count;
  double _frame_time;

private:
  static ParticleManager2 *_global_ptr;
};

#include "particleManager2.I"

#endif // PARTICLEMANAGER2_H
