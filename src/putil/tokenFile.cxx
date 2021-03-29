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
read(Filename filename, const DSearchPath &search_path) {
  filename.set_binary();

  Filename resolved = filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(resolved, search_path)) {
    return false;
  }

  if (tokenfile_cat.is_debug()) {
    tokenfile_cat.debug()
      << "Resolved the filename\n";
  }

  _filename = filename;
  _fullpath = resolved;

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

  if (tokenfile_cat.is_debug()) {
    tokenfile_cat.debug()
      << "Tokenize result: " << success << "\n";
    tokenfile_cat.debug()
      << "We have " << _num_tokens << " tokens\n";
  }

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
  _tokens = new Token;
  _tokens->_data = "START";
  _num_tokens = 0;
  _token = _tokens;

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

  std::string current_token;
  char quote_character = 0;
  bool quoted_string = false;
  bool comment = false;
  bool new_line = true;
  bool line_escape = false;
  int line_number = 1;

  while (is.tellg() < length) {
    const char c = reader.get_int8();

    // Check for a comment.
    if (!quoted_string && !comment && c == '/' && is.tellg() < length) {
      const char next = reader.get_int8();
      if (next == '/') {
        comment = true;

        // The beginning of a comment ends the current token.
        if (current_token.length() != 0) {
          PT(Token) tok = new Token;
          tok->_data = current_token;
          tok->_newline = new_line;
          tok->_line_number = line_number;
          add_token(tok);
          new_line = false;
          current_token.clear();
        }

        continue;

      } else {
        // Move back to where we were if it wasn't a comment.
        is.seekg((std::streamoff)-1, std::ios::cur);
      }
    }

    if (c == '\\') {
      line_escape = true;
    } else if (c != '\n' && c != '\r' && c != ' ' && c != '\t') {
      line_escape = false;
    }

    if (c == '\r' || c == '\n') {

      if (c == '\r') {
        // We got a carriage return, indicating a line break.  Skip over the
        // line feed if that's the next character.

        if (is.tellg() < length) {
          const char next = reader.get_int8();
          if (next != '\n') {
            // If the next character is not a line feed, so we should return to
            // where we were.
            is.seekg((std::streamoff)-1, std::ios::cur);
          }
        }
      }
      // Line break.
      if (comment) {
        // Line break always ends a comment.
        comment = false;

      } else if (quoted_string) {
        // Got a line break but quoted string was not closed.
        tokenfile_cat.error()
          << "Unclosed quoted string at line break (line " << line_number << ").\n";
        return false;

      } else if (current_token.length() != 0) {
        // End of a token.
        PT(Token) tok = new Token;
        tok->_data = current_token;
        tok->_newline = new_line;
        tok->_line_number = line_number;
        add_token(tok);
        current_token.clear();
      }

      new_line = !line_escape;
      line_number++;

      line_escape = false;

    } else if (comment) {
      // Ignore all characters while in a comment.

    } else if (c == ' ' || c == '\t') {
      // Whitespace.  If we're in a quoted string, add it to the token.
      // Otherwise, it's the end of a token.
      if (quoted_string) {
        current_token += c;

      } else if (current_token.length() != 0) {
        // End of a token.
        PT(Token) tok = new Token;
        tok->_data = current_token;
        tok->_newline = new_line;
        tok->_line_number = line_number;
        add_token(tok);
        new_line = false;
        current_token.clear();
      }

    } else if (quoted_string && c == quote_character) {
      // End of a quoted string, end of the token.
      PT(Token) tok = new Token;
      tok->_data = current_token;
      tok->_newline = new_line;
      tok->_line_number = line_number;
      add_token(tok);
      new_line = false;
      current_token.clear();
      quoted_string = false;
      quote_character = 0;

    } else if (!quoted_string && c == '"') {
      // Beginning of a quoted string.  Quotes don't get added to the token.
      quoted_string = true;
      quote_character = c;

      // Should also end the current token.
      if (current_token.length() != 0) {
        PT(Token) tok = new Token;
        tok->_data = current_token;
        tok->_newline = new_line;
        tok->_line_number = line_number;
        add_token(tok);
        new_line = false;
        current_token.clear();
      }

    } else {
      // Token character.
      current_token += c;
    }
  }

  // Move to head of token list.
  _token = _tokens;

  if (quoted_string) {
    tokenfile_cat.error()
      << "Unclosed quoted string at end-of-file.\n";
    return false;
  }

  return true;
}

/**
 * Retrieves the next token.  If cross_line is true, allows the next token to
 * be on a new line.
 *
 * Returns true if the next token was retrieved, or false if there is no next
 * token, or cross_line was false and the next token was on a new line.
 */
bool TokenFile::
next_token(bool cross_line) {
  if (_token->_next == nullptr || _token == nullptr) {
    // End of all tokens.
    if (!cross_line && _token) {
      tokenfile_cat.error()
        << "Line " << _token->_line_number << " is incomplete!\n";
    }
    _token = nullptr;
    return false;
  }

  Token *token = _token->_next;
  if (!cross_line && token->_newline) {
    // Token is on a new line, but we didn't want it to be on a new line.
    if (_token) {
      tokenfile_cat.error()
        << "Line " << _token->_line_number << " is incomplete!\n";
    }
    _token = nullptr;
    return false;
  }

  // Token is valid, store it off.
  _token = token;
  return true;
}

/**
 * Returns true if a next token is available.
 *
 * If cross_line is false, the next token must be on the same line as the
 * current token.
 */
bool TokenFile::
token_available(bool cross_line) const {
  if (_token->_next == nullptr) {
    return false;
  }

  if (!cross_line) {
    return !_token->_next->_newline;
  } else {
    return true;
  }
}

/**
 * Returns the value of the currently stored token, or empty string if there is
 * no current token.
 */
std::string TokenFile::
get_token() const {
  if (_token == nullptr) {
    return std::string();
  }

  return _token->_data;
}

/**
 *
 */
void TokenFile::
add_token(Token *tok) {
  _token->_next = tok;
  _token = tok;
}
