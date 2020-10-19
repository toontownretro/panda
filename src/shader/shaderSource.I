/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderSource.I
 * @author lachbr
 * @date 2020-10-18
 */

/**
 *
 */
INLINE ShaderSource::
ShaderSource() {
  _format = SF_unknown;
}

/**
 * Returns the full raw source of the shader.
 */
INLINE const std::string &ShaderSource::
get_source() const {
  return _source;
}

/**
 * Returns the source of the shader before the location where #defines are to
 * be inserted.
 */
INLINE const std::string &ShaderSource::
get_before_defines() const {
  return _before_defines;
}

/**
 * Returns the source of the shader after the location where #defines are to
 * be inserted.
 */
INLINE const std::string &ShaderSource::
get_after_defines() const {
  return _after_defines;
}

/**
 * Returns the format of the shader source (raw or from disk).
 */
INLINE ShaderSource::SourceFormat ShaderSource::
get_format() const {
  return _format;
}

/**
 * Returns a new ShaderSource object from the raw source code.
 */
INLINE CPT(ShaderSource) ShaderSource::
from_raw(const std::string &source) {
  PT(ShaderSource) src = new ShaderSource;
  src->_source = source;
  src->_format = SF_raw;
  return src;
}
