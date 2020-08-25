/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file keyvalues.cpp
 * @author Brian Lach
 * @date April 15, 2020
 *
 * @desc The new and much cleaner interface for dealing with Valve's KeyValue
 *	 format. KeyValues can either be created procedurally or loaded from
 *	 disk.
 *
 * Parsing code is based on the implementation from
 *https://github.com/JulioC/keyvalues-python.
 *
 */

#include "keyValues.h"

#include "virtualFileSystem.h"

NotifyCategoryDeclNoExport(keyvalues) NotifyCategoryDef(keyvalues, "")

char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A') {
    return in - ('Z' - 'z');
  }

  return in;
}

enum {
  KVTOKEN_NONE,
  KVTOKEN_BLOCK_BEGIN,
  KVTOKEN_BLOCK_END,
  KVTOKEN_STRING,
  KVTOKEN_MACROS,
};

struct KeyValueToken_t
{
  int type;
  std::string data;

  bool invalid() const { return type == KVTOKEN_NONE; }
};

class CKeyValuesTokenizer
{
public:
  CKeyValuesTokenizer(const std::string &buffer);

  KeyValueToken_t next_token();

private:
  void ignore_whitespace();
  bool ignore_comment();
  std::string get_string();

  char current();
  bool forward();
  char next();
  std::string location();

private:
  std::string _buffer;
  size_t _buflen;
  size_t _position;
  int _last_line_break;
  int _line;
};

CKeyValuesTokenizer::
    CKeyValuesTokenizer(const std::string &buffer)
{
  _buffer = buffer;
  _buflen = buffer.size();
  _position = 0;
  _last_line_break = 0;
  _line = 1;
}

KeyValueToken_t CKeyValuesTokenizer::next_token()
{
  KeyValueToken_t token;

  while (1)
  {
    ignore_whitespace();
    if (!ignore_comment())
    {
      break;
    }
  }

  // Get the next character and check if we got any character
  char c = current();
  if (!c)
  {
    token.type = KVTOKEN_NONE;
    return token;
  }

  // Emit any valid tokens
  if (c == '{')
  {
    forward();
    token.type = KVTOKEN_BLOCK_BEGIN;
    return token;
  }
  else if (c == '}')
  {
    forward();
    token.type = KVTOKEN_BLOCK_END;
    return token;
  }
  else
  {
    token.data = get_string();
    token.type = KVTOKEN_STRING;
    return token;
  }
}

std::string CKeyValuesTokenizer::get_string()
{
  bool escape = false;
  std::string result = "";

  bool quoted = false;
  if (current() == '"')
  {
    quoted = true;
    forward();
  }

  while (1)
  {
    char c = current();

    // Check if we have a character yet
    if (!c)
    {
      break;
    }

    // These characters are not part of unquoted strings.
    if (!quoted && (c == '{' || c == '}' || c == ' ' || c == '\t'))
    {
      break;
    }

    // Check if it's the end of a quoted string.
    if (!escape && quoted && c == '"')
    {
      break;
    }

    // Check if it's the end of the line
    if (c == '\n' || c == '\r') {
      if (quoted) {
        // If we reached the end of a line in a quoted string,
        // this is a syntax error.
        keyvalues_cat.error()
          << "Syntax error: reached end of line while parsing quoted string\n";
      }
      forward();
      break;
    }

    // Add the character or escape sequence to the result
    if (escape)
    {
      escape = false;

      if (c == '"')
      {
        result += '"';
      }
      else if (c == '\\')
      {
        result += '\\';
      }
    }
    else if (c == '\\')
    {
      escape = true;
    }
    else {
      result += c;
    }

    forward();
  }

  if (quoted)
  {
    forward();
  }

  return result;
}

void CKeyValuesTokenizer::ignore_whitespace()
{
  while (1)
  {
    char c = current();

    if (!c)
    {
      break;
    }

    if (c == '\n')
    {
      // Keep track of this data for debug
      _last_line_break = _position;
      _line++;
    }

    if (c != ' ' && c != '\n' && c != '\t' && c != '\r')
    {
      break;
    }

    forward();
  }
}

bool CKeyValuesTokenizer::ignore_comment()
{
  if (current() == '/' && next() == '/')
  {
    while (current() != '\n')
    {
      if (!forward()) {
        return true;
      }
    }

    return true;
  }

  return false;
}

char CKeyValuesTokenizer::current()
{
  if (_position >= _buflen)
  {
    return 0;
  }

  return _buffer[_position];
}

bool CKeyValuesTokenizer::forward()
{
  _position++;
  return _position < _buflen;
}

