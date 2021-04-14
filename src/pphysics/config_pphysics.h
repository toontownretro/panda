/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pphysics.h
 * @author lachbr
 * @date 2021-04-13
 */

#ifndef CONFIG_PPHYSICS_H
#define CONFIG_PPHYSICS_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableString.h"

ConfigureDecl(config_pphysics, EXPCL_PANDA_PPHYSICS, EXPTP_PANDA_PPHYSICS);
NotifyCategoryDecl(pphysics, EXPCL_PANDA_PPHYSICS, EXPTP_PANDA_PPHYSICS);

extern EXPCL_PANDA_PPHYSICS ConfigVariableBool phys_enable_pvd;
extern EXPCL_PANDA_PPHYSICS ConfigVariableString phys_pvd_host;
extern EXPCL_PANDA_PPHYSICS ConfigVariableInt phys_pvd_port;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_tolerance_scale;
extern EXPCL_PANDA_PPHYSICS ConfigVariableBool phys_track_allocations;

extern EXPCL_PANDA_PPHYSICS void init_libpphysics();

#endif // CONFIG_PPHYSICS_H
