/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tokenFile.I
 * @author lachbr
 * @date 2021-02-12
 */

/**
 *
 */
INLINE TokenBlock::
TokenBlock() {
  _token_index = 0;
}

/**
 * Returns the next token in the block, or NULL if there is no next token.
 */
INLINE TokenBase *TokenBlock::
next() {
  if (_token_index >= _tokens.size() - 1) {
    return nullptr;
  }

  return _tokens[_token_index++];
}

/**
 * Returns the previous token in the block, or NULL if there is no previous
 * token.
 */
INLINE TokenBase *TokenBlock::
prev() {
  if (_token_index == 0) {
    return nullptr;
  }

  return _tokens[--_token_index];
}

/**
 *
 */
INLINE TokenFile::
TokenFile() {
}

/**
 * Returns the root TokenBlock of the TokenFile.
 */
INLINE TokenBlock *TokenFile::
get_root() const {
  return _root;
}
