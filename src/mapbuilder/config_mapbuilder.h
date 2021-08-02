/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_mapbuilder.h
 * @author brian
 * @date 2021-07-05
 */

#ifndef CONFIG_MAPBUILDER_H
#define CONFIG_MAPBUILDER_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"

ConfigureDecl(config_mapbuilder, EXPCL_PANDA_MAPBUILDER, EXPTP_PANDA_MAPBUILDER);

NotifyCategoryDecl(mapbuilder, EXPCL_PANDA_MAPBUILDER, EXPTP_PANDA_MAPBUILDER);

extern EXPCL_PANDA_MAPBUILDER void init_libmapbuilder();

#endif // CONFIG_MAPBUILDER_H
