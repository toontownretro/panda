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
 * Resets the ShaderStage to a clean slate, ready for the shader that needs to
 * be generated.
 */
INLINE void ShaderStage::
reset() {
  _source = nullptr;
  _defines = std::ostringstream();
}

/**
 * Sets the filename of the source code for this shader stage.
 */
INLINE void ShaderStage::
set_source_filename(const Filename &filename) {
  _source = ShaderSource::from_filename(filename);
  _defines = std::ostringstream();
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
get_final_source() const {
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
    ss << _defines.str();
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