char CKeyValuesTokenizer::next()
{
  if ((_position + 1) >= _buflen)
  {
    return 0;
  }

  return _buffer[_position + 1];
}

std::string CKeyValuesTokenizer::location()
{
  std::ostringstream ss;
  ss << "line " << _line << ", column " << (_position - _last_line_break);
  return ss.str();
}

//------------------------------------------------------------------------------------------------

int CKeyValues::find_child(const std::string &name) const
{
  size_t count = _children.size();
  for (size_t i = 0; i < count; i++)
  {
    if (_children[i]->get_name() == name)
    {
      return (int)i;
    }
  }

  return -1;
}

pvector<CKeyValues *>
CKeyValues::get_children_with_name(const std::string &name) const
{
  pvector<CKeyValues *> result;

  size_t count = _children.size();
  for (size_t i = 0; i < count; i++)
  {
    CKeyValues *child = _children[i];
    if (child->get_name() == name)
    {
      result.push_back(child);
    }
  }

  return result;
}

void CKeyValues::parse(CKeyValuesTokenizer *tokenizer)
{
  bool has_key = false;
  std::string key;

  while (1)
  {
    KeyValueToken_t token = tokenizer->next_token();
    if (token.invalid())
    {
      // keyvalues_cat.error()
      //	<< "Unexpected end of file\n";
      break;
    }

    if (token.type == KVTOKEN_BLOCK_END)
    {
      break;
    }
    else if (token.type == KVTOKEN_BLOCK_BEGIN)
    {
      PT(CKeyValues)
      child = new CKeyValues(key, this);
      child->_filename = _filename;
      child->parse(tokenizer);
      has_key = false;
    }
    else if (token.type == KVTOKEN_STRING)
    {
      if (has_key) {
        _keyvalues[key] = token.data;
        key = "";
        has_key = false;
      } else {
        key = token.data;
        has_key = true;
      }
    }
  }
}

