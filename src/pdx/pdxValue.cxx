/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxValue.cxx
 * @author brian
 * @date 2021-06-10
 */

#include "pdxValue.h"
#include "pdxElement.h"
#include "pdxList.h"
#include "indent.h"
#include "tokenFile.h"
#include "config_pdx.h"
#include "virtualFileSystem.h"

/**
 *
 */
PDXValue::
PDXValue() {
  clear();
}

/**
 *
 */
PDXValue::
~PDXValue() {
  clear();
}

/**
 *
 */
PDXValue::
PDXValue(const PDXValue &copy) {
  operator = (copy);
}

/**
 *
 */
PDXValue::
PDXValue(PDXValue &&other) {
  operator = (std::move(other));
}

/**
 *
 */
void PDXValue::
operator = (const PDXValue &copy) {
  // NOTE: Elements and lists are copied by reference.
  _value_type = copy._value_type;
  _value = copy._value;
}

/**
 *
 */
void PDXValue::
operator = (PDXValue &&other) {
  _value_type = std::move(other._value_type);
  _value = std::move(other._value);
}

/**
 *
 */
PDXValue::
PDXValue(const std::string &value) {
  set_string(value);
}

/**
 *
 */
PDXValue::
PDXValue(bool value) {
  set_bool(value);
}

/**
 *
 */
PDXValue::
PDXValue(float value) {
  set_float(value);
}

/**
 *
 */
PDXValue::
PDXValue(int value) {
  set_int(value);
}

/**
 *
 */
PDXValue::
PDXValue(PDXElement *value) {
  set_element(value);
}

/**
 *
 */
PDXValue::
PDXValue(PDXList *value) {
  set_list(value);
}

/**
 *
 */
void PDXValue::
set_string(const std::string &value) {
  _value_type = VT_string;
  _value = value;
}

/**
 *
 */
void PDXValue::
set_bool(bool value) {
  _value_type = VT_boolean;
  _value = value;
}

/**
 *
 */
bool PDXValue::
get_bool() const {
  // Any value type can be evaluated as a boolean.
  switch (_value_type) {
  case VT_boolean:
    return std::get<bool>(_value);
  case VT_int:
    return std::get<int>(_value) != 0;
  case VT_float:
    return std::get<float>(_value) != 0.0f;
  case VT_string:
    return std::get<std::string>(_value).length() != 0;
  case VT_list:
    return std::get<PT(PDXList)>(_value)->size() > 0;
  case VT_element:
    return std::get<PT(PDXElement)>(_value)->get_num_attributes() > 0;
  default:
#ifndef NDEBUG
    nassert_raise("PDXValue::get_bool(): Value type can not be evaluated as boolean");
#endif
    return false;
  }
}

/**
 *
 */
void PDXValue::
set_float(float value) {
  _value_type = VT_float;
  _value = value;
}

/**
 *
 */
void PDXValue::
set_int(int value) {
  _value_type = VT_int;
  _value = value;
}

/**
 * Reads the PDX file from the indicated filename into this PDXValue.  Returns
 * true on success, or false if the file could not be read or parsed.
 */
bool PDXValue::
read(const Filename &filename, const DSearchPath &search_path) {
  TokenFile tokens;
  tokens.local_object();
  if (!tokens.read(filename, search_path)) {
    pdx_cat.error()
      << "Failed to read PDX file " << filename << "\n";
    return false;
  }

  return parse(&tokens);
}

/**
 * Writes this PDXValue (and all nested PDXValues) to the indicated filename.
 * Returns true on success, or false if the indicated file could not be opened
 * for writing.
 */
bool PDXValue::
write(const Filename &filename) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::ostream *out = vfs->open_write_file(filename, false, true);
  if (out == nullptr) {
    pdx_cat.error()
      << "Failed to write PDX file " << filename << ", unable to open file "
      << "for writing.\n";
    return false;
  }

  write(*out, 0);

  vfs->close_write_file(out);

  return true;
}

/**
 *
 */
void PDXValue::
set_element(PDXElement *value) {
  _value_type = VT_element;
  _value = PT(PDXElement)(value);
}

/**
 *
 */
