/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tokenFile.h
 * @author brian
 * @date 2021-02-12
 */

#ifndef TOKENFILE_H
#define TOKENFILE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pvector.h"
#include "pointerTo.h"
#include "config_putil.h"

NotifyCategoryDecl(tokenfile, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);

/**
 * Generic text file tokenizer.  File is chopped up into tokens that can be
 * iterated through and interpreted as needed.
 */
class EXPCL_PANDA_PUTIL TokenFile : public ReferenceCount {
PUBLISHED:
  enum TokenType {
    TT_invalid = -1,

    // "example?!{}[]"
    TT_string,
    // ? ! : { } [ ] .
    TT_symbol,
    // 1 2 3 -4 -5
    TT_integer,
    // 0.1 2.3 -4.5
    TT_float,
    // Non-quoted, non-spaced set of letters only.
    TT_word,
  };

  class Token {
  public:
    INLINE Token();

    std::string _data;
    PN_stdfloat _numeric_data;
    // Is the token on a new line?
    bool _newline;
    int _line_number;

    Token *_next;

    TokenType _type;
  };

  INLINE TokenFile();
  ~TokenFile();

  void set_symbols(const std::string &symbols);

  bool read(Filename filename, const DSearchPath &search_path = get_model_path());
  bool tokenize(std::istream &stream);

  bool next_token(bool cross_line = false);

  bool token_available(bool cross_line = false) const;

  std::string get_token() const;
  PN_stdfloat get_numeric_token() const;
  TokenType get_token_type() const;

  INLINE const Filename &get_filename() const;
  INLINE const Filename &get_fullpath() const;

private:
  void add_token(Token *token);

  bool is_word_character(char c) const;

private:
  // Head of linked list.
  Token *_tokens;

  Token *_token;
  int _num_tokens;

  std::string _symbols;

  Filename _filename;
  Filename _fullpath;
};

#include "tokenFile.I"

#endif // TOKENFILE_H
