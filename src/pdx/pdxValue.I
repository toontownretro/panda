/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxValue.I
 * @author brian
 * @date 2021-06-10
 */

/**
 *
 */
INLINE std::string PDXValue::
get_string() const {
  switch (_value_type) {
  case VT_string:
    return std::get<std::string>(_value);

  // We can also convert these types to strings on the fly.
  case VT_int: {
    std::ostringstream ss;
    ss << std::get<int>(_value);
    return ss.str();
  }
  case VT_float: {
    std::ostringstream ss;
    ss << std::get<float>(_value);
    return ss.str();
  }
  case VT_boolean: {
    if (std::get<bool>(_value)) {
      return "true";
    } else {
      return "false";
    }
  }
  default:
#ifndef NDEBUG
    nassert_raise("PDXValue::get_string(): Value type is not string, int, float, or boolean");
#endif
    return "";
  }
}

/**
 *
 */
INLINE bool PDXValue::
is_string() const {
  return _value_type == VT_string;
}

/**
 *
 */
INLINE bool PDXValue::
is_bool() const {
  return _value_type == VT_boolean;
}

/**
 *
 */
INLINE float PDXValue::
get_float() const {
  // We can accept float, int, or boolean.
  switch (_value_type) {
  case VT_float:
    return std::get<float>(_value);
  case VT_int:
    return std::get<int>(_value);
  case VT_boolean:
    return std::get<bool>(_value) ? 1.0f : 0.0f;
  default:
#ifndef NDEBUG
    nassert_raise("PDXValue::get_float(): Value type is not int, float, or boolean");
#endif
    return 0.0f;
  }
}

/**
 *
 */
INLINE bool PDXValue::
is_float() const {
  return _value_type == VT_float;
}

/**
 *
 */
INLINE int PDXValue::
get_int() const {
  // We can accept int, float, or boolean.
  switch (_value_type) {
  case VT_int:
    return std::get<int>(_value);
  case VT_float:
    return std::get<float>(_value);
  case VT_boolean:
    return std::get<bool>(_value) ? 1 : 0;
  default:
#ifndef NDEBUG
    nassert_raise("PDXValue::get_int(): Value type is not int, float, or boolean");
#endif
    return 0;
  }
}

/**
 *
 */
INLINE bool PDXValue::
is_int() const {
  return _value_type == VT_int;
}

/**
 *
 */
INLINE PDXElement *PDXValue::
get_element() const {
  // Elements and lists must be the exact type.
  nassertr(_value_type == VT_element, nullptr);
  return std::get<PT(PDXElement)>(_value);
}

/**
 *
 */
INLINE bool PDXValue::
is_element() const {
  return _value_type == VT_element;
}

/**
 *
 */
INLINE PDXList *PDXValue::
get_list() const {
  // Elements and lists must be the exact type.
  nassertr(_value_type == VT_list, nullptr);
  return std::get<PT(PDXList)>(_value);
}

/**
 *
 */
INLINE bool PDXValue::
is_list() const {
  return _value_type == VT_list;
}

/**
 *
 */
INLINE PDXValue::ValueType PDXValue::
get_value_type() const {
  return _value_type;
}
