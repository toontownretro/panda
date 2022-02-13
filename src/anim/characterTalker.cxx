/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterTalker.cxx
 * @author brian
 * @date 2022-02-10
 */

#include "characterTalker.h"
#include "configVariableDouble.h"
#include "luse.h"
#include "character.h"
#include "clockObject.h"

static ConfigVariableDouble talker_phoneme_delay("talker-phoneme-delay", 0.0);
static ConfigVariableDouble talker_phoneme_filter("talker-phoneme-filter", 0.08);

#define STRONG_CROSSFADE_START		0.60f
#define WEAK_CROSSFADE_START		0.40f

/**
 *
 */
CharacterTalker::
CharacterTalker(Character *character, Phonemes *phonemes) :
  _character(character),
  _phonemes(phonemes),
  _start_time(0.0f)
{
  _classes[Phonemes::PC_normal]._key_weights.resize(phonemes->_class_keys[Phonemes::PC_normal].size());
  _classes[Phonemes::PC_strong]._key_weights.resize(phonemes->_class_keys[Phonemes::PC_strong].size());
  _classes[Phonemes::PC_weak]._key_weights.resize(phonemes->_class_keys[Phonemes::PC_weak].size());
}

/**
 * Starts the character lip-synching to the indicated audio track and sentence
 * pair.
 */
void CharacterTalker::
speak(AudioSound *sound, SpokenSentence *sentence) {
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _sentence = sentence;
  _audio = sound;
}

/**
 * Stops the current voice line.
 */
void CharacterTalker::
stop() {
  _audio = nullptr;
  _sentence = nullptr;
  _start_time = 0.0f;
}

/**
 *
 */
void CharacterTalker::
update() {
  // Start at silence.
  //auto it = _phonemes->_mappings[Phonemes::PC_normal].find('_');
  //if (it != _phonemes->_mappings[Phonemes::PC_normal].end()) {
  //  const Phonemes::SliderInfluences &infs = (*it).second;
  //  for (const Phonemes::SliderInfluence &inf : infs) {
  //    if (inf._slider != -1) {
  //      _character->set_slider_value(inf._slider, inf._setting);
  //    }
  //  }

  //} else {
    // If there is no silence phoneme defined for the character, just set all
    // the keys to zero.
    for (int slider : _phonemes->_all_keys) {
      if (slider != -1) {
        _character->set_slider_value(slider, 0.0f);
      }
    }
  //}

  if (_audio == nullptr || _sentence == nullptr) {
    return;
  }

  if (_audio->status() != AudioSound::PLAYING) {
    // Audio track not playing, so don't update.
    return;
  }

  PN_stdfloat sentence_length = _sentence->_length;
  PN_stdfloat time_since_start = ClockObject::get_global_clock()->get_frame_time() - _start_time;

  if (time_since_start >= (sentence_length + 2.0f)) {
    return;
  }

  for (int i = 0; i < Phonemes::PC_COUNT; ++i) {
    for (size_t j = 0; j < _classes[i]._key_weights.size(); ++j) {
      _classes[i]._key_weights[j] = 0.0f;
    }
  }

  // Adjust actual time.
  PN_stdfloat t = time_since_start - talker_phoneme_delay;

  PN_stdfloat dt = talker_phoneme_filter;

  bool just_started = false;

  // Get intensity setting for this time (from spline).
  PN_stdfloat emphasis_intensity = get_sentence_intensity(t, sentence_length);

  add_visemes_for_sentence(emphasis_intensity, t, dt, just_started);

  for (int i = 0; i < Phonemes::PC_COUNT; ++i) {
    for (int j = 0; j < _classes[i]._key_weights.size(); ++j) {
      if (_phonemes->_class_keys[i][j] != -1) {
        _character->set_slider_value(_phonemes->_class_keys[i][j],
          _character->get_slider_value(_phonemes->_class_keys[i][j]) + _classes[i]._key_weights[j]);
      }
    }
  }
}

/**
 *
 */
