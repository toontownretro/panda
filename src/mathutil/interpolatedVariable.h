/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interpolatedVariable.h
 * @author brian
 * @date 2021-05-03
 */

#ifndef INTERPOLATEDVARIABLE_H
#define INTERPOLATEDVARIABLE_H

#include "pandabase.h"
#include "config_mathutil.h"
#include "referenceCount.h"
#include "luse.h"
#include "circBuffer.h"
#include "clockObject.h"
#include "pdeque.h"
#include "lerpFunctions.h"
#include "mathutil_misc.h"
#include "configVariableDouble.h"

extern EXPCL_PANDA_MATHUTIL ConfigVariableDouble iv_extrapolate_amount;

static constexpr double extra_interpolation_history_stored = 0.05;

template <class Type>
class SamplePointBase {
public:
  INLINE SamplePointBase() { timestamp = 0.0; }
  Type value;
  double timestamp;

  INLINE SamplePointBase(const SamplePointBase<Type> &other) {
    value = other.value;
    timestamp = other.timestamp;
  }

  INLINE void operator = (const SamplePointBase<Type> &other) {
    value = other.value;
    timestamp = other.timestamp;
  }
};

class EXPCL_PANDA_MATHUTIL InterpolationContext {
PUBLISHED:
  INLINE InterpolationContext();
  INLINE ~InterpolationContext();

  INLINE static void enable_extrapolation(bool flag);
  INLINE static bool has_context();
  INLINE static bool is_extrapolation_allowed();
  INLINE static void set_last_timestamp(double time);
  INLINE static double get_last_timestamp();

private:
  InterpolationContext *_next;
  bool _old_allow_extrapolation;
  double _old_last_timestamp;

  static InterpolationContext *_head;
  static bool _allow_extrapolation;
  static double _last_timestamp;
};

class InterpolationInfo {
PUBLISHED:
  INLINE InterpolationInfo() {
    hermite = false;
    oldest = -1;
    older = -1;
    newer = -1;
    frac = 0.0;
  }

  bool hermite;
  int oldest; // Only set if using hermite interpolation.
  int older;
  int newer;
  double frac;
};

/**
 *
 */
class InterpolatedVariableBase : public ReferenceCount {
public:
  virtual bool record_void_value(const void *value, double timestamp,
                                 bool record_last_networked = true) = 0;
  virtual void record_last_networked_void_value(const void *value, double timestamp) = 0;
  virtual void copy_interpolated_value(void *dest) const = 0;
  virtual void copy_last_networked_value(void *dest) const = 0;
  virtual int interpolate_into(double now, void *dest) = 0;

  virtual bool record_value(double timestamp, bool record_last_networked = true) = 0;
  virtual void record_last_networked_value(double timestamp) = 0;
  virtual void copy_interpolated_value() const = 0;
  virtual void copy_last_networked_value() const = 0;
  virtual int interpolate_into(double now) = 0;
  virtual void reset() = 0;
};

/**
 * A variable whose changes in values are buffered and interpolated.  The type
 * used needs to have vector-like math operators (/, *, etc).
 *
 * The user should record changes in values to the variable and associate it
 * with a timestamp.  Later, an interpolated value can be calculated based
 * on the current rendering time, which can be retrieved by the user.
 *
 * Implementation derived from InterpolatedVar code in Valve's Source Engine.
 */
template <class Type>
class InterpolatedVariable : public InterpolatedVariableBase {
public:
  typedef Type (*GetValueFn)(void *data);
  typedef void (*SetValueFn)(Type val, void *data);

  INLINE void set_getter_func(GetValueFn func, void *data);
  INLINE void set_setter_func(SetValueFn func, void *data);
  INLINE void set_data_ptr(Type *ptr);

PUBLISHED:
  INLINE InterpolatedVariable();

  // Versions for if we don't have a stored pointer/getter/setter for the real variable memory.
  virtual bool record_void_value(const void *value, double timestamp, bool record_last_networked) override;
  virtual void record_last_networked_void_value(const void *value, double timestamp) override;
  virtual void copy_interpolated_value(void *dest) const override;
  virtual void copy_last_networked_value(void *dest) const override;
  virtual int interpolate_into(double now, void *dest) override;

  virtual bool record_value(double timestamp, bool record_last_networked = true) override;
  virtual void record_last_networked_value(double timestamp) override;
  virtual void copy_interpolated_value() const override;
  virtual void copy_last_networked_value() const override;
  virtual int interpolate_into(double now) override;
  virtual void reset() override;

  INLINE bool record_value(const Type &value, double timestamp,
                           bool record_last_networked = true);

  INLINE void record_last_networked_value(const Type &value, double timestamp);

  INLINE void set_interpolation_amount(PN_stdfloat amount);
  INLINE PN_stdfloat get_interpolation_amount() const;

  INLINE void set_looping(bool loop);
  INLINE bool get_looping() const;

