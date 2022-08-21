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
#include "pStatClient.h"
#include "pStatTimer.h"
#include "pStatCollector.h"

PStatCollector exec_job_pcollector("JobSystem:ExecuteJob");
static PStatCollector sleep_pcollector("JobSystem:Sleep");

IMPLEMENT_CLASS(JobWorkerThread);

/**
 *
 */
JobWorkerThread::
JobWorkerThread(const std::string &name) :
  Thread(name, name),
  _current_job(nullptr),
  _state(S_idle)
{
}

/**
 *
 */
void JobWorkerThread::
thread_main() {
  JobSystem *sys = JobSystem::get_global_ptr();

  while (true) {
    PStatClient::thread_tick();

    Job *job = sys->pop_job(this);
    if (job == nullptr) {
      PStatTimer timer(sleep_pcollector);

      sys->_cv_mutex.acquire();
      while (AtomicAdjust::get(sys->_queued_jobs) == 0) {
        sys->_cv_work_available.wait();
      }
      sys->_cv_mutex.release();

    } else {
      PStatTimer timer(exec_job_pcollector);

      AtomicAdjust::set(_state, S_busy);

      // Operate on the pipeline stage of the thread that scheduled this
      // job.
      set_pipeline_stage(job->get_pipeline_stage());

      _current_job = job;

      job->set_state(Job::S_working);
      job->execute();
      if (job->unref()) {
        job->set_state(Job::S_complete);
      } else {
        delete job;
      }

      _current_job = nullptr;

      AtomicAdjust::set(_state, S_idle);
    }
  }
}