void PDXValue::
set_list(PDXList *value) {
  _value_type = VT_list;
  _value = PT(PDXList)(value);
}

/**
 *
 */
void PDXValue::
clear() {
  _value_type = VT_invalid;
  _value = 0;
}

/**
 *
 */
bool PDXValue::
to_vec2(LVecBase2 &vec) const {
  if (!is_list()) {
    return false;
  }

  PDXList *list = get_list();

  for (size_t i = 0; i < list->size() && i < 2; i++) {
    const PDXValue &val = list->get(i);
    vec[i] = val.get_float();
  }

  return true;
}

/**
 *
 */
void PDXValue::
from_vec2(const LVecBase2 &vec) {
  PT(PDXList) list = new PDXList;
  list->append(vec[0]);
  list->append(vec[1]);
  set_list(list);
}

/**
 *
 */
bool PDXValue::
to_vec3(LVecBase3 &vec) const {
  if (!is_list()) {
    return false;
  }

  PDXList *list = get_list();

  for (size_t i = 0; i < list->size() && i < 3; i++) {
    const PDXValue &val = list->get(i);
    vec[i] = val.get_float();
  }

  return true;
}

/**
 *
 */
void PDXValue::
from_vec3(const LVecBase3 &vec) {
  PT(PDXList) list = new PDXList;
  list->append(vec[0]);
  list->append(vec[1]);
  list->append(vec[2]);
  set_list(list);
}

/**
 *
 */
bool PDXValue::
to_vec4(LVecBase4 &vec) const {
  if (!is_list()) {
    return false;
  }

  PDXList *list = get_list();

  for (size_t i = 0; i < list->size() && i < 4; i++) {
    const PDXValue &val = list->get(i);
    vec[i] = val.get_float();
  }

  return true;
}

/**
 *
 */
void PDXValue::
from_vec4(const LVecBase4 &vec) {
  PT(PDXList) list = new PDXList;
  list->append(vec[0]);
  list->append(vec[1]);
  list->append(vec[2]);
  list->append(vec[3]);
  set_list(list);
}

/**
 *
 */
bool PDXValue::
to_mat3(LMatrix3 &mat) const {
  if (!is_list()) {
    return false;
  }

  PDXList *list = get_list();

  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      size_t index = (i * 3) + j;
      if (index >= list->size()) {
        return false;
      }
      const PDXValue &val = list->get(index);
      mat[i][j] = val.get_float();
    }
  }

  return true;
}

/**
 *
 */
void PDXValue::
from_mat3(const LMatrix3 &mat) {
  PT(PDXList) list = new PDXList;
  list->append(mat[0][0]);
  list->append(mat[0][1]);
  list->append(mat[0][2]);
  list->append(mat[1][0]);
  list->append(mat[1][1]);
  list->append(mat[1][2]);
  list->append(mat[2][0]);
  list->append(mat[2][1]);
  list->append(mat[2][2]);
  set_list(list);
}

/**
 *
 */
bool PDXValue::
to_mat4(LMatrix4 &mat) const {
  if (!is_list()) {
    return false;
  }

  PDXList *list = get_list();

  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      size_t index = (i * 4) + j;
      if (index >= list->size()) {
        return false;
      }
      const PDXValue &val = list->get(index);
      mat[i][j] = val.get_float();
    }
  }

  return true;
}

/**
 *
 */
void PDXValue::
from_mat4(const LMatrix4 &mat) {
  PT(PDXList) list = new PDXList;
  list->append(mat[0][0]);
  list->append(mat[0][1]);
  list->append(mat[0][2]);
  list->append(mat[0][3]);
  list->append(mat[1][0]);
  list->append(mat[1][1]);
  list->append(mat[1][2]);
  list->append(mat[1][3]);
  list->append(mat[2][0]);
  list->append(mat[2][1]);
  list->append(mat[2][2]);
  list->append(mat[2][3]);
  list->append(mat[3][0]);
  list->append(mat[3][1]);
  list->append(mat[3][2]);
  list->append(mat[3][3]);
  set_list(list);
}

/**
 * Writes the value to the indicated datagram.
 */
