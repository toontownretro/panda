/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tokenFile.h
 * @author lachbr
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
 * Generic script file tokenizer with nested block support.  File is chopped
 * up into tokens that can be iterated through and interpreted as needed.
 */
class EXPCL_PANDA_PUTIL TokenFile : public ReferenceCount {
PUBLISHED:
  struct Token {
    std::string _data;
    // Is the token on a new line?
    bool _newline;
    int _line_number;
  };

  INLINE TokenFile();

  bool read(const Filename &filename, const DSearchPath &search_path = get_model_path());
  bool tokenize(std::istream &stream);

  bool next_token(bool cross_line = false);

  bool token_available(bool cross_line = false) const;

  std::string get_token() const;

  INLINE const Filename &get_filename() const;
  INLINE const Filename &get_fullpath() const;

private:
  pvector<Token> _tokens;
  Token *_token;
  int _token_index;

  Filename _filename;
  Filename _fullpath;
};

#include "tokenFile.I"

#endif // TOKENFILE_H
