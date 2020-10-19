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

#ifndef SHADERSTAGE_H
#define SHADERSTAGE_H

#include "config_shader.h"
#include "filename.h"
#include "shaderSource.h"

#include <string>

/**
 * This class represents a single stage of a generated shader program.  It
 * contains a set of #defines that will be inserted into the associated
 * source when creating the shader program.
 */
class EXPCL_PANDA_SHADER ShaderStage {
public:
  ShaderStage() = default;

  INLINE void reset();

  INLINE void set_source_filename(const Filename &filename);
  INLINE void set_source_raw(const std::string &source);

  INLINE std::string get_final_source() const;

  template <class T>
  INLINE void set_define(const std::string &name, const T &value);
  INLINE void set_define(const std::string &name, const std::string &value = "1");

private:
  CPT(ShaderSource) _source;

  std::ostringstream _defines;
};

#include "shaderStage.I"
#include "shaderStage.T"

#endif // SHADERSTAGE_H