PT(CKeyValues)
CKeyValues::load(const Filename &filename)
{
  if (filename.empty()) {
    return nullptr;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename load_filename;
  if (filename.is_local()) {
    // Look along the model path for the file
    DSearchPath search_path(get_model_path());
    for (int i = 0; i < search_path.get_num_directories(); i++) {
      Filename search(search_path.get_directory(i), filename);
      if (vfs->exists(search)) {
        load_filename = search;
        break;
      }
    }

  } else {
    // This is an absolute filename. Use it as-is
    load_filename = filename;
  }

  if (load_filename.empty())
  {
    keyvalues_cat.error() << "Unable to find `" << filename.get_fullpath()
                          << "`\n";
    return nullptr;
  }

  std::string buffer = vfs->read_file(load_filename, true);

  PT(CKeyValues) kv = from_string(buffer);
  kv->_filename = filename;

  return kv;
}

PT(CKeyValues) CKeyValues::
from_string(const std::string &buffer) {
  CKeyValuesTokenizer tokenizer(buffer);

  PT(CKeyValues) kv = new CKeyValues;
  kv->parse(&tokenizer);

  // We should have nothing left.
  if (!tokenizer.next_token().invalid()) {
    keyvalues_cat.error() << "Unexpected EOF\n";
    return nullptr;
  }

  return kv;
}

//------------------------------------------------------------------------------------------------
// Helper functions for parsing string values that represent numbers.
//------------------------------------------------------------------------------------------------

vector_float CKeyValues::parse_float_list(const std::string &str)
{
  vector_float result;
  std::string curr_num_string;
  int current = 0;
  while (current < str.length())
  {
    char let = str[current];
    if (let == ' ')
    {
      result.push_back(stof(curr_num_string));
      curr_num_string = "";
    }
    else
    {
      curr_num_string += let;
    }
    current++;

    if (current >= str.length())
    {
      result.push_back(stof(curr_num_string));
      curr_num_string = "";
    }
  }

  return result;
}

vector_int CKeyValues::parse_int_list(const std::string &str)
{
  vector_int result;
  std::string curr_num_string;
  int current = 0;
  while (current < str.length())
  {
    char let = str[current];
    if (let == ' ')
    {
      result.push_back(stoi(curr_num_string));
      curr_num_string = "";
    }
    else
    {
      curr_num_string += let;
    }
    current++;

    if (current >= str.length())
    {
      result.push_back(stoi(curr_num_string));
      curr_num_string = "";
    }
  }

  return result;
}

pvector<vector_float>
CKeyValues::
parse_float_tuple_list(const std::string &str) {
  pvector<vector_float> result;
  int current = 0;
  std::string curr_num_string;
  vector_float tuple_result;

  while (current < str.length()) {
    char let = str[current];
    if (let == '(' || (let == ' ' && str[current - 1] == ')')) {
      tuple_result.clear();

    } else if (let == ' ' || let == ')') {
      tuple_result.push_back(std::stof(curr_num_string));
      curr_num_string = "";
      if (let == ')') {
        result.push_back(tuple_result);
      }

    } else {
      curr_num_string += let;
    }
    current++;
  }

  return result;
}

void CKeyValues::
parse_material_axis(const std::string &str, LVector3 &axis, LVector2 &shift_scale) {
  sscanf(str.c_str(), "[%f %f %f %f] %f", &axis[0], &axis[1], &axis[2], &shift_scale[0], &shift_scale[1]);
}

void CKeyValues::
parse_plane_points(const std::string &str, LPoint3 &p0, LPoint3 &p1, LPoint3 &p2) {
  sscanf(str.c_str(), "(%f %f %f) (%f %f %f) (%f %f %f)",
    &p0[0], &p0[1], &p0[2],
    &p1[0], &p1[1], &p1[2],
    &p2[0], &p2[1], &p2[2]);
}

LVecBase2f CKeyValues::
to_2f(const std::string &str) {
  vector_float vec = CKeyValues::parse_float_list(str);
  LVecBase2f lvec;
  for (size_t i = 0; i < vec.size(); i++) {
      lvec[i] = vec[i];
  }
  return lvec;
}

LVecBase3f CKeyValues::
to_3f(const std::string &str) {
  vector_float vec = CKeyValues::parse_float_list(str);
  LVecBase3f lvec;
  for (size_t i = 0; i < vec.size(); i++) {
      lvec[i] = vec[i];
  }
  return lvec;
}

LVecBase4f CKeyValues::
to_4f(const std::string &str) {
  vector_float vec = CKeyValues::parse_float_list(str);
  LVecBase4f lvec;
  for (size_t i = 0; i < vec.size(); i++) {
      lvec[i] = vec[i];
  }
  return lvec;
}

template <class T>
std::string CKeyValues::
to_string(T v) {
  return std::to_string(v);
}

template <class T>
std::string CKeyValues::
to_string(const pvector<T> &v) {
  std::string res = "";
  for (size_t i = 0; i < v.size(); i++) {
    res += to_string(v[i]);
  }

  return res;
}

std::string CKeyValues::
to_string(const LVecBase2f &v) {
  std::ostringstream ss;
  ss << v[0] << " " << v[1];
  return ss.str();
}

std::string CKeyValues::
to_string(const LVecBase3f &v) {
  std::ostringstream ss;
  ss << v[0] << " " << v[1] << " " << v[2];
  return ss.str();
}

std::string CKeyValues::
to_string(const LVecBase4f &v) {
  std::ostringstream ss;
  ss << v[0] << " " << v[1] << " " << v[2] << " " << v[3];
  return ss.str();
}

void CKeyValues::
write(const Filename &filename, int indent) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::ostringstream out;

  int curr_indent = 0;
  do_write(out, indent, curr_indent);

  vfs->write_file(filename, out.str(), false);
}

void CKeyValues::
do_indent(std::ostringstream &out, int curr_indent) {
  int indents = curr_indent;
  for (int i = 0; i < indents; i++) {
    out << " ";
  }
}

void CKeyValues::
do_write(std::ostringstream &out, int indent, int &curr_indent) {
  bool is_root = _name == root_block_name;

  // Don't write a block or keyvalues if we're the root.
  // The keyvalues format can have multiple root level blocks,
  // but can't have keyvalues outside of a block.
  if (!is_root) {
    // Open the block
    do_indent(out, curr_indent);
    out << _name << "\n";
    do_indent(out, curr_indent);
    out << "{\n";
    curr_indent += indent;

    // Write out keyvalues
    for (size_t i = 0; i < _keyvalues.size(); i++) {
      do_indent(out, curr_indent);
      out << "\"" << _keyvalues.get_key(i) << "\"" << " " << "\"" << _keyvalues.get_data(i) << "\"\n";
    }

    // Only put a line break after the keyvalues if we have child blocks.
    if (_children.size() != 0) {
      out << "\n";
    }
  }

  // Now write the child blocks
  for (size_t i = 0; i < _children.size(); i++) {
    CKeyValues *child = _children[i];
    child->do_write(out, indent, curr_indent);
    // Add an extra line break in between child blocks, but not
    // after the last child block.
    if (i != _children.size() - 1) {
      out << "\n";
    }
  }

  if (!is_root) {
    // Close the block
    curr_indent -= indent;
    do_indent(out, curr_indent);
    out << "}\n";
  }
}
