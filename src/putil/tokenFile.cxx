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
 * Returns true if the character is a valid word character, false otherwise.
 */
bool TokenFile::
is_word_character(char c) const {
  return strchr(_symbols.c_str(), c) == nullptr && c != '"';
}

/**
 *
 */
TokenFile::
~TokenFile() {
  // Free all of our tokens.
  Token *token = _tokens;
  while (token != nullptr) {
    Token *tmp = token;
    token = token->_next;
    delete tmp;
  }
}

/**
 * Sets the string of characters that should be interpreted as symbol tokens.
 */
void TokenFile::
set_symbols(const std::string &symbols) {
  _symbols = symbols;
}

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
  bool comment = false;
  bool new_line = true;
  bool line_escape = false;
  TokenType token_type = TT_invalid;
  int line_number = 1;

  while (is.tellg() < length) {
    const char c = reader.get_int8();

    // Check for a comment.
    if (token_type != TT_string && !comment && c == '/' && is.tellg() < length) {
      const char next = reader.get_int8();
      if (next == '/') {
        comment = true;

        // The beginning of a comment ends the current token.
        if (current_token.length() != 0) {
          Token *tok = new Token;
          tok->_data = current_token;
          tok->_newline = new_line;
          tok->_line_number = line_number;
          tok->_type = token_type;
          add_token(tok);
          new_line = false;
          current_token.clear();
          token_type = TT_invalid;
        }

        continue;

      } else {
        // Move back to where we were if it wasn't a comment.
        is.seekg((std::streamoff)-1, std::ios::cur);
      }
    }

    if (c == '\\' && token_type != TT_string) {
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

      } else if (token_type == TT_string) {
        // Got a line break but quoted string was not closed.
        tokenfile_cat.error()
          << "Unclosed quoted string at line break (line " << line_number << ").\n";
        return false;

      } else if (current_token.length() != 0) {
        // End of a token.
        Token *tok = new Token;
        tok->_data = current_token;
        tok->_newline = new_line;
        tok->_line_number = line_number;
        tok->_type = token_type;
        add_token(tok);
        current_token.clear();
        token_type = TT_invalid;
      }

      new_line = !line_escape;
      line_number++;

      line_escape = false;

    } else if (comment) {
      // Ignore all characters while in a comment.

    } else if (c == ' ' || c == '\t') {
      // Whitespace.  If we're in a quoted string, add it to the token.
      // Otherwise, it's the end of a token.
      if (token_type == TT_string) {
        current_token += c;

      } else if (current_token.length() != 0) {
        // End of a token.
        Token *tok = new Token;
        tok->_data = current_token;
        tok->_newline = new_line;
        tok->_line_number = line_number;
        tok->_type = token_type;
        add_token(tok);
        new_line = false;
        current_token.clear();
        token_type = TT_invalid;
      }

    } else if (current_token.length() == 0 && token_type != TT_string) {
      // Beginning of a token.  Determine the token type.

      if (c == '"') {
        token_type = TT_string;

      } else {
        if (c == '+' || c == '-' || c == '.') {
          // This might be the beginning of a numeric token.  Look ahead to
          // confirm it.
          bool got_numeric = false;

          if (is.tellg() < length) {
            unsigned char next = reader.get_int8();
            if (c == '.') {
              // If it's a dot, the next character must be a digit to be numeric.
              if (isdigit(next)) {
                token_type = TT_float;
                got_numeric = true;
              }

            } else if (next == '.') {
              // If the current character is + or -, and the next character is a dot,
              // then the character after that must be a digit.
              if (is.tellg() < length) {
                unsigned char nextnext = reader.get_int8();
                if (isdigit(nextnext)) {
                  token_type = TT_float;
                  got_numeric = true;
                }

                is.seekg((std::streamoff)-1, std::ios::cur);
              }

            } else if (isdigit(next)) {
              token_type = TT_integer;
              got_numeric = true;
            }

            is.seekg((std::streamoff)-1, std::ios::cur);
          }

          if (!got_numeric) {
            // It's just a symbol then.
            token_type = TT_symbol;
          }

        } else if (isdigit(c)) {
          token_type = TT_integer;

        } else if (strchr(_symbols.c_str(), c) == nullptr) {
          // It's the beginning of a word
          token_type = TT_word;

        } else {
          token_type = TT_symbol;
        }

        current_token += c;

        if (token_type == TT_symbol) {
          // Each individual symbol is a token.
          Token *tok = new Token;
          tok->_data = current_token;
          tok->_newline = new_line;
          tok->_line_number = line_number;
          tok->_type = token_type;
          add_token(tok);
          new_line = false;
          current_token.clear();
          token_type = TT_invalid;
        }
      }
    } else {
      assert(token_type != TT_symbol);

      // In a token.
      if (token_type == TT_string) {
        if (c == '"') {
          // End of the string.
          Token *tok = new Token;
          tok->_data = current_token;
          tok->_newline = new_line;
          tok->_line_number = line_number;
          tok->_type = token_type;
          add_token(tok);
          new_line = false;
          current_token.clear();
          token_type = TT_invalid;
        } else {
          // Add all characters to the quoted string.
          current_token += c;
        }

      } else if (token_type == TT_word) {

        if (!is_word_character(c)) {
          // We are in a word and this character is not a valid word character.
          // End of word.
          Token *tok = new Token;
          tok->_data = current_token;
          tok->_newline = new_line;
          tok->_line_number = line_number;
          tok->_type = token_type;
          token_type = TT_invalid;
          add_token(tok);
          new_line = false;
          current_token.clear();

          // Move back to process this character next iteration.
          is.seekg((std::streamoff)-1, std::ios::cur);
        } else {
          current_token += c;
        }

      } else if (token_type == TT_integer || token_type == TT_float) {
        if (isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'E' || c == 'e') {
          if (c == '.') {
            token_type = TT_float;
          }
          current_token += c;
        } else {
          Token *tok = new Token;
          tok->_data = current_token;
          tok->_newline = new_line;
          tok->_line_number = line_number;
          tok->_type = token_type;
          token_type = TT_invalid;
          add_token(tok);
          new_line = false;
          current_token.clear();

          // Move back to process this character next iteration.
          is.seekg((std::streamoff)-1, std::ios::cur);
        }
      }
    }
  }

  // Move to head of token list.
  _token = _tokens;

  if (token_type == TT_string) {
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
 * Returns the numeric value of the currently stored token, or 0 if the token
 * is not numeric or there is no current token.
 */
PN_stdfloat TokenFile::
get_numeric_token() const {
  if (_token == nullptr || (_token->_type != TT_integer && _token->_type != TT_float)) {
    return 0.0;
  }

  return _token->_numeric_data;
}

/**
 * Returns the type of the current token, or TT_invalid if there is no current
 * token.
 */
TokenFile::TokenType TokenFile::
get_token_type() const {
  if (_token == nullptr) {
    return TT_invalid;
  }

  return _token->_type;
}

/**
 *
 */
void TokenFile::
add_token(Token *tok) {
  _token->_next = tok;
  _token = tok;
  if (tok->_type == TT_integer || tok->_type == TT_float) {
    tok->_numeric_data = atof(tok->_data.c_str());
  }
}
