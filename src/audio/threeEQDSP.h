/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threeEQDSP.h
 * @author brian
 * @date 2023-02-13
 */

#ifndef THREEEQDSP_H
#define THREEEQDSP_H

#include "dsp.h"

/**
 * 3-band equalizer.  Tweaks the gain of individual frequency bands.
 * For instance, you can make low frequencies quieter, high frequencies
 * louder, etc.
 */
class EXPCL_PANDA_AUDIO ThreeEQDSP : public DSP {
PUBLISHED:
  INLINE ThreeEQDSP();

  INLINE void set_gain(float low, float mid, float high);
  INLINE void set_crossover_frequencies(float low_mid, float mid_high);
  INLINE void set_crossover_slope(int slope);

  INLINE float get_low_gain() const;
  INLINE float get_mid_gain() const;
  INLINE float get_high_gain() const;

  INLINE float get_low_mid_crossover_frequency() const;
  INLINE float get_mid_high_crossover_frequency() const;

  INLINE int get_crossover_slope() const;

  MAKE_PROPERTY(low_gain, get_low_gain);
  MAKE_PROPERTY(mid_gain, get_mid_gain);
  MAKE_PROPERTY(high_gain, get_high_gain);
  MAKE_PROPERTY(low_mid_crossover_frequency, get_low_mid_crossover_frequency);
  MAKE_PROPERTY(mid_high_crossover_frequency, get_mid_high_crossover_frequency);
  MAKE_PROPERTY(crossover_slope, get_crossover_slope, set_crossover_slope);

private:
  float _low_gain;
  float _mid_gain;
  float _high_gain;
  float _low_mid_crossover_freq;
  float _mid_high_crossover_freq;
  int _crossover_slope;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "ThreeEQDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "threeEQDSP.I"

#endif // THREEEQDSP_H
