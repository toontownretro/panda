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
#include "trueClock.h"

PStatCollector exec_job_pcollector("JobSystem:ExecuteJob");
static PStatCollector sleep_pcollector("JobSystem:Sleep");

IMPLEMENT_CLASS(JobWorkerThread);

/**
 *
 */
JobWorkerThread::
JobWorkerThread(const std::string &name, int index) :
  Thread(name, name),
  _thread_index(index),
  _current_job(nullptr),
  _state(S_idle),
  _pstats_tick_signal(false)
{
}

/**
 *
 */
void JobWorkerThread::
thread_main() {
  JobSystem *sys = JobSystem::get_global_ptr();

  while (true) {

    if (!_pstats_tick_signal.test_and_set()) {
      PStatClient::thread_tick();
    }

    Job *job = sys->pop_job(this, true);
    if (job != nullptr) {

      PStatTimer timer(exec_job_pcollector);

      AtomicAdjust::set(_state, S_busy);

#ifdef THREADED_PIPELINE
      // Operate on the pipeline stage of the thread that scheduled this
      // job.
      set_pipeline_stage(job->get_pipeline_stage());
#endif

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

    } else {
      sys->_queued_jobs.wait(0u);
    }
  }
}
