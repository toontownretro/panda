/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_jobsystem.cxx
 * @author brian
 * @date 2022-08-16
 */

#include "config_jobsystem.h"
#include "job.h"
#include "jobWorkerThread.h"

ConfigureDef(config_jobsystem);
ConfigureFn(config_jobsystem) {
  init_libjobsystem();
}

NotifyCategoryDef(jobsystem, "");

ConfigVariableInt job_system_num_worker_threads
("job-system-num-worker-threads", "-1",
 PRC_DESC("Specifies the number of worker threads the job system should create. "
          "Max is number of hardware threads - 1, specify -1 to use that number."));

/**
 *
 */
void
init_libjobsystem() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  JobWorkerThread::init_type();
  Job::init_type();
  ParallelProcessJob::init_type();
}
