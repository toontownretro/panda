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
#include "mutexHolder.h"

IMPLEMENT_CLASS(JobWorkerThread);

/**
 *
 */
JobWorkerThread::
JobWorkerThread(const std::string &name) :
  Thread(name, name),
  _current_job(nullptr)
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
      sys->_cv_mutex.acquire();
      sys->_cv_work_available.wait();
      sys->_cv_mutex.release();
      //sys->wait_for_work();
      //Thread::force_yield();
      //Thread::relax();
    } else {
      // Operate on the pipeline stage of the thread that scheduled this
      // job.
      set_pipeline_stage(job->get_pipeline_stage());
      _current_job = job;
      job->set_state(Job::S_working);
      job->execute();
      sys->job_finished();
      job->set_state(Job::S_complete);
      _current_job = nullptr;
    }
  }
}
