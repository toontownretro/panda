/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_particlesystem2.h
 * @author brian
 * @date 2022-04-02
 */

#ifndef CONFIG_PARTICLESYSTEM2_H
#define CONFIG_PARTICLESYSTEM2_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"

ConfigureDecl(config_particlesystem2, EXPCL_PANDA_PARTICLESYSTEM2, EXPTP_PANDA_PARTICLESYSTEM2);
NotifyCategoryDecl(particlesystem2, EXPCL_PANDA_PARTICLESYSTEM2, EXPTP_PANDA_PARTICLESYSTEM2);

extern EXPCL_PANDA_PARTICLESYSTEM2 void init_libparticlesystem2();

#endif // CONFIG_PARTICLESYSTEM2_H
