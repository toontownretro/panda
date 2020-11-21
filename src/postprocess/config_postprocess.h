/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_postprocess.h
 * @author lachbr
 * @date 2020-09-20
 */

#ifndef CONFIG_POSTPROCESS_H
#define CONFIG_POSTPROCESS_H

#include "dconfig.h"
#include "pandabase.h"
#include "notifyCategoryProxy.h"

ConfigureDecl(config_postprocess, EXPCL_PANDA_POSTPROCESS, EXPTP_PANDA_POSTPROCESS);

NotifyCategoryDecl(postprocess, EXPCL_PANDA_POSTPROCESS, EXPTP_PANDA_POSTPROCESS);

extern EXPCL_PANDA_POSTPROCESS void init_libpostprocess();

#endif // CONFIG_POSTPROCESS_H
