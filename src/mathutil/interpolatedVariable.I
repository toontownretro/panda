/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interpolatedVariable.I
 * @author brian
 * @date 2021-05-03
 */

/**
 *
 */
INLINE InterpolationContext::
InterpolationContext() {
  _old_allow_extrapolation = _allow_extrapolation;
  _old_last_timestamp = _last_timestamp;

  // By default, disable extrapolation unless explicitly enabled.
  _allow_extrapolation = false;

  // This is the context stack.
  _next = _head;
  _head = this;
}

/**
 *
 */
INLINE InterpolationContext::
~InterpolationContext() {
  // Restore values from previous stack element.
  _allow_extrapolation = _old_allow_extrapolation;
  _last_timestamp = _old_last_timestamp;

  _head = _next;
}

/**
 *
 */
INLINE void InterpolationContext::
enable_extrapolation(bool flag) {
  _allow_extrapolation = flag;
}

/**
 *
 */
INLINE bool InterpolationContext::
has_context() {
  return _head != nullptr;
}

/**
 *
 */
INLINE bool InterpolationContext::
is_extrapolation_allowed() {
  return _allow_extrapolation;
}

/**
 *
 */
INLINE void InterpolationContext::
set_last_timestamp(double time) {
  _last_timestamp = time;
}

/**
 *
 */
INLINE double InterpolationContext::
get_last_timestamp() {
  return _last_timestamp;
}

/**
 *
 */
template<class Type>
INLINE InterpolatedVariable<Type>::
InterpolatedVariable() {
  _interpolated_value_time = -1.0;
  _last_networked_time = -1.0;
  _last_networked_value = Type();
  _interpolation_amount = 0.0;
  _looping = false;
  _angles = false;
}

/**
 *
 */
template<class Type>
bool InterpolatedVariable<Type>::
record_void_value(const void *value, double timestamp, bool record_last_networked) {
  return record_value(*(const Type *)value, timestamp, record_last_networked);
}

/**
 *
 */
template<class Type>
void InterpolatedVariable<Type>::
record_last_networked_void_value(const void *value, double timestamp) {
  record_last_networked_value(*(const Type *)value, timestamp);
}

/**
 *
 */
template<class Type>
void InterpolatedVariable<Type>::
copy_interpolated_value(void *dest) const {
  *(Type *)dest = _interpolated_value;
}

/**
 *
 */
template<class Type>
void InterpolatedVariable<Type>::
copy_last_networked_value(void *dest) const {
  *(Type *)dest = _last_networked_value;
}

/**
 *
 */
template<class Type>
int InterpolatedVariable<Type>::
interpolate_into(double now, void *dest) {
  int ret = interpolate(now);
  *(Type *)dest = _interpolated_value;
  return ret;
}

/**
 *
 */