void CharacterTalker::
add_visemes_for_sentence(PN_stdfloat emphasis_intensity, PN_stdfloat t, PN_stdfloat dt, bool just_started) {
  int count = (int)_sentence->_all_phonemes.size();

  for (int i = 0; i < count; ++i) {
    const SpokenSentence::WordPhoneme *phoneme = &_sentence->_all_phonemes[i];

    if (t > phoneme->_start_time && t < phoneme->_end_time) {
      bool cross_fade = true;

      if (cross_fade) {
        if (i < count - 1) {
          const SpokenSentence::WordPhoneme *next = &_sentence->_all_phonemes[i + 1];

          if (next->_start_time == phoneme->_end_time) {
            // No gap, so increase the blend length to the end of the next phoneme.
            dt = std::max(dt, std::min(next->_end_time - t, phoneme->_end_time - phoneme->_start_time));

          } else {
            // Dead space, so increase the blend length to the start of the next phoneme.
            dt = std::max(dt, std::min(next->_start_time - t, phoneme->_end_time - phoneme->_start_time));
          }

        } else {
          // Last phoneme in list, increase the blend length to the length of the current phoneme.
          dt = std::max(dt, phoneme->_end_time - phoneme->_start_time);
        }
      }
    }


    PN_stdfloat t1 = (phoneme->_start_time - t) / dt;
    PN_stdfloat t2 = (phoneme->_end_time - t) / dt;

    if (t1 < 1.0f && t2 > 0.0f) {
      PN_stdfloat scale;

      if (t2 > 1.0f) {
        t2 = 1.0f;
      }
      if (t1 < 0.0f) {
        t1 = 0.0f;
      }

      scale = (t2 - t1);

      add_viseme(emphasis_intensity, phoneme->_phoneme_code, scale);
    }
  }
}

/**
 *
 */
bool CharacterTalker::
setup_emphasis_blend(int phoneme) {
  for (int i = 0; i < Phonemes::PC_COUNT; ++i) {
    EmphasizedPhoneme *info = &_classes[i];

    // Assume it's bogus.
    info->_valid = false;
    info->_amount = 0.0f;

    auto it = _phonemes->_mappings[i].find(phoneme);
    if (it != _phonemes->_mappings[i].end()) {
      info->_influences = &(*it).second;
      info->_valid = true;

    } else {
      return true;
    }
  }

  return false;
}

/**
 *
 */
void CharacterTalker::
compute_blended_setting(PN_stdfloat emphasis_intensity) {
  bool has_weak = _classes[Phonemes::PC_weak]._valid;
  bool has_strong = _classes[Phonemes::PC_strong]._valid;

  if (emphasis_intensity > STRONG_CROSSFADE_START) {
    if (has_strong) {
      // Blend in some of the strong.
      PN_stdfloat dist_remaining = 1.0f - emphasis_intensity;
      PN_stdfloat frac = dist_remaining / (1.0f - STRONG_CROSSFADE_START);

      _classes[Phonemes::PC_normal]._amount = frac * 2.0f * STRONG_CROSSFADE_START;
      _classes[Phonemes::PC_strong]._amount = 1.0f - frac;

    } else {
      emphasis_intensity = std::min(emphasis_intensity, STRONG_CROSSFADE_START);
      _classes[Phonemes::PC_normal]._amount = 2.0f * emphasis_intensity;
    }

  } else if (emphasis_intensity < WEAK_CROSSFADE_START) {
    if (has_weak) {
      // Blend in some of the weak.
      PN_stdfloat dist_remaining = WEAK_CROSSFADE_START - emphasis_intensity;
      PN_stdfloat frac = dist_remaining / WEAK_CROSSFADE_START;

      _classes[Phonemes::PC_normal]._amount = (1.0f - frac) * 2.0f * WEAK_CROSSFADE_START;
      _classes[Phonemes::PC_weak]._amount = frac;

    } else {
      emphasis_intensity = std::max(emphasis_intensity, WEAK_CROSSFADE_START);
      _classes[Phonemes::PC_normal]._amount = 2.0f * emphasis_intensity;
    }

  } else {
    _classes[Phonemes::PC_normal]._amount = 2.0f * emphasis_intensity;
  }
}

/**
 *
 */
void CharacterTalker::
add_viseme(PN_stdfloat emphasis_intensity, int phoneme, PN_stdfloat scale) {
  bool skip = setup_emphasis_blend(phoneme);
  if (skip) {
    return;
  }

  compute_blended_setting(emphasis_intensity);

  // Move sliders.
  for (int i = 0; i < Phonemes::PC_COUNT; ++i) {
    EmphasizedPhoneme *info = &_classes[i];
    if (!info->_valid || info->_amount == 0.0f) {
      continue;
    }

    for (size_t j = 0; j < info->_influences->size(); ++j) {
      const Phonemes::SliderInfluence &inf = (*info->_influences)[j];
      //if (inf._slider != -1) {
      //  _character->set_slider_value(inf._slider, inf._setting);
      //}
      //_key_weights[j] += inf._setting;
      info->_key_weights[j] += info->_amount * scale * inf._setting;
    }
  }
}

