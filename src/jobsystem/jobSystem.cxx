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
#include "config_jobsystem.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "virtualFileSystem.h"

static PStatCollector parallel_proc_pcollector("JobSystem:ParallelProcess");
static PStatCollector schedule_pcollector("JobSystem:Schedule");
static PStatCollector get_job_pcollector("JobSystem:GetJob");
static PStatCollector steal_job_pcollector("JobSystem:GetJob:Steal");
static PStatCollector wait_job_pcollector("JobSystem:WaitJob");
static PStatCollector exec_job_pcollector("JobSystem:ExecuteJobWhileWaiting");

JobSystem *JobSystem::_global_ptr = nullptr;

/**
 *
 */
JobSystem::
JobSystem() :
  _queue_lock("jobsystem-queue-lock"),
  _initialized(false),
  _queued_jobs(0u)
{
  initialize();
}

/**
 *
 */
void JobSystem::
initialize() {
  if (_initialized) {
    return;
  }

  int num_workers = job_system_num_worker_threads;
  if (num_workers < 0) {
    num_workers = Thread::get_num_supported_threads() - 1;
  } else {
    num_workers = std::min(num_workers, Thread::get_num_supported_threads() - 1);
  }
  num_workers = std::max(0, num_workers);

  _job_queues = new JobQueue[num_workers + 1];
  _randomizers.push_back(Randomizer(1));
  for (int i = 0; i < num_workers; ++i) {
    std::ostringstream ss;
    ss << "job-worker-" << i;
    PT(JobWorkerThread) thread = new JobWorkerThread(ss.str(), i, this);
    thread->start(TP_normal, true);
    _randomizers.push_back(Randomizer(i + 2));
    _worker_threads.push_back(thread);
  }

  _initialized = true;
}

/**
 *
 */
void JobSystem::
new_frame() {
  // Ticks each worker thread's pstats.
  if (PStatClient::is_connected()) {
    for (JobWorkerThread *thread : _worker_threads) {
      thread->_pstats_tick_signal.clear();
    }
  }
}

/**
 *
 */
void JobSystem::
schedule(Job *job) {
  PStatTimer timer(schedule_pcollector);

  Thread *thread = Thread::get_current_thread();
  int queue_index = (thread->get_type() == JobWorkerThread::get_class_type()) ? DCAST(JobWorkerThread, thread)->_thread_index + 1 : 0;

#ifdef THREADED_PIPELINE
  job->set_pipeline_stage(thread->get_pipeline_stage());
#endif

  if (!_worker_threads.empty()) {

    job->ref();
    job->set_state(Job::S_queued);

    //push_event(JobSystemEvent::ET_schedule_job);

    _job_queues[queue_index].push(job);

    _queued_jobs.fetch_add(1u);
    _queued_jobs.notify_one();

  } else {
    // No worker threads, execute job right now on this thread.
    job->set_state(Job::S_working);
    job->execute();
    job->set_state(Job::S_complete);
  }
}

/**
 * Schedules several jobs at the same time.  A bit more efficient than
 * calling schedule() for each job.
 */
void JobSystem::
schedule(Job **jobs, int count, bool wait) {
  PStatTimer timer(schedule_pcollector);

  if (!_worker_threads.empty()) {
    Thread *thread = Thread::get_current_thread();
    int queue_index = (thread->get_type() == JobWorkerThread::get_class_type()) ? DCAST(JobWorkerThread, thread)->_thread_index + 1 : 0;

    for (int i = 0; i < count; ++i) {
#ifdef THREADED_PIPELINE
      jobs[i]->set_pipeline_stage(thread->get_pipeline_stage());
#endif
      jobs[i]->ref();
      jobs[i]->set_state(Job::S_queued);

      _job_queues[queue_index].push(jobs[i]);

      //push_event(JobSystemEvent::ET_schedule_job);
    }

    _queued_jobs.fetch_add(count);
    _queued_jobs.notify_all();

    if (wait) {
      for (int i = 0; i < count; ++i) {
        wait_job(jobs[i], thread);
      }
    }

  } else {
    for (int i = 0; i < count; ++i) {
      jobs[i]->set_state(Job::S_working);
      jobs[i]->execute();
      jobs[i]->set_state(Job::S_complete);
    }
  }
}

/**
 *
 */
void JobSystem::
parallel_process(int count, std::function<void(int)> func, int count_threshold, bool job_per_item) {
  PStatTimer timer(parallel_proc_pcollector);

  if (count == 0) {
    return;

  } else if (count == 1) {
    func(0);
    return;

  } else if (count < count_threshold || _worker_threads.empty()) {
    // No worker threads or not enough items to prohibit scheduling jobs.
    for (int i = 0; i < count; ++i) {
      func(i);
    }
    return;
  }

  ParallelProcessJob job;
  job.local_object();
  job._first_item = 0;
  job._num_items = count;
  job._function = func;
  schedule(&job);
  wait_job(&job);
}

/**
 * Blocks until the indicated job executes to completion.
 *
 * While waiting, this thread will attempt to service other jobs
 * in the queue.
 */
void JobSystem::
wait_job(Job *job, Thread *thread) {
  PStatTimer timer(wait_job_pcollector);

  if (job->get_state() == Job::S_fresh) {
    return;
  }

#ifdef THREADED_PIPELINE
  int orig_pipeline_stage = thread->get_pipeline_stage();
#endif

  bool is_worker = (thread->get_type() == JobWorkerThread::get_class_type());

  while (job->get_state() != Job::S_complete) {
    //Thread::relax();

#if 1
    Job *job2 = pop_job(thread, is_worker);
    if (job2 != nullptr) {
      exec_job_pcollector.start();

#ifdef THREADED_PIPELINE
      thread->set_pipeline_stage(job2->get_pipeline_stage());
#endif

      //push_event(JobSystemEvent::ET_start_job);

      job2->set_state(Job::S_working);
      job2->execute();
      if (job2->unref()) {
        job2->set_state(Job::S_complete);
      } else {
        delete job;
      }

      //push_event(JobSystemEvent::ET_finish_job);

      exec_job_pcollector.stop();
    }
#endif
  }

#ifdef THREADED_PIPELINE
  thread->set_pipeline_stage(orig_pipeline_stage);
#endif
}

/**
 *
 */
void JobSystem::
write_events(const Filename &filename) {
  MutexHolder holder(_event_lock);

  pofstream stream;
  Filename fname = filename;
  fname.set_text();
  if (fname.open_append(stream)) {
    for (JobSystemEvent *event = _events; event != nullptr; event = event->next) {
      stream << event->thread_name << " " << event->time << " " << event->type << "\n";
    }
    stream.flush();
    stream.close();
  } else {
    jobsystem_cat.warning()
      << "Could not open " << fname << " for append/write.\n";
  }

  // Clear out the events.
  for (JobSystemEvent *event = _events; event != nullptr;) {
    JobSystemEvent *save_event = event;
    event = save_event->next;
    delete save_event;
  }
}