  INLINE void set_angles(bool flag);
  INLINE bool get_angles() const;

  INLINE void clear_history();
  INLINE void reset(const Type &value);

  INLINE int interpolate(double now);

  INLINE const Type &get_interpolated_value() const;
  INLINE double get_interpolated_time() const;

  INLINE const Type &get_last_networked_value() const;
  INLINE double get_last_networked_time() const;

  INLINE void get_derivative(Type *out, double now);
  INLINE void get_derivative_smooth_velocity(Type *out, double now);

  INLINE double get_interval() const;

  INLINE Type *get_sample_value(int index);
  INLINE double get_sample_timestamp(int index) const;
  INLINE void set_sample_value(int index, const Type &value);
  INLINE int get_num_samples() const;

  INLINE bool get_interpolation_info(double now, int &newer, int &older, int &oldest) const;
  INLINE bool get_interpolation_info(InterpolationInfo &info, double now) const;

  INLINE void push_front(const Type &value, double timestamp, bool flush_newer);

private:
  INLINE void remove_samples_before(double timestamp);

  INLINE bool get_interpolation_info(InterpolationInfo &info, double now,
                                     int &no_more_changes) const;

  INLINE bool get_value(Type *val) const;
  INLINE void set_value(const Type &val) const;

private:
  typedef SamplePointBase<Type> SamplePoint;
  typedef pdeque<SamplePoint> SamplePoints;
  SamplePoints _sample_points;

  INLINE void time_fixup_hermite(SamplePoint &fixup, SamplePoint *&prev, SamplePoint *&start, SamplePoint *&end);
  INLINE void time_fixup2_hermite(SamplePoint &fixup, SamplePoint *&prev, SamplePoint *&start, double dt);
  INLINE void interpolate_hermite(Type *out, double frac, SamplePoint *prev, SamplePoint *start, SamplePoint *end, bool looping = false);
  INLINE void derivative_hermite(Type *out, double frac, SamplePoint *original_prev, SamplePoint *start, SamplePoint *end);
  INLINE void derivative_hermite_smooth_velocity(Type *out, double frac, SamplePoint *b, SamplePoint *c, SamplePoint *d);

  INLINE void interpolate(Type *out, double frac, SamplePoint *start, SamplePoint *end);
  INLINE void derivative_linear(Type *out, SamplePoint *start, SamplePoint *end);

  INLINE void extrapolate(Type *out, SamplePoint *old, SamplePoint *new_point, double dest_time, double max_extrapolate);

  // The most recently computed interpolated value.
  double _interpolated_value_time;
  Type _interpolated_value;

  // The most recently recorded sample point.
  Type _last_networked_value;
  double _last_networked_time;

  PN_stdfloat _interpolation_amount;
  bool _looping;
  bool _angles;

  // Stuff to retrieve/store destination memory.
  GetValueFn _getter;
  void *_getter_data;
  SetValueFn _setter;
  void *_setter_data;
  Type *_data_ptr;
};

BEGIN_PUBLISH

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<float>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LVecBase2f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LVecBase3f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LVecBase4f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LQuaternionf>);

typedef InterpolatedVariable<float> InterpolatedFloat;
typedef InterpolatedVariable<LVecBase2f> InterpolatedVec2f;
typedef InterpolatedVariable<LVecBase3f> InterpolatedVec3f;
typedef InterpolatedVariable<LVecBase4f> InterpolatedVec4f;
typedef InterpolatedVariable<LQuaternionf> InterpolatedQuatf;

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<double>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LVecBase2d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LVecBase3d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LVecBase4d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<LQuaterniond>);

typedef InterpolatedVariable<double> InterpolatedDouble;
typedef InterpolatedVariable<LVecBase2d> InterpolatedVec2d;
typedef InterpolatedVariable<LVecBase3d> InterpolatedVec3d;
typedef InterpolatedVariable<LVecBase4d> InterpolatedVec4d;
typedef InterpolatedVariable<LQuaterniond> InterpolatedQuatd;

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, InterpolatedVariable<int>);
typedef InterpolatedVariable<int> InterpolatedInt;

#ifdef STDFLOAT_DOUBLE
typedef InterpolatedDouble InterpolatedSTDFloat;
typedef InterpolatedVec2d InterpolatedVec2;
typedef InterpolatedVec3d InterpolatedVec3;
typedef InterpolatedVec3d InterpolatedVec4;
typedef InterpolatedQuatd InterpolatedQuat;
#else
typedef InterpolatedFloat InterpolatedSTDFloat;
typedef InterpolatedVec2f InterpolatedVec2;
typedef InterpolatedVec3f InterpolatedVec3;
typedef InterpolatedVec3f InterpolatedVec4;
typedef InterpolatedQuatf InterpolatedQuat;
#endif

END_PUBLISH

#include "interpolatedVariable.I"

#endif // INTERPOLATEDVARIABLE_H