void PDXValue::
to_datagram(Datagram &dg) const {
  dg.add_uint8(_value_type);
  switch (_value_type) {
  case VT_string:
    dg.add_string(get_string());
    break;
  case VT_int:
    dg.add_int32(get_int());
    break;
  case VT_float:
    dg.add_float32(get_float());
    break;
  case VT_boolean:
    dg.add_bool(get_bool());
    break;
  case VT_list:
    get_list()->to_datagram(dg);
    break;
  case VT_element:
    get_element()->to_datagram(dg);
    break;
  default:
    break;
  }
}

/**
 * Reads in the PDXValue from the indicated datagram.
 */
void PDXValue::
from_datagram(DatagramIterator &scan) {
  _value_type = (ValueType)scan.get_uint8();
  switch (_value_type) {
  case VT_string:
    _value = scan.get_string();
    break;
  case VT_int:
    _value = scan.get_int32();
    break;
  case VT_float:
    _value = scan.get_float32();
    break;
  case VT_boolean:
    _value = scan.get_bool();
    break;
  case VT_list: {
      PT(PDXList) list = new PDXList;
      list->from_datagram(scan);
      _value = list;
    }
    break;
  case VT_element: {
      PT(PDXElement) elem = new PDXElement;
      elem->from_datagram(scan);
      _value = elem;
    }
    break;
  }
}

/**
 * Writes the value to the indicated output stream.
 */
void PDXValue::
write(std::ostream &out, int indent_level) const {
  switch (_value_type) {
  case VT_invalid:
    indent(out, indent_level) << "null";
    break;
  case VT_string:
    indent(out, indent_level) << "\"" << std::get<std::string>(_value) << "\"";
    break;
  case VT_float:
    indent(out, indent_level) << std::get<float>(_value);
    break;
  case VT_int:
    indent(out, indent_level) << std::get<int>(_value);
    break;
  case VT_boolean:
    if (std::get<bool>(_value)) {
      indent(out, indent_level) << "true";
    } else {
      indent(out, indent_level) << "false";
    }
    break;
  case VT_list:
    std::get<PT(PDXList)>(_value)->write(out, indent_level);
    break;
  case VT_element:
    std::get<PT(PDXElement)>(_value)->write(out, indent_level);
    break;
  }
}

/**
 * Fills in the PDXValue (and all nested PDXValues) by parsing the indicated
 * tokens.
 */
bool PDXValue::
parse(TokenFile *tokens, bool get_next) {
  if (get_next) {
    if (!tokens->token_available(true)) {
      pdx_cat.error()
        << "EOF while parsing PDX value\n";
      return false;
    }

    tokens->next_token(true);
  }

  if (tokens->get_token_type() == TokenFile::TT_float) {
    set_float(tokens->get_numeric_token());

    return true;

  } else if (tokens->get_token_type() == TokenFile::TT_integer) {
    set_int(tokens->get_numeric_token());

    return true;

  } else if (tokens->get_token_type() == TokenFile::TT_string) {
    set_string(tokens->get_token());

    return true;

  } else if (tokens->get_token_type() == TokenFile::TT_word) {
    if (tokens->get_token() == "true") {
      set_bool(true);
    } else if (tokens->get_token() == "false") {
      set_bool(false);
    } else {
      set_string(tokens->get_token());
    }

    return true;

  } else if (tokens->get_token_type() == TokenFile::TT_symbol) {
    if (tokens->get_token() == "{") {
      PT(PDXElement) elem = new PDXElement;
      if (!elem->parse(tokens)) {
        pdx_cat.error()
          << "Failed to parse PDX element\n";
        return false;
      }
      set_element(elem);
      return true;

    } else if (tokens->get_token() == "[") {
      PT(PDXList) list = new PDXList;
      if (!list->parse(tokens)) {
        pdx_cat.error()
          << "Failed to parse PDX list\n";
        return false;
      }
      set_list(list);
      return true;

    } else {
      pdx_cat.error()
        << "Invalid value symbol: " << tokens->get_token() << "\n";
      return false;
    }

  } else {
    pdx_cat.error()
      << "Invalid value token: " << tokens->get_token_type() << ", " << tokens->get_token() << "\n";
    return false;
  }
}
