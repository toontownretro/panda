/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file phonemes.cxx
 * @author brian
 * @date 2022-02-01
 */

#include "phonemes.h"
#include "string_utils.h"
#include "tokenFile.h"
#include "vector_int.h"
#include "character.h"

/**
 * Reads in the slider phoneme combinations for a particular phoneme class
 * from the indicated filename.
 */
bool Phonemes::
read(PhonemeClass cls, Filename filename, Character *character) {
  TokenFile toks;
  if (!toks.read(filename)) {
    return false;
  }

  vector_int keys;

  pmap<PhonemeCode, std::string> phoneme_names;

  while (toks.token_available(true)) {
    toks.next_token(true);

    std::string tok = toks.get_token();

    if (tok == "$keys") {
      while (toks.token_available()) {
        toks.next_token();
        keys.push_back(character->find_slider(toks.get_token()));
      }

    } else if (tok == "$hasweighting" || tok == "$normalized") {

    } else {
      // Phoneme definition.
      std::string phoneme_name = tok;
      toks.next_token();
      std::string phoneme_code_str = toks.get_token();

      int code;
      if (phoneme_code_str[1] == 'x') {
        // Hex string.
        sscanf(&phoneme_code_str[2], "%x", &code);

      } else {
        // Single ASCII character.
        code = (int)phoneme_code_str[0];
      }

      phoneme_names[code] = phoneme_name;

      for (size_t i = 0; i < keys.size(); ++i) {
        toks.next_token();
        PN_stdfloat setting = toks.get_numeric_token();
        toks.next_token();
        PN_stdfloat weight = toks.get_numeric_token();

        add_phoneme_slider(cls, code, keys[i], setting, weight);
      }

      // Skip string description
      toks.next_token();
    }
  }

  _class_keys[cls] = keys;

  for (int slider : keys) {
    if (std::find(_all_keys.begin(), _all_keys.end(), slider) == _all_keys.end()) {
      _all_keys.push_back(slider);
    }
  }

  return true;
}

/**
 *
 */
void Phonemes::
add_phoneme_slider(PhonemeClass cls, PhonemeCode code, int slider, PN_stdfloat setting, PN_stdfloat weight) {
  auto it = _mappings[cls].find(code);
  if (it == _mappings[cls].end()) {
    _mappings[cls][code] = SliderInfluences();
  }

  SliderInfluence inf;
  inf._slider = slider;
  inf._setting = setting;
  inf._weight = weight;
  _mappings[cls][code].push_back(inf);
}

/**
 *
 */
int Phonemes::
get_num_phoneme_sliders(PhonemeClass cls, PhonemeCode code) const {
  auto it = _mappings[cls].find(code);
  if (it == _mappings[cls].end()) {
    return 0;
  }

  return (int)((*it).second.size());
}

/**
 *
 */
int Phonemes::
get_phoneme_slider(PhonemeClass cls, PhonemeCode code, int n, PN_stdfloat &setting, PN_stdfloat &weight) const {
  auto it = _mappings[cls].find(code);
  if (it == _mappings[cls].end()) {
    return -1;
  }

  const SliderInfluences &infs = (*it).second;

  nassertr(n >= 0 && n < (int)infs.size(), -1);

  setting = infs[n]._setting;
  weight = infs[n]._weight;
  return infs[n]._slider;
}
