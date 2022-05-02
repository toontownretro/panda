/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobSystem.cxx
 * @author brian
 * @date 2022-04-30
 */

#include "jobSystem.h"
#include "mutexHolder.h"

JobSystem *JobSystem::_global_ptr = nullptr;

/**
 *
 */
JobSystem::
JobSystem() :
  _cv_mutex("jobsystem-cv-mutex"),
  _cv_work_available(_cv_mutex),
  _queue_lock("jobsystem-queue-lock"),
  _initialized(false),
  _queued_jobs(0)
{
}

/**
 *
 */
void JobSystem::
initialize() {
  if (_initialized) {
    return;
  }

  int num_workers = Thread::get_num_supported_threads() - 1;
  for (int i = 0; i < num_workers; ++i) {
    std::ostringstream ss;
    ss << "job-worker-" << i;
    PT(JobWorkerThread) thread = new JobWorkerThread(ss.str());
    thread->start(TP_urgent, true);
    _worker_threads.push_back(thread);
  }

  _initialized = true;
}