/**
 *
 */
SpokenSentence::Emphasis *CharacterTalker::
get_emphasis_sample(int n, PN_stdfloat length) {
  static SpokenSentence::Emphasis null_start;
  null_start._time = 0.0f;
  null_start._value = 0.5f;
  static SpokenSentence::Emphasis null_end;
  null_end._time = length;
  null_end._value = 0.5f;

  if (n < 0) {
    return &null_start;

  } else if (n >= (int)_sentence->_emphasis_samples.size()) {
    return &null_end;

  } else {
    return &_sentence->_emphasis_samples[n];
  }
}

/**
 *
 */
static void
catmull_rom_spline(const LVecBase3 &p1, const LVecBase3 &p2,
                   const LVecBase3 &p3, const LVecBase3 &p4,
                   PN_stdfloat t, LVecBase3 &out) {
  PN_stdfloat t_sqr = t * t * 0.5f;
  PN_stdfloat t_sqr_sqr = t * t_sqr;
  t *= 0.5f;

  LVecBase3 a, b, c, d;

  // Matrix row 1.
  a = p1 * -t_sqr_sqr; // 0.5 t^3 * [ (-1*p1) + ( 3*p2) + (-3*p3) + p4 ]
  b = p2 * t_sqr_sqr * 3;
  c = p3 * t_sqr_sqr * -3;
  d = p4 * t_sqr_sqr;

  out += a;
  out += b;
  out += c;
  out += d;

  // Matrix row 2.
  a = p1 * t_sqr * 2; // 0.5 t^2 * [ ( 2*p1) + (-5*p2) + ( 4*p3) - p4 ]
  b = p2 * t_sqr * -5;
  c = p2 * t_sqr * 4;
  d = p4 * -t_sqr;

  out += a;
  out += b;
  out += c;
  out += d;

  // Matrix row 3.
  a = p1 * -t; // 0.5 t * [ (-1*p1) + p3 ]
  b = p3 * t;

  out += a;
  out += b;

  // Matrix row 4.
  out += p2; // p2
}

/**
 *
 */
PN_stdfloat CharacterTalker::
get_sentence_intensity(PN_stdfloat t, PN_stdfloat length) {
  PN_stdfloat zero_value = 0.5f;

  if (_sentence->_emphasis_samples.empty()) {
    return zero_value;
  }

  int c = (int)_sentence->_emphasis_samples.size();

  int i;
  for (i = -1; i < c; ++i) {
    SpokenSentence::Emphasis *s = get_emphasis_sample(i, length);
    SpokenSentence::Emphasis *n = get_emphasis_sample(i + 1, length);
    if (s == nullptr || n == nullptr) {
      continue;
    }

    if (t >= s->_time && t <= n->_time) {
      break;
    }
  }

  int prev = i - 1;
  int start = i;
  int end = i + 1;
  int next = i + 2;

  prev = std::max(-1, prev);
  start = std::max(-1, start);
  end = std::min(end, c);
  next = std::min(next, c);

  SpokenSentence::Emphasis *e_pre = get_emphasis_sample(prev, length);
  SpokenSentence::Emphasis *e_start = get_emphasis_sample(start, length);
  SpokenSentence::Emphasis *e_end = get_emphasis_sample(end, length);
  SpokenSentence::Emphasis *e_next = get_emphasis_sample(next, length);

  PN_stdfloat dt = e_end->_time - e_start->_time;
  dt = std::clamp(dt, 0.01f, 1.0f);

  LVecBase3 v_pre(e_pre->_time, e_pre->_value, 0);
  LVecBase3 v_start(e_start->_time, e_start->_value, 0);
  LVecBase3 v_end(e_end->_time, e_end->_value, 0);
  LVecBase3 v_next(e_next->_time, e_next->_value, 0);

  PN_stdfloat f2 = (t - e_start->_time) / dt;
  f2 = std::clamp(f2, 0.0f, 1.0f);

  LVecBase3 out(0.0f);
  catmull_rom_spline(v_pre, v_start, v_end, v_next, f2, out);

  return std::clamp(out[1], 0.0f, 1.0f);
}
