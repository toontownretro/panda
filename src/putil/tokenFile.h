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

NotifyCategoryDecl(tokenfile, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);

class TokenBlock;
class TokenString;
class StreamReader;

/**
 * Base class for TokenBlock and TokenString.
 */
class EXPCL_PANDA_PUTIL TokenBase : public ReferenceCount {
PUBLISHED:
  virtual TokenBlock *as_block() { return nullptr; }
  virtual TokenString *as_string() { return nullptr; }
};

/**
 * A single string token from a TokenFile.
 */
class EXPCL_PANDA_PUTIL TokenString : public TokenBase {
PUBLISHED:
  virtual TokenString *as_string() { return this; }

  const std::string &get_token() const { return _token; }

private:
  std::string _token;

  friend class TokenFile;
};

/**
 * A token that contains TokenStrings, and possibly nested TokenBlocks.
 */
class EXPCL_PANDA_PUTIL TokenBlock : public TokenBase {
PUBLISHED:
  INLINE TokenBlock();

  virtual TokenBlock *as_block() { return this; }

  INLINE TokenBase *next();
  INLINE TokenBase *prev();

private:
  pvector<PT(TokenBase)> _tokens;
  size_t _token_index;

  friend class TokenFile;
};

/**
 * Generic script file tokenizer with nested block support.  File is chopped
 * up into tokens that can be iterated through and interpreted as needed.
 */
class EXPCL_PANDA_PUTIL TokenFile : public ReferenceCount {
PUBLISHED:
  INLINE TokenFile();

  bool read(const Filename &filename);
  bool tokenize(std::istream &stream);
  bool r_tokenize(TokenBlock *block, StreamReader &reader, int &block_level, std::streampos length);

  INLINE TokenBlock *get_root() const;

private:
  PT(TokenBlock) _root;
};

#include "tokenFile.I"

#endif // TOKENFILE_H
