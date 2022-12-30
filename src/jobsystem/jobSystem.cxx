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

static PStatCollector parallel_proc_pcollector("JobSystem:ParallelProcess");
static PStatCollector schedule_pcollector("JobSystem::Schedule");
static PStatCollector get_job_pcollector("JobSystem:GetJob");
static PStatCollector steal_job_pcollector("JobSystem:GetJob:Steal");
static PStatCollector wait_job_pcollector("JobSystem:WaitJob");
extern PStatCollector exec_job_pcollector;

thread_local int js_steal_idx = 0;

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

  for (int i = 0; i < num_workers; ++i) {
    std::ostringstream ss;
    ss << "job-worker-" << i;
    PT(JobWorkerThread) thread = new JobWorkerThread(ss.str());
    thread->start(TP_normal, true);
    _worker_threads.push_back(thread);
  }

  _initialized = true;
}

/**
 *
 */
void JobSystem::
schedule(Job *job) {
  PStatTimer timer(schedule_pcollector);

  Thread *thread = Thread::get_current_thread();

#ifdef THREADED_PIPELINE
  job->set_pipeline_stage(thread->get_pipeline_stage());
#endif

  if (!_worker_threads.empty()) {

    job->ref();
    job->set_state(Job::S_queued);

    AtomicAdjust::inc(_queued_jobs);

    if (thread->get_type() == JobWorkerThread::get_class_type()) {
      JobWorkerThread *jthread = DCAST(JobWorkerThread, thread);
      jthread->_local_queue.push(job);
    } else {
      _queue_lock.acquire();
      _job_queue.push(job);
      _queue_lock.release();
    }

    _cv_work_available.notify();

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

    JobWorkerThread *jthread = nullptr;
    if (thread->get_type() == JobWorkerThread::get_class_type()) {
      jthread = DCAST(JobWorkerThread, thread);
    }

    for (int i = 0; i < count; ++i) {
#ifdef THREADED_PIPELINE
      jobs[i]->set_pipeline_stage(thread->get_pipeline_stage());
#endif
      jobs[i]->ref();
      jobs[i]->set_state(Job::S_queued);
    }

    AtomicAdjust::add(_queued_jobs, count);

    if (jthread != nullptr) {
      for (int i = 0; i < count; ++i) {
        jthread->_local_queue.push(jobs[i]);
      }
    } else {
      _queue_lock.acquire();
      for (int i = 0; i < count; ++i) {
        _job_queue.push(jobs[i]);
      }
      _queue_lock.release();
    }

    _cv_work_available.notify_all();

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

  Thread *thread = Thread::get_current_thread();
#ifdef THREADED_PIPELINE
  int pipeline_stage = thread->get_pipeline_stage();
#endif

  // + 1 because the main thread chips in while waiting.
  int thread_count = (int)_worker_threads.size() + 1;

  int num_per_thread = count / thread_count;
  int remainder = count % thread_count;

  int job_count = job_per_item ? count : std::min(count, thread_count);

  pvector<ParallelProcessJob> jobs;
  jobs.resize(job_count);
  if (!job_per_item && num_per_thread > 0) {
    for (size_t i = 0; i < thread_count; ++i) {
      int first = num_per_thread * i;
      int count = num_per_thread;
      if (i == (thread_count - 1)) {
        count += remainder;
      }
      jobs[i].local_object();
      jobs[i].ref();
#ifdef THREADED_PIPELINE
      jobs[i].set_pipeline_stage(pipeline_stage);
#endif
      jobs[i].set_state(Job::S_queued);
      jobs[i]._first_item = first;
      jobs[i]._num_items = count;
      jobs[i]._function = func;
    }
  } else {
    // If there are fewer items than worker threads, create a job for
    // each item.
    for (int i = 0; i < count; ++i) {
      jobs[i].local_object();
      jobs[i].ref();
#ifdef THREADED_PIPELINE
      jobs[i].set_pipeline_stage(pipeline_stage);
#endif
      jobs[i].set_state(Job::S_queued);
      jobs[i]._first_item = i;
      jobs[i]._num_items = 1;
      jobs[i]._function = func;
    }
  }

  AtomicAdjust::add(_queued_jobs, job_count);

  if (thread->get_type() == JobWorkerThread::get_class_type()) {
    JobWorkerThread *jthread = DCAST(JobWorkerThread, thread);
    for (int i = 0; i < job_count; ++i) {
      jthread->_local_queue.push(&jobs[i]);
    }
  } else {
    _queue_lock.acquire();
    for (int i = 0; i < job_count; ++i) {
      _job_queue.push(&jobs[i]);
    }
    _queue_lock.release();
  }

  _cv_work_available.notify_all();

  for (int i = 0; i < job_count; ++i) {
    wait_job(&jobs[i], thread);
  }
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

      job2->set_state(Job::S_working);
      job2->execute();
      if (job2->unref()) {
        job2->set_state(Job::S_complete);
      } else {
        delete job;
      }

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
Job *JobSystem::
get_job_for_thread(Thread *thread, bool is_worker) {
  //PStatTimer timer(get_job_pcollector);

  JobWorkerThread *jthread = nullptr;
  if (is_worker) {
    jthread = DCAST(JobWorkerThread, thread);
  }

  if (jthread != nullptr) {
    // This is a worker thread.  Try to pop a job from its local
    // queue.
    if (!jthread->_local_queue.empty()) {
      std::optional<Job *> item = jthread->_local_queue.pop();
      if (item.has_value()) {
        return item.value();
      }
    }

  } else if (!_job_queue.empty()) {
    // A non-worker thread (like App or Cull).  Attempt to steal
    // from the "non-worker" queue.
    std::optional<Job *> item = _job_queue.steal();
    if (item.has_value()) {
      return item.value();
    }
  }

  // We weren't able to get a job from the thread's local queue.
  // Attempt to steal from other busy worker threads.

  {
    //PStatTimer timer2(steal_job_pcollector);

    // For worker threads, attempt to steal from the non-worker queue first.
    // This is redundant for non-worker threads.
    if (jthread != nullptr) {
      if (!_job_queue.empty()) {
        std::optional<Job *> item = _job_queue.steal();
        if (item.has_value()) {
          return item.value();
        }
      }
    }

    // Finally, try to steal from worker threads that are busy with another
    // job with outstanding work in their local queues.

    if (!_worker_threads.empty()) {
      static thread_local Randomizer random(Thread::get_current_thread_id());
      int index = random.random_int((int)_worker_threads.size());
      JobWorkerThread *othread = _worker_threads[index];
      if (othread != thread && !othread->_local_queue.empty()) {
        std::optional<Job *> item = othread->_local_queue.steal();
        if (item.has_value()) {
          return item.value();
        }
      }
    }
  }

  return nullptr;
}
