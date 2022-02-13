/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spokenSentence.h
 * @author brian
 * @date 2022-02-10
 */

#ifndef SPOKENSENTENCE_H
#define SPOKENSENTENCE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pmap.h"
#include "pvector.h"
#include "pointerTo.h"
#include "simpleHashMap.h"

class TokenFile;

/**
 * Defines a sentence spoken by an animated character.  Contains a list
 * of words along the timeline of an audio track that contain individual
 * phonemes which map to slider weights for the character speaking.
 */
class EXPCL_PANDA_ANIM SpokenSentence : public ReferenceCount {
PUBLISHED:
  SpokenSentence();

  /**
   * Defines a point in the timeline of the audio track in which emphasis
   * should be given to specific word or phoneme.  Maps to increased/decreased
   * phoneme slider weights.
   */
  class Emphasis {
  public:
    PN_stdfloat _time;
    PN_stdfloat _value;
  };

  /**
   * Defines a single phoneme of a word in the sentence.
   */
  class WordPhoneme {
  public:
    // Start and end points of this phoneme along the timeline of the
    // associated audio track.  Should be contained within the start and
    // end time of the word the phoneme belongs to.
    PN_stdfloat _start_time;
    PN_stdfloat _end_time;
    PN_stdfloat _volume;
    unsigned short _phoneme_code;
    std::string _phoneme_name;
  };

  /**
   * Defines a single word in the sentence that is spoken at some point in the
   * timeline of the audio track.  Contains a list of phonemes that make up
   * the word.
   */
  class Word {
  public:
    // Start and end points of this word along the timeline of the associated
    // audio track.
    PN_stdfloat _start_time;
    PN_stdfloat _end_time;

    // Text version of the word.
    std::string _word;

    pvector<WordPhoneme> _phonemes;
  };

  pvector<Word> _words;
  pvector<WordPhoneme> _all_phonemes;
  pvector<Emphasis> _emphasis_samples;

  // Plain text representation of sentence.
  std::string _sentence;

  bool _voice_duck;

  PN_stdfloat _length;

  void parse_sentence(TokenFile *tokens);
  void parse_plain_text(TokenFile *tokens);
  void parse_words(TokenFile *tokens);
  void parse_emphasis(TokenFile *tokens);
  void parse_options(TokenFile *tokens);
  void parse_close_caption(TokenFile *tokens);
};

/**
 * A collection of spoken sentences by name.
 */
class EXPCL_PANDA_ANIM SentenceCollection {
PUBLISHED:
  SentenceCollection() = default;

  bool load(const Filename &filename);

  SpokenSentence *get_sentence(const std::string &name) const;
  int get_num_sentences() const;
  SpokenSentence *get_sentence(int n) const;
  std::string get_sentence_name(int n) const;

private:
  SimpleHashMap<std::string, PT(SpokenSentence), string_hash> _sentences;
};

#include "spokenSentence.I"

#endif // SPOKENSENTENCE_H
