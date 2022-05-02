/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobWorkerThread.cxx
 * @author brian
 * @date 2022-04-30
 */

#include "jobWorkerThread.h"
#include "jobSystem.h"

IMPLEMENT_CLASS(JobWorkerThread);

/**
 *
 */
JobWorkerThread::
JobWorkerThread(const std::string &name) :
  Thread(name, name)
{
}

/**
 *
 */
void JobWorkerThread::
thread_main() {
  JobSystem *sys = JobSystem::get_global_ptr();
  while (true) {
    //PStatClient::thread_tick();

    PT(Job) job;
    sys->pop_job(job);
    if (job == nullptr) {
      sys->wait_for_work();
      //Thread::force_yield();
      //Thread::relax();
    } else {
      job->execute();
      sys->job_finished();
    }
  }
}
