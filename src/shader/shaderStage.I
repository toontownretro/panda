/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.h
 * @author lachbr
 * @date 2020-10-18
 */

/**
 *
 */
INLINE ShaderStage::
ShaderStage() {
  _has_defines_str = false;
}

/**
 *
 */
INLINE void ShaderStage::
operator =(const ShaderStage &copy) {
  _source = copy._source;
  _defines_str = copy._defines_str;
  _has_defines_str = copy._has_defines_str;
}

/**
 * Resets the ShaderStage to a clean slate, ready for the shader that needs to
 * be generated.
 */
INLINE void ShaderStage::
reset() {
  _source = nullptr;
  _defines = std::ostringstream();
  _has_defines_str = false;
}

/**
 * Sets the filename of the source code for this shader stage.
 */
INLINE void ShaderStage::
set_source_filename(const Filename &filename) {
  _source = ShaderSource::from_filename(filename);
  _defines = std::ostringstream();
  _has_defines_str = false;
}

/**
 * Sets the raw source code for this shader stage.
 */
INLINE void ShaderStage::
set_source_raw(const std::string &source) {
  _source = ShaderSource::from_raw(source);
}

/**
 * Returns the final source code for this stage that will be compiled into the
 * shader program.  If the source was loaded from disk, returns the source with
 * the #defines inserted.
 */
INLINE std::string ShaderStage::
get_final_source() {
  if (!_source) {
    return "";
  }

  switch(_source->get_format()) {
  case ShaderSource::SF_raw:
    return _source->get_source();

  case ShaderSource::SF_file:
  {
    std::ostringstream ss;
    ss << _source->get_before_defines() << "\n";
    ss << get_defines_str();
    ss << _source->get_after_defines();
    return ss.str();
  }

  default:
    return "";
  }
}

/**
 * Sets a #define for the shader stage.
 */
INLINE void ShaderStage::
set_define(const std::string &name, const std::string &value) {
  set_define<std::string>(name, value);
}

/**
 * Returns a string representing the #defines that should be inserted into the
 * shader code.
 */
INLINE const std::string &ShaderStage::
get_defines_str() const {
  ShaderStage *nc_this = (ShaderStage *)this;
  if (!_has_defines_str) {
    nc_this->_defines_str = _defines.str();
    nc_this->_has_defines_str = true;
  }

  return _defines_str;
}

/**
 * Adds the hash of the shader stage onto the running hash.
 */
INLINE size_t ShaderStage::
add_hash(size_t hash) const {
  hash = pointer_hash::add_hash(hash, _source.p());
  hash = string_hash::add_hash(hash, get_defines_str());
  return hash;
}

/**
 *
 */
INLINE bool ShaderStage::
operator < (const ShaderStage &other) const {
  if (_source != other._source) {
    return (_source < other._source);
  }

  if (get_defines_str() != other.get_defines_str()) {
    return (get_defines_str() < other.get_defines_str());
  }

  return false;
}

/**
 *
 */
INLINE bool ShaderStage::
operator == (const ShaderStage &other) const {
  if (_source != other._source) {
    return false;
  }

  if (get_defines_str() != other.get_defines_str()) {
    return false;
  }

  return true;
}
