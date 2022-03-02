/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dxgsg11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef CONFIG_DXGSG11_H
#define CONFIG_DXGSG11_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"

ConfigureDecl(config_dxgsg11, EXPCL_PANDA_DXGSG11, EXPTP_PANDA_DXGSG11);
NotifyCategoryDecl(dxgsg11, EXPCL_PANDA_DXGSG11, EXPTP_PANDA_DXGSG11);

extern EXPCL_PANDA_DXGSG11 void init_libdxgsg11();

#endif // CONFIG_DXGSG11_H
