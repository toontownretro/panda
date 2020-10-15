/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.h
 * @author lachbr
 * @date 2020-10-12
 */

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "config_shader.h"

/**
 *
 */
class EXPCL_PANDA_SHADER ShaderManager {
PUBLISHED:
  static ShaderManager *get_global_ptr();

public:
  void load_shader_libraries();

private:
  static ShaderManager *_global_ptr;
};

#endif // SHADERMANAGER_H
