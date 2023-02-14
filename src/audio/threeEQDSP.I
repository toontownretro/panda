/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threeEQDSP.I
 * @author brian
 * @date 2023-02-13
 */

/**
 *
 */
INLINE ThreeEQDSP::
ThreeEQDSP() :
  DSP(DT_3eq),
  _low_gain(0.0f),
  _mid_gain(0.0f),
  _high_gain(0.0f),
  _low_mid_crossover_freq(400.0f),
  _mid_high_crossover_freq(4000.0f),
  _crossover_slope(1)
{
}

/**
 *
 */
INLINE void ThreeEQDSP::
set_gain(float low, float mid, float high) {
  _low_gain = low;
  _mid_gain = mid;
  _high_gain = high;
  on_values_changed();
}

/**
 *
 */
INLINE void ThreeEQDSP::
set_crossover_frequencies(float low_mid, float mid_high) {
  _low_mid_crossover_freq = low_mid;
  _mid_high_crossover_freq = mid_high;
  on_values_changed();
}

/**
 *
 */
INLINE void ThreeEQDSP::
set_crossover_slope(int slope) {
  _crossover_slope = slope;
  on_values_changed();
}

/**
 *
 */
INLINE float ThreeEQDSP::
get_low_gain() const {
  return _low_gain;
}

/**
 *
 */
INLINE float ThreeEQDSP::
get_mid_gain() const {
  return _mid_gain;
}

/**
 *
 */
INLINE float ThreeEQDSP::
get_high_gain() const {
  return _high_gain;
}

/**
 *
 */
INLINE float ThreeEQDSP::
get_low_mid_crossover_frequency() const {
  return _low_mid_crossover_freq;
}

/**
 *
 */
INLINE float ThreeEQDSP::
get_mid_high_crossover_frequency() const {
  return _mid_high_crossover_freq;
}

/**
 *
 */
INLINE int ThreeEQDSP::
get_crossover_slope() const {
  return _crossover_slope;
}

