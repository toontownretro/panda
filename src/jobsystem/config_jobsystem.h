/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_jobsystem.h
 * @author brian
 * @date 2022-08-16
 */

#ifndef CONFIG_JOBSYSTEM_H
#define CONFIG_JOBSYSTEM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableInt.h"

ConfigureDecl(config_jobsystem, EXPCL_PANDA_JOBSYSTEM, EXPTP_PANDA_JOBSYSTEM);
NotifyCategoryDecl(jobsystem, EXPCL_PANDA_JOBSYSTEM, EXPTP_PANDA_JOBSYSTEM);

extern EXPCL_PANDA_JOBSYSTEM ConfigVariableInt job_system_num_worker_threads;

extern EXPCL_PANDA_JOBSYSTEM void init_libjobsystem();

#endif // CONFIG_JOBSYSTEM_H