template<class Type>
INLINE bool InterpolatedVariable<Type>::
record_value(const Type &value, double timestamp, bool record_last_networked) {

  // This is a big optimization where it can potentially avoid expensive
  // interpolation involving this variable if it didn't get an actual new
  // value in here.
  bool ret = true;
  if (!_sample_points.empty()) {
    if (value == _sample_points[0].value) {
      ret = false;
    }
  }

  push_front(value, timestamp, true);

  if (record_last_networked) {
    _last_networked_value = value;
    _last_networked_time = timestamp;
  }

  ClockObject *clock = ClockObject::get_global_clock();
  double now = clock->get_frame_time();

  // Make sure that there aren't any super old samples still hanging around.
  remove_samples_before(now - _interpolation_amount - extra_interpolation_history_stored);

  return ret;
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
record_last_networked_value(const Type &value, double timestamp) {
  _last_networked_value = value;
  _last_networked_time = timestamp;
}

/**
 * Sets the interpolation amount, the amount of time to delay the computed
 * value of the variable.  A higher value will result in slower response time,
 * but allow more time to accumulate samples to interpolate.  A lower value
 * results in quicker response time, but there will be less of a time window
 * to accumulate samples.  That means samples that come in too slow will not
 * be able to be interpolated.
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
set_interpolation_amount(PN_stdfloat amount) {
  _interpolation_amount = amount;
}

/**
 * Returns the interpolation amount.  See set_interpolation_amount().
 */
template<class Type>
INLINE PN_stdfloat InterpolatedVariable<Type>::
get_interpolation_amount() const {
  return _interpolation_amount;
}

/**
 * Sets whether or not the variable is looping.
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
set_looping(bool loop) {
  _looping = loop;
}

/**
 * Returns whether or not the variable is looping.
 */
template<class Type>
INLINE bool InterpolatedVariable<Type>::
get_looping() const {
  return _looping;
}

/**
 * Sets whether or not the variable represents euler angles.  This makes sure
 * angles are interpolated correctly.
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
set_angles(bool flag) {
  _angles = flag;
}

/**
 * Returns whether or not the variable represents euler angles.  This makes
 * sure angles are interpolated correctly.
 */
template<class Type>
INLINE bool InterpolatedVariable<Type>::
get_angles() const {
  return _angles;
}

/**
 * Removes all previous sample points.
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
clear_history() {
  _sample_points.clear();
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
reset(const Type &value) {
  clear_history();

  ClockObject *clock = ClockObject::get_global_clock();
  double now = clock->get_frame_time();
  push_front(value, now, false);
  push_front(value, now, false);
  push_front(value, now, false);
}

/**
 * Computes an interpolated value for this variable based on the given time.
 *
 * The result of this method call is stored and can be retrieved with
 * get_interpolated_value()/get_interpolated_time().
 *
 * Returns 1 is the value will always be the same if time is always increasing.
 */
template<class Type>
INLINE int InterpolatedVariable<Type>::
interpolate(double now) {
  int no_more_changes = 0;

  InterpolationInfo info;
  if (!get_interpolation_info(info, now, no_more_changes)) {
    return no_more_changes;
  }

  if (info.hermite) {
    // Base case, we have 3 valid sample points.
    interpolate_hermite(&_interpolated_value, info.frac, &_sample_points[info.oldest],
                        &_sample_points[info.older], &_sample_points[info.newer]);

  } else if (info.newer == info.older) {
    // This means the server clock got way behind the client clock.
    // Extrapolate the value here based on its previous velocity (out to a
    // certain amount).
    int real_older = info.newer + 1;
    if (InterpolationContext::is_extrapolation_allowed() &&
        (real_older >= 0 && real_older < (int)_sample_points.size()) &&
        _sample_points[real_older].timestamp != 0.0 &&
        _interpolation_amount > 0.000001 &&
        InterpolationContext::get_last_timestamp() <= _last_networked_time) {

      // Use the velocity here (extrapolate up to 1/4 of a second).
      extrapolate(&_interpolated_value, &_sample_points[real_older], &_sample_points[info.newer],
                  now - _interpolation_amount,
                  iv_extrapolate_amount.get_value());
    } else {
      // Interpolate
      interpolate(&_interpolated_value, info.frac, &_sample_points[info.older],
                  &_sample_points[info.newer]);
    }
  } else {
    // Interpolate
    interpolate(&_interpolated_value, info.frac, &_sample_points[info.older],
                &_sample_points[info.newer]);
  }

  // Clear out all entries before the oldest since we should never access them
  // again.
  remove_samples_before(now - _interpolation_amount - extra_interpolation_history_stored);

  _interpolated_value_time = now;

  return no_more_changes;
}

/**
 * Returns the most recently computed interpolated value.
 *
 * The user is responsible for calling interpolate() for this value to be
 * calculated.
 */
template<class Type>
INLINE const Type &InterpolatedVariable<Type>::
get_interpolated_value() const {
  return _interpolated_value;
}

/**
 * Returns the time of the most recently computed interpolated value.
 *
 * The user is responsible for calling interpolate() for the interpolated value
 * to be calculated.
 */
template<class Type>
INLINE double InterpolatedVariable<Type>::
get_interpolated_time() const {
  return _interpolated_value_time;
}

/**
 *
 */
template<class Type>
INLINE const Type &InterpolatedVariable<Type>::
get_last_networked_value() const {
  return _last_networked_value;
}

/**
 *
 */
template<class Type>
INLINE double InterpolatedVariable<Type>::
get_last_networked_time() const {
  return _last_networked_time;
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
get_derivative(Type *out, double now) {
  InterpolationInfo info;
  int no_more_changes;
  if (!get_interpolation_info(info, now, no_more_changes)) {
    return;
  }

  if (info.hermite) {
    derivative_hermite(out, info.frac, &_sample_points[info.oldest],
                       &_sample_points[info.older], &_sample_points[info.newer]);
  } else {
    derivative_linear(out, &_sample_points[info.older], &_sample_points[info.newer]);
  }
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
get_derivative_smooth_velocity(Type *out, double now) {
  InterpolationInfo info;
  int no_more_changes;
  if (!get_interpolation_info(info, now, no_more_changes)) {
    return;
  }

  bool extrapolate = false;
  int real_older = 0;

  if (info.hermite) {
    derivative_hermite_smooth_velocity
      (out, info.frac, &_sample_points[info.oldest],
       &_sample_points[info.older], &_sample_points[info.newer]);
    return;

  } else if (info.newer == info.older && InterpolationContext::is_extrapolation_allowed()) {
    // This means the server clock got way behind the client clock.
    // Extrapolate the value here based on its previous velocity (out to a
    // certain amount).
    real_older = info.newer + 1;
    if (real_older >= 0 && real_older < (int)_sample_points.size() &&
        _sample_points[real_older].timestamp != 0.0) {
      if (_interpolation_amount > 0.000001 &&
          InterpolationContext::get_last_timestamp() <= (now - _interpolation_amount)) {
        extrapolate = true;
      }
    }
  }

  if (extrapolate) {
    // Get the velocity from the last segment.
    derivative_linear(out, &_sample_points[real_older], &_sample_points[info.newer]);

    // Now ramp it to zero after iv_extrapolate_amount.
    double dest_time = now - _interpolation_amount;
    double diff = dest_time - _sample_points[info.newer].timestamp;
    diff = std::clamp(diff, 0.0, iv_extrapolate_amount.get_value() * 2);
    if (diff > iv_extrapolate_amount.get_value()) {
      double scale = 1 - (diff - iv_extrapolate_amount.get_value()) / iv_extrapolate_amount.get_value();
      *out = *out * scale;
    }

  } else {
    derivative_linear(out, &_sample_points[info.older], &_sample_points[info.newer]);
  }
}

/**
 * Returns the time elapsed between the two most recent samples.
 */
template<class Type>
INLINE double InterpolatedVariable<Type>::
get_interval() const {
  if (_sample_points.size() > (size_t)1) {
    return _sample_points[0].timestamp - _sample_points[1].timestamp;
  }

  return 0.0;
}

/**
 * Returns the value of the variable at the indicated sample index.
 */
template<class Type>
INLINE Type *InterpolatedVariable<Type>::
get_sample_value(int index) {
  if (index >= 0 && index < (int)_sample_points.size()) {
    SamplePoint &sample = _sample_points[index];
    return &sample.value;

  } else {
    return nullptr;
  }
}

/**
 * Returns the timestamp of the sample at the indicated index.
 */
template<class Type>
INLINE double InterpolatedVariable<Type>::
get_sample_timestamp(int index) const {
  if (index >= 0 && index < (int)_sample_points.size()) {
    return _sample_points[index].timestamp;

  } else {
    return 0.0;
  }
}

/**
 * Changes the value of the variable at the indicated sample index.
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
set_sample_value(int index, const Type &value) {
  if (index >= 0 && index < (int)_sample_points.size()) {
    _sample_points[index].value = value;
  }
}

/**
 * Fills in the indices of the three sample points to interpolate based on the
 * provided rendering time.
 */
template<class Type>
INLINE bool InterpolatedVariable<Type>::
get_interpolation_info(double now, int &newer, int &older, int &oldest) const {
  InterpolationInfo info;
  int no_more_changes;
  bool result = get_interpolation_info(info, now, no_more_changes);
  newer = info.newer;
  older = info.older;
  oldest = info.oldest;
  return result;
}

/**
 *
 */
template<class Type>
INLINE bool InterpolatedVariable<Type>::
get_interpolation_info(InterpolationInfo &info, double now) const {
  int no_more_changes;
  return get_interpolation_info(info, now, no_more_changes);
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
push_front(const Type &value, double timestamp, bool flush_newer) {
  size_t new_slot;

  if (flush_newer) {
    // Get rid of anything that has a timestamp after this sample.  The server
    // might have corrected our clock and moved us back, so our current
    // timestamp is less than a timestamp we added previously.
    while (!_sample_points.empty()) {
      if ((_sample_points[0].timestamp + 0.0001) > timestamp) {
        _sample_points.pop_front();
      } else {
        break;
      }
    }

    new_slot = 0;
    _sample_points.push_front(SamplePoint());

  } else {
    new_slot = 0;
    _sample_points.push_front(SamplePoint());
    for (size_t i = 1; i < _sample_points.size(); i++) {
      if (_sample_points[i].timestamp <= timestamp) {
        break;
      }
      _sample_points[new_slot] = _sample_points[i];
      new_slot = i;
    }
  }

  SamplePoint &point = _sample_points[new_slot];
  point.value = value;
  point.timestamp = timestamp;
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
remove_samples_before(double timestamp) {
  for (size_t i = 0; i < _sample_points.size(); i++) {
    if (_sample_points[i].timestamp < timestamp) {
      // We need to preserve this sample (ie: the one right before this
      // timestamp) and the sample right before it (for hermite blending), and
      // we can get rid of everything else.
      _sample_points.resize(i + 3);
      break;
    }
  }
}

/**
 *
 */
template<class Type>
INLINE bool InterpolatedVariable<Type>::
get_interpolation_info(InterpolationInfo &info, double now, int &no_more_changes) const {
  double target_time = now - _interpolation_amount;

  no_more_changes = 0;

  info.hermite = false;
  info.frac = 0;
  info.oldest = info.older = info.newer = -1;

  for (size_t i = 0; i < _sample_points.size(); i++) {
    info.older = i;

    double older_change_time = _sample_points[i].timestamp;
    if (older_change_time == 0.0) {
      break;
    }

    if (target_time < older_change_time) {
      info.newer = info.older;
      continue;
    }

    if (info.newer == -1) {
      // Have it linearly interpolate between the newest 2 entries.
      info.newer = info.older;

      // Since the time given is past all of our entries, then as long as time
      // continues to increase, we'll be returning the same value.
      no_more_changes = 1;
      return true;
    }

    double newer_change_time = _sample_points[info.newer].timestamp;
    double dt = newer_change_time - older_change_time;
    if (dt > 0.0001) {
      info.frac = (target_time - older_change_time) / (newer_change_time - older_change_time);
      info.frac = std::min(info.frac, 2.0);

      int oldest_index = i + 1;

      if (oldest_index >= 0 && oldest_index < (int)_sample_points.size()) {
        info.oldest = oldest_index;
        double oldest_change_time = _sample_points[oldest_index].timestamp;
        double dt2 = older_change_time - oldest_change_time;
        if (dt2 > 0.0001) {
          info.hermite = true;
        }
      }

      // If info.newer is the most recent entry we have, and all 2 or 3 other
      // entries are identical, then we're always going to return the same value
      // if now increases.
      if (!_sample_points.empty() && info.newer == 0) {
        if (_sample_points[info.newer].value == _sample_points[info.older].value) {
          if (!info.hermite ||
              _sample_points[info.newer].value == _sample_points[info.oldest].value) {
            no_more_changes = 1;
          }
        }
      }
    }

    return true;
  }

  // Didn't find any, return last entry?
  if (info.newer != -1) {
    info.older = info.newer;
    return true;
  }

  // This is the single-sample case.
  info.newer = info.older;
  if (info.older == -1) {
    no_more_changes = -1;
    return false;
  }
  return true;
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
time_fixup_hermite(SamplePoint &fixup, SamplePoint *&prev, SamplePoint *&start, SamplePoint *&end) {
  time_fixup2_hermite(fixup, prev, start, end->timestamp - start->timestamp);
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
time_fixup2_hermite(SamplePoint &fixup, SamplePoint *&prev, SamplePoint *&start, double dt1) {
  double dt2 = start->timestamp - prev->timestamp;

  // If times are not of the same interval, renormalize the earlier sample to
  // allow for uniform hermite spline interpolation.
  if (std::abs(dt1 - dt2) > 0.0001 && dt2 > 0.0001) {
    // Renormalize.
    double frac = dt1 / dt2;

    // Fixed interval into past.
    fixup.timestamp = start->timestamp - dt1;

    if (_looping) {
      fixup.value = LoopingLerp(1 - frac, prev->value, start->value);

    } else if (_angles) {
      fixup.value = tlerp_angles(1 - frac, prev->value, start->value);

    } else {
      fixup.value = tlerp(1 - frac, prev->value, start->value);
    }

    // Point previous sample at fixed version
    prev = &fixup;
  }
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
interpolate_hermite(Type *out, double frac, SamplePoint *prev, SamplePoint *start, SamplePoint *end, bool looping) {

  SamplePoint fixup;
  time_fixup_hermite(fixup, prev, start, end);

  if (_looping) {
    *out = LoopingLerp_Hermite(frac, prev->value, start->value, end->value);

  } else if (!_angles) {
    *out = Lerp_Hermite(frac, prev->value, start->value, end->value);

  } else {
    // Can't do hermite with angles, get discontinuities, so just linear
    // interp.
    *out = tlerp_angles(frac, start->value, end->value);
  }

  // Clamp the output from interpolation.
  Lerp_Clamp(*out);
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
derivative_hermite(Type *out, double frac, SamplePoint *prev, SamplePoint *start, SamplePoint *end) {
  SamplePoint fixup;
  time_fixup_hermite(fixup, prev, start, end);

  double divisor = 1.0 / (end->timestamp - start->timestamp);
  nassertv(!_looping);
  *out = Derivative_Hermite(frac, prev->value, start->value, end->value);
  *out = *out * divisor;
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
derivative_hermite_smooth_velocity(Type *out, double frac, SamplePoint *b, SamplePoint *c, SamplePoint *d) {
  SamplePoint fixup;
  time_fixup_hermite(fixup, b, c, d);
  Type prev_vel = (c->value - b->value) / (c->timestamp - b->timestamp);
  Type cur_vel = (d->value - c->value) / (d->timestamp - c->timestamp);

  if (_angles) {
    *out = tlerp_angles(frac, prev_vel, cur_vel);

  } else {
    *out = tlerp(frac, prev_vel, cur_vel);
  }

}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
interpolate(Type *out, double frac, SamplePoint *start, SamplePoint *end) {
  if (start == end) {
    // Quick exit.
    *out = end->value;
    Lerp_Clamp(*out);
    return;
  }

  nassertv(frac >= 0.0 && frac <= 1.0);

  if (_looping) {
    *out = LoopingLerp(frac, start->value, end->value);
  } else if (_angles) {
    *out = tlerp_angles(frac, start->value, end->value);
  } else {
    *out = tlerp(frac, start->value, end->value);
  }
  Lerp_Clamp(*out);
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
derivative_linear(Type *out, SamplePoint *start, SamplePoint *end) {
  if (start == end || std::abs(start->timestamp - end->timestamp) < 0.0001) {
    *out = start->value * 0.0;
  } else {
    double divisor = 1.0 / (end->timestamp - start->timestamp);
    *out = (end->value - start->value) * divisor;
  }
}

/**
 *
 */
template<class Type>
INLINE void InterpolatedVariable<Type>::
extrapolate(Type *out, SamplePoint *old, SamplePoint *new_point, double dest_time, double max_extrapolate) {
  if (std::abs(old->timestamp - new_point->timestamp) < 0.001 ||
      dest_time <= new_point->timestamp) {
    *out = new_point->value;
  } else {
    double extrapolation_amount = std::min(dest_time - new_point->timestamp,
                                           max_extrapolate);
    double divisor = 1.0 / (new_point->timestamp - old->timestamp);

    if (_angles) {
      *out = tlerp_angles(1.0 + extrapolation_amount * divisor, old->value, new_point->value);
    } else {
      *out = tlerp(1.0 + extrapolation_amount * divisor, old->value, new_point->value);
    }
  }
}
