/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_glesgsg.h
 * @author rdb
 * @date 2009-05-21
 */

#ifndef CONFIG_GLESGSG_H
#define CONFIG_GLESGSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_glesgsg, EXPCL_PANDA_GLESGSG, EXPTP_PANDA_GLESGSG);
NotifyCategoryDecl(glesgsg, EXPCL_PANDA_GLESGSG, EXPTP_PANDA_GLESGSG);

extern EXPCL_PANDA_GLESGSG void init_libglesgsg();

#endif
