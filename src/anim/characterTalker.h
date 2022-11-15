/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterTalker.h
 * @author brian
 * @date 2022-02-10
 */

#ifndef CHARACTERTALKER_H
#define CHARACTERTALKER_H

#include "pandabase.h"
#include "animChannel.h"
#include "spokenSentence.h"
#include "audioSound.h"
#include "pointerTo.h"
#include "phonemes.h"

/**
 * Moves sliders around to make a character's mouth move with speech.
 */
class EXPCL_PANDA_ANIM CharacterTalker : public AnimChannel {
PUBLISHED:
  CharacterTalker(Phonemes *phonemes);
  CharacterTalker(const CharacterTalker &copy);

  void speak(AudioSound *sound, SpokenSentence *sentence);
  void stop();

  // AnimChannel interface.
  virtual PT(AnimChannel) make_copy() const override;
  virtual PN_stdfloat get_length(Character *character) const override;
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data) override;
  virtual LVector3 get_root_motion_vector(Character *character) const override;

private:
  PN_stdfloat get_sentence_intensity(PN_stdfloat t, PN_stdfloat length);
  SpokenSentence::Emphasis *get_emphasis_sample(int n, PN_stdfloat length);
  void add_visemes_for_sentence(PN_stdfloat emphasis_intensity, PN_stdfloat t, PN_stdfloat dt, bool just_started);
  void add_viseme(PN_stdfloat emphasis_intensity, int phoneme, PN_stdfloat scale);
  bool setup_emphasis_blend(int phoneme);
  void compute_blended_setting(PN_stdfloat emphasis_intensity);

private:
  PN_stdfloat _start_time;

  PT(AudioSound) _audio;
  PT(SpokenSentence) _sentence;

  PT(Phonemes) _phonemes;

  class EmphasizedPhoneme {
  public:
    const Phonemes::SliderInfluences *_influences;
    PN_stdfloat _amount;
    bool _valid;

    vector_stdfloat _key_weights;
  };
  EmphasizedPhoneme _classes[Phonemes::PC_COUNT];
};

#include "characterTalker.I"

#endif // CHARACTERTALKER_H
