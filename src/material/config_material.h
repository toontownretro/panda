/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_material.h
 * @author lachbr
 * @date 2021-03-16
 */

#ifndef CONFIG_MATERIAL_H
#define CONFIG_MATERIAL_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"

#define EXPCL_PANDA_MATERIAL
#define EXPTP_PANDA_MATERIAL

ConfigureDecl(config_material, EXPCL_PANDA_MATERIAL, EXPTP_PANDA_MATERIAL);
NotifyCategoryDecl(material, EXPCL_PANDA_MATERIAL, EXPTP_PANDA_MATERIAL);

extern EXPCL_PANDA_MATERIAL void init_libmaterial();

#endif // CONFIG_MATERIAL_H
