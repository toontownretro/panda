/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file phonemes.h
 * @author brian
 * @date 2022-02-01
 */

#ifndef PHONEMES_H
#define PHONEMES_H

#include "pandabase.h"
#include "referenceCount.h"
#include "vector_stdfloat.h"
#include "vector_string.h"
#include "vector_int.h"

class Character;

/**
 * Maps vocal phonemes to slider value combinations of a particular character.
 * Allows use of sliders to make a character's mouth move with speech.
 */
class EXPCL_PANDA_ANIM Phonemes : public ReferenceCount {
PUBLISHED:
  typedef unsigned short PhonemeCode;

  enum PhonemeClass {
    PC_normal,
    PC_strong,
    PC_weak,

    PC_COUNT,
  };

  class SliderInfluence {
  public:
    SliderInfluence() = default;
    int _slider;
    PN_stdfloat _setting;
    PN_stdfloat _weight;
  };
  typedef pvector<SliderInfluence> SliderInfluences;
  typedef pmap<PhonemeCode, SliderInfluences> PhonemeMapping;

  Phonemes() = default;

  void add_phoneme_slider(PhonemeClass cls, PhonemeCode code, int slider, PN_stdfloat setting, PN_stdfloat weight);
  int get_num_phoneme_sliders(PhonemeClass cls, PhonemeCode code) const;
  int get_phoneme_slider(PhonemeClass cls, PhonemeCode code, int n, PN_stdfloat &setting, PN_stdfloat &weight) const;

  bool read(PhonemeClass cls, Filename filename, Character *character);

public:
  PhonemeMapping _mappings[PC_COUNT];
  vector_int _class_keys[PC_COUNT];
  vector_int _all_keys;

};

#include "phonemes.I"

#endif // PHONEMES_H
