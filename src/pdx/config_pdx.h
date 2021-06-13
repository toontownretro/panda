/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pdx.h
 * @author brian
 * @date 2021-06-10
 */

#ifndef CONFIG_PDX_H
#define CONFIG_PDX_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_pdx, EXPCL_PANDA_PDX, EXPTP_PANDA_PDX);
NotifyCategoryDecl(pdx, EXPCL_PANDA_PDX, EXPTP_PANDA_PDX);

extern EXPCL_PANDA_PDX void init_libpdx();

#endif // CONFIG_PDX_H
