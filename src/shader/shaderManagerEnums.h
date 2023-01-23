/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManagerEnums.h
 * @author brian
 * @date 2020-11-02
 */

#ifndef SHADERMANAGERENUMS_H
#define SHADERMANAGERENUMS_H

#include "pandabase.h"

/**
 * Various enumerations relating to shader things.
 */
class ShaderManagerEnums {
PUBLISHED:
  enum ShaderQuality {
    SQ_low,
    SQ_medium,
    SQ_high,
  };
};

EXPCL_PANDA_SHADER std::ostream &operator << (std::ostream &out, ShaderManagerEnums::ShaderQuality sm);
EXPCL_PANDA_SHADER std::istream &operator >> (std::istream &in, ShaderManagerEnums::ShaderQuality &sm);

#endif // SHADERMANAGERENUMS_H
