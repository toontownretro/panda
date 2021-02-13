/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tokenFile.cxx
 * @author lachbr
 * @date 2021-02-12
 */

#include "tokenFile.h"
#include "config_putil.h"
#include "virtualFileSystem.h"
#include "streamReader.h"

NotifyCategoryDef(tokenfile, "util");

/**
 * Reads and tokenizes script file at the indicated filename.
 *
 * Returns true on success, or false if the file could not be read.
 */
bool TokenFile::
read(const Filename &filename) {
  Filename resolved = filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(resolved, get_model_path())) {
    return false;
  }

  if (tokenfile_cat.is_debug()) {
    tokenfile_cat.debug()
      << "Resolved the filename\n";
  }

  PT(VirtualFile) vfile = vfs->get_file(resolved);
  if (vfile == nullptr) {
    return false;
  }

  if (tokenfile_cat.is_debug()) {
    tokenfile_cat.debug()
      << "Got the virtual file\n";
  }

  std::istream *stream = vfile->open_read_file(true);
  if (stream == nullptr) {
    return false;
  }

  if (tokenfile_cat.is_debug()) {
    tokenfile_cat.debug()
      << "Successfully opened " << resolved << "\n";
  }

  bool success = tokenize(*stream);

  vfile->close_read_file(stream);

  return success;
}

/**
 * Given the indicated input stream, tokenizes the data.
 *
 * Returns true on success, or false if there was a syntax error.
 */
bool TokenFile::
tokenize(std::istream &is) {
  _root = new TokenBlock;

  IStreamWrapper *wrapper = new IStreamWrapper(&is, false);
  wrapper->acquire();

  is.seekg(0, std::ios::end);
  std::streampos length = is.tellg();
  is.seekg(0);

  if (tokenfile_cat.is_debug()) {
    tokenfile_cat.debug()
      << "Length is " << length << "\n";
  }

  StreamReader reader(&is, false);

  int block_level = 0;

  if (!r_tokenize(_root, reader, block_level, length)) {
    return false;
  }

  return true;
}

/**
 * Recursive implementation of tokenize().
 */
bool TokenFile::
r_tokenize(TokenBlock *block, StreamReader &reader, int &block_level, std::streampos length) {
  std::istream *stream = reader.get_istream();

  std::string current_token;
  char quote_character = 0;
  bool quoted_string = false;
  bool comment = false;

  while (stream->tellg() < length) {
    const char c = reader.get_int8();

    if (tokenfile_cat.is_spam()) {
      tokenfile_cat.spam()
        << std::string(&c, 1) << "\n";
    }

    // Check for a comment.
    if (!quoted_string && !comment && c == '/' && stream->tellg() < length) {
      const char next = reader.get_int8();
      if (next == '/') {
        comment = true;
        continue;

      } else {
        // Move back to where we were if it wasn't a comment.
        stream->seekg((std::streamoff)-1, std::ios::cur);
      }
    }

    if (c == '\n' || c == '\r') {
      // Line break.
      if (comment) {
        // Line break always ends a comment.
        comment = false;

      } else if (quoted_string) {
        // Got a line break but quoted string was not closed.
        tokenfile_cat.error()
          << "Unclosed quoted string at line break.\n";
        return false;

      } else if (current_token.length() != 0) {
        // End of a token.
        PT(TokenString) tok = new TokenString;
        tok->_token = current_token;
        block->_tokens.push_back(tok);
        current_token.clear();
      }

    } else if (comment) {
      // Ignore all characters while in a comment.

    } else if (c == ' ' || c == '\t') {
      // Whitespace.  If we're in a quoted string, add it to the token.
      // Otherwise, it's the end of a token.
      if (quoted_string) {
        current_token += c;

      } else if (current_token.length() != 0) {
        // End of a token.
        PT(TokenString) tok = new TokenString;
        tok->_token = current_token;
        block->_tokens.push_back(tok);
        current_token.clear();
      }

    } else if (!quoted_string && c == '{') {
      // Beginning of a nested block.
      block_level++;

      if (current_token.length() != 0) {
        // Also the end of the current token.
        PT(TokenString) tok = new TokenString;
        tok->_token = current_token;
        block->_tokens.push_back(tok);
        current_token.clear();
      }

      PT(TokenBlock) child = new TokenBlock;
      if (!r_tokenize(child, reader, block_level, length)) {
        return false;
      }
      block_level--;
      block->_tokens.push_back(child);

    } else if (!quoted_string && c == '}') {
      // End of ourselves.
      if (current_token.length() != 0) {
        // Also the end of the current token.
        PT(TokenString) tok = new TokenString;
        tok->_token = current_token;
        block->_tokens.push_back(tok);
        current_token.clear();
      }

      return true;

    } else if (quoted_string && c == quote_character) {
      // End of a quoted string, end of the token.
      PT(TokenString) tok = new TokenString;
      tok->_token = current_token;
      block->_tokens.push_back(tok);
      current_token.clear();
      quoted_string = false;
      quote_character = 0;

    } else if (!quoted_string && c == '\'' || c == '"') {
      // Beginning of a quoted string.  Quotes don't get added to the token.
      quoted_string = true;
      quote_character = c;

    } else {
      // Token character.
      current_token += c;
    }
  }

  if (quoted_string) {
    tokenfile_cat.error()
      << "Unclosed quoted string at end-of-file.\n";
    return false;
  }

  if (block_level > 0) {
    tokenfile_cat.error()
      << block_level << " unclosed block(s) at end-of-file.\n";
    return false;
  }

  return true;
}
