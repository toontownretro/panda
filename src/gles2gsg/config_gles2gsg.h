/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_gles2gsg.h
 * @author rdb
 * @date 2009-06-14
 */

#ifndef CONFIG_GLES2GSG_H
#define CONFIG_GLES2GSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_gles2gsg, EXPCL_PANDA_GLES2GSG, EXPTP_PANDA_GLES2GSG);
NotifyCategoryDecl(gles2gsg, EXPCL_PANDA_GLES2GSG, EXPTP_PANDA_GLES2GSG);

extern EXPCL_PANDA_GLES2GSG void init_libgles2gsg();

#endif
