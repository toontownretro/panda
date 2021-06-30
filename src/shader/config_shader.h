/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_shader.h
 * @author lachbr
 * @date 2020-10-12
 */

#ifndef CONFIG_SHADER_H
#define CONFIG_SHADER_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"
#include "configVariableList.h"
#include "configVariableEnum.h"
#include "configVariableBool.h"
#include "configVariableFilename.h"
#include "shaderEnums.h"

ConfigureDecl(config_shader, EXPCL_PANDA_SHADER, EXPTP_PANDA_SHADER);
NotifyCategoryDecl(shadermgr, EXPCL_PANDA_SHADER, EXPTP_PANDA_SHADER);

extern EXPCL_PANDA_SHADER ConfigVariableList &get_shader_libraries();

extern EXPCL_PANDA_SHADER ConfigVariableEnum<ShaderEnums::ShaderQuality> &
config_get_shader_quality();

extern EXPCL_PANDA_SHADER ConfigVariableBool &config_get_use_vertex_lit_for_no_material();

extern EXPCL_PANDA_SHADER ConfigVariableFilename default_cube_map;

extern EXPCL_PANDA_SHADER void init_libshader();

#endif // CONFIG_SHADER_H
