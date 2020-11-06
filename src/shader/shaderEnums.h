/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderEnums.h
 * @author lachbr
 * @date 2020-11-02
 */

#ifndef SHADERENUMS_H
#define SHADERENUMS_H

#include "pandabase.h"

/**
 * Various enumerations relating to shader things.
 */
class ShaderEnums {
PUBLISHED:
  enum ShaderQuality {
    SQ_low,
    SQ_medium,
    SQ_high,
  };
};

EXPCL_PANDA_SHADER std::ostream &operator << (std::ostream &out, ShaderEnums::ShaderQuality sm);
EXPCL_PANDA_SHADER std::istream &operator >> (std::istream &in, ShaderEnums::ShaderQuality &sm);

#endif // SHADERENUMS_H
