/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramAttrib.I
 * @author lachbr
 * @date 2020-10-15
 */

/**
 *
 */
INLINE ParamAttrib::
ParamAttrib() {
}

/**
 *
 */
INLINE ParamAttrib::
ParamAttrib(const ParamAttrib &other) {
  _params = other._params;
}

/**
 * Creates a new empty ParamAttrib.
 */
INLINE CPT(RenderAttrib) ParamAttrib::
make() {
  PT(ParamAttrib) attr = new ParamAttrib;

  return return_new(attr);
}

/**
 * Returns a new ParamAttrib with the new parameter set, but everything
 * else the same.
 */
INLINE CPT(RenderAttrib) ParamAttrib::
set_param(const std::string &key, const std::string &value) const {
  PT(ParamAttrib) attr = new ParamAttrib(*this);
  attr->_params[downcase(key)] = value;
  return return_new(attr);
}

/**
 * Returns the number of key-value parameters.
 */
INLINE int ParamAttrib::
get_num_params() const {
  return (int)_params.size();
}

/**
 * Returns the index of the given parameter key, or -1 if it doesn't exist.
 */
INLINE int ParamAttrib::
find_param(const std::string &key) const {
  return _params.find(key);
}

/**
 * Returns true if the given parameter key exists, or false if it does not.
 */
INLINE bool ParamAttrib::
has_param(const std::string &key) const {
  return find_param(key) != -1;
}

/**
 * Returns the parameter key at the given index.
 */
INLINE const std::string &ParamAttrib::
get_param_key(int n) const {
  return _params.get_key(n);
}

/**
 * Returns the parameter value at the given index.
 */
INLINE const std::string &ParamAttrib::
get_param_value(int n) const {
  return _params.get_data(n);
}

/**
 * Returns the parameter value at the given index as a boolean.
 */
INLINE bool ParamAttrib::
get_param_value_bool(int n) const {
  return (bool)get_param_value_int(n);
}

/**
 * Returns the parameter value at the given index as an integer.
 */
INLINE int ParamAttrib::
get_param_value_int(int n) const {
  return atoi(get_param_value(n).c_str());
}

/**
 * Returns the parameter value at the given index as a float.
 */
INLINE float ParamAttrib::
get_param_value_float(int n) const {
  return atof(get_param_value(n).c_str());
}

/**
 * Returns the parameter value at the given index as a 2-component vector.
 */
INLINE LVecBase2f ParamAttrib::
get_param_value_2f(int n) const {
  return KeyValues::to_2f(get_param_value(n));
}

/**
 * Returns the parameter value at the given index as a 3-component vector.
 */
INLINE LVecBase3f ParamAttrib::
get_param_value_3f(int n) const {
  return KeyValues::to_3f(get_param_value(n));
}

/**
 * Returns the parameter value at the given index as a 4-component vector.
 */
INLINE LVecBase4f ParamAttrib::
get_param_value_4f(int n) const {
  return KeyValues::to_4f(get_param_value(n));
}
