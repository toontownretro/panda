/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_bsp.h
 * @author lachbr
 * @date 2020-12-30
 */

#ifndef CONFIG_BSP_H
#define CONFIG_BSP_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"

ConfigureDecl(config_bsp, EXPCL_PANDA_BSP, EXPTP_PANDA_BSP);
NotifyCategoryDecl(bsp, EXPCL_PANDA_BSP, EXPTP_PANDA_BSP);

extern EXPCL_PANDA_BSP void init_libbsp();

#endif // CONFIG_BSP_H
