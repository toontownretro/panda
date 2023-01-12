/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_map.h
 * @author brian
 * @date 2020-12-18
 */

#ifndef CONFIG_MAP_H
#define CONFIG_MAP_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

NotifyCategoryDecl(map, EXPCL_PANDA_MAP, EXPTP_PANDA_MAP);
ConfigureDecl(config_map, EXPCL_PANDA_MAP, EXPTP_PANDA_MAP);

extern EXPCL_PANDA_MAP void init_libmap();

#endif // CONFIG_MAP_H
