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

#ifdef BUILDING_PANDA_SHADER
#define EXPCL_PANDA_SHADER EXPORT_CLASS
#define EXPTP_PANDA_SHADER EXPORT_TEMPL
#else
#define EXPCL_PANDA_SHADER IMPORT_CLASS
#define EXPTP_PANDA_SHADER IMPORT_TEMPL
#endif // BUILDING_PANDA_SHADER

ConfigureDecl(config_shader, EXPCL_PANDA_SHADER, EXPTP_PANDA_SHADER);
NotifyCategoryDecl(shader, EXPCL_PANDA_SHADER, EXPTP_PANDA_SHADER);

extern EXPCL_PANDA_SHADER ConfigVariableList &get_shader_libraries();

extern EXPCL_PANDA_SHADER void init_libshader();

#endif // CONFIG_SHADER_H
