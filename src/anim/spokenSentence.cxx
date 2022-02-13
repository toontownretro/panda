/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spokenSentence.cxx
 * @author brian
 * @date 2022-02-10
 */

#include "spokenSentence.h"
#include "tokenFile.h"
#include "filename.h"

/**
 * xyz
 */
SpokenSentence::
SpokenSentence() :
  _voice_duck(false),
  _length(0.0f)
{
}

/**
 * Parses the sentence words from the given TokenFile.
 */
void SpokenSentence::
parse_sentence(TokenFile *tokens) {

  while (true) {
    if (!tokens->token_available(true)) {
      std::cerr << "EOF while parsing sentence\n";
      return;
    }

    tokens->next_token(true);
    std::string section = tokens->get_token();
    if (section.empty() || section == "}") {
      break;
    }

    tokens->next_token(true);
    std::string tok = tokens->get_token();

    if (tok != "{") {
      break;
    }

    if (section == "PLAINTEXT") {
      parse_plain_text(tokens);

    } else if (section == "WORDS") {
      parse_words(tokens);

    } else if (section == "EMPHASIS") {
      parse_emphasis(tokens);

    } else if (section == "CLOSECAPTION") {
      parse_close_caption(tokens);

    } else if (section == "OPTIONS") {
      parse_options(tokens);
    }
  }
}

/**
 * Parses the plain-text representation of the sentence.
 */
void SpokenSentence::
parse_plain_text(TokenFile *tokens) {
  while (true) {
    tokens->next_token(true);

    std::string tok = tokens->get_token();

    if (tok == "}") {
      break;

    } else {
      _sentence += tok + " ";
    }
  }
}

/**
 *
 */
void SpokenSentence::
parse_words(TokenFile *tokens) {
  while (true) {
    tokens->next_token(true);

    std::string tok = tokens->get_token();

    if (tok == "}") {
      break;
    }

    if (tok != "WORD") {
      break;
    }

    tokens->next_token();
    std::string word_str = tokens->get_token();

    tokens->next_token();
    PN_stdfloat start_time = tokens->get_numeric_token();
    tokens->next_token();
    PN_stdfloat end_time = tokens->get_numeric_token();

    Word word;
    word._word = word_str;
    word._start_time = start_time;
    word._end_time = end_time;

    tokens->next_token(true);
    tok = tokens->get_token();
    if (tok != "{") {
      break;
    }

    // Parse word phonemes.
    while (true) {
      tokens->next_token(true);
      tok = tokens->get_token();

      if (tok == "}") {
        break;
      }

      WordPhoneme wp;
      wp._phoneme_code = tokens->get_numeric_token();

      tokens->next_token();
      wp._phoneme_name = tokens->get_token();

      tokens->next_token();
      wp._start_time = tokens->get_numeric_token();
      tokens->next_token();
      wp._end_time = tokens->get_numeric_token();
      tokens->next_token();
      wp._volume = tokens->get_numeric_token();

      word._phonemes.push_back(wp);
      _all_phonemes.push_back(wp);

      _length = std::max(_length, wp._end_time);
    }

    _words.push_back(word);
  }
}

/**
 *
 */
void SpokenSentence::
parse_emphasis(TokenFile *tokens) {
  while (true) {
    tokens->next_token(true);

    std::string tok = tokens->get_token();

    if (tok == "}") {
      break;
    }

    Emphasis emph;
    emph._time = tokens->get_numeric_token();
    tokens->next_token();
    emph._value = tokens->get_numeric_token();

    _emphasis_samples.push_back(emph);
  }
}

/**
 *
 */
void SpokenSentence::
parse_options(TokenFile *tokens) {
  while (true) {
    tokens->next_token(true);
    if (tokens->get_token() == "}") {
      break;
    }

    if (tokens->get_token().empty()) {
      break;
    }

    std::string key = tokens->get_token();
    tokens->next_token();
    std::string value = tokens->get_token();

    if (key == "voice_duck") {
      _voice_duck = (bool)atoi(value.c_str());

    } // Checksum ignored.
  }
}

/**
 *
 */
void SpokenSentence::
parse_close_caption(TokenFile *tokens) {
  // Unused, so just skip over all the tokens to the end.
  while (true) {
    tokens->next_token(true);
    if (tokens->get_token() == "}") {
      break;
    }
  }
}

/**
 * Reads the sentence definitions from the indicated filename.
 */
bool SentenceCollection::
load(const Filename &filename) {
  TokenFile toks;
  toks.set_symbols("{}");
  if (!toks.read(filename)) {
    return false;
  }

  while (toks.token_available(true)) {
    toks.next_token(true);

    std::string sentence_name = toks.get_token();
    toks.next_token(true);
    if (toks.get_token() != "{") {
      std::cerr << "Expected opening brace after sentence name, got " << toks.get_token() << "\n";
      return false;
    }

    // Skip version 1.0
    toks.next_token(true);
    toks.next_token();

    PT(SpokenSentence) sentence = new SpokenSentence;
    sentence->parse_sentence(&toks);

    _sentences[Filename::from_os_specific(sentence_name).get_fullpath()] = sentence;
  }

  return true;
}

/**
 * Returns the SpokenSentence with the indicated name, or nullptr if no
 * sentence exists with that name.
 */
SpokenSentence *SentenceCollection::
get_sentence(const std::string &name) const {
  int it = _sentences.find(name);
  if (it != -1) {
    return _sentences.get_data(it);
  }

  return nullptr;
}

/**
 *
 */
int SentenceCollection::
get_num_sentences() const {
  return (int)_sentences.size();
}

/**
 *
 */
SpokenSentence *SentenceCollection::
get_sentence(int n) const {
  nassertr(n >= 0 && n < (int)_sentences.size(), nullptr);
  return _sentences.get_data(n);
}

/**
 *
 */
std::string SentenceCollection::
get_sentence_name(int n) const {
  nassertr(n >= 0 && n < (int)_sentences.size(), std::string());
  return _sentences.get_key(n);
}
