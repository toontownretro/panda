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
#include "configVariableBool.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "virtualFileSystem.h"

static PStatCollector parallel_proc_pcollector("JobSystem:ParallelProcess");
static PStatCollector parallel_proc_per_item_pcollector("JobSystem:ParallelProcessPerItem");
static PStatCollector schedule_pcollector("JobSystem:Schedule");
static PStatCollector get_job_pcollector("JobSystem:GetJob");
static PStatCollector steal_job_pcollector("JobSystem:GetJob:Steal");
static PStatCollector wait_job_pcollector("JobSystem:WaitJob");
static PStatCollector exec_job_pcollector("JobSystem:ExecuteJobWhileWaiting");

JobSystem *JobSystem::_global_ptr = nullptr;

ConfigVariableBool js_workers_only
("job-system-workers-only", true,
 PRC_DESC("When enabled, Only job worker threads will take "
          "jobs, Otherwise all threads will try and contribute "
          "to the work."));

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
JobSystem::
~JobSystem() 
{
  if (!_initialized) { return; }
  
  delete[] _job_queues;
  delete[] _randomizers;
  delete[] _worker_threads;
}

/**
 *
 */
void JobSystem::
initialize() {
  if (_initialized) {
    return;
  }

  _num_workers = job_system_num_worker_threads;
  if (_num_workers < 0) {
    _num_workers = Thread::get_num_supported_threads() - 1;
  } else {
    _num_workers = std::min(_num_workers, Thread::get_num_supported_threads() - 1);
  }
  _num_workers = std::max(0, _num_workers);
  
  _worker_threads = new PT(JobWorkerThread)[_num_workers + 1]();
  _worker_threads[_num_workers] = nullptr;
  
  _randomizers = new Randomizer[_num_workers + 2]();
  
  _job_queues = new JobQueue[_num_workers + 1]();
  
  _randomizers[0] = Randomizer(1);
  for (int32_t i = 0; i < _num_workers; ++i) {
    std::ostringstream ss;
    ss << "job-worker-" << i;
    PT(JobWorkerThread) thread = new JobWorkerThread(ss.str(), i, this);
    thread->start(TP_normal, true);
    _randomizers[i + 1] = Randomizer(i + 2);
    _worker_threads[i] = thread;
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
    for (int32_t i = 0; i < _num_workers; ++i) {
      JobWorkerThread *thread = _worker_threads[i];
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

#ifdef THREADED_PIPELINE
  job->set_pipeline_stage(thread->get_pipeline_stage());
#endif

  if (get_num_threads() <= 0) {
    // No worker threads, execute job right now on this thread.
    job->set_state(Job::S_working);
    job->execute();
    job->set_state(Job::S_complete);
    return;
  }
  
  int queue_index = (thread->get_type() == JobWorkerThread::get_class_type()) ? DCAST(JobWorkerThread, thread)->_thread_index + 1 : 0;

  job->ref();
  job->set_state(Job::S_queued);

  //push_event(JobSystemEvent::ET_schedule_job);

  _job_queues[queue_index].push(job);

  _queued_jobs.fetch_add(1u);
  _queued_jobs.notify_one();
}

/**
 * Schedules several jobs at the same time.  A bit more efficient than
 * calling schedule() for each job.
 */
void JobSystem::
schedule(Job **jobs, size_t count, bool wait) {
  PStatTimer timer(schedule_pcollector);
  
  Thread *thread = Thread::get_current_thread();

  int32_t num_threads = get_num_threads();
  if (num_threads <= 0) {
    for (size_t i = 0; i < count; ++i) {
      Job *&job = jobs[i];
      nassertv(job != nullptr);
#ifdef THREADED_PIPELINE
      job->set_pipeline_stage(thread->get_pipeline_stage());
#endif
      job->set_state(Job::S_working);
      job->execute();
      job->set_state(Job::S_complete);
    }
    return;
  }
  
  bool is_worker = thread->get_type() == JobWorkerThread::get_class_type();
  for (size_t i = 0; i < count; ++i) {
    //int queue_index = is_worker ? DCAST(JobWorkerThread, thread)->_thread_index + 1 : i % num_threads;
    int queue_index = is_worker ? (DCAST(JobWorkerThread, thread)->_thread_index + i) % num_threads : i % num_threads;
    
    Job *&job = jobs[i];
    nassertv(job != nullptr);
#ifdef THREADED_PIPELINE
    job->set_pipeline_stage(thread->get_pipeline_stage());
#endif
    job->ref();
    job->set_state(Job::S_queued);

    _job_queues[queue_index].push(job);

    //push_event(JobSystemEvent::ET_schedule_job);
  }

  _queued_jobs.fetch_add(count);
  _queued_jobs.notify_all();

  if (!wait) { return; }
    
  for (size_t i = 0; i < count; ++i) {
    wait_job(jobs[i], thread);
  }
}

/**
 *
 */
void JobSystem::
parallel_process(size_t count, std::function<void(size_t)> func, size_t count_threshold) {
  PStatTimer timer(parallel_proc_pcollector);

  if (count == 0) {
    return;

  } else if (count == 1) {
    func(0);
    return;

  } else if (count < count_threshold || get_num_threads() <= 0) {
    // No worker threads or not enough items to prohibit scheduling jobs.
    for (size_t i = 0; i < count; ++i) {
      func(i);
    }
    return;
  }

  ParallelProcessJob job;
  job.local_object();
  job._first_item = 0;
  job._num_items = count;
  job._function = std::move(func);
  schedule(&job);
  wait_job(&job);
}

/**
 *
 */
void JobSystem::
parallel_process_per_item(size_t count, std::function<void(size_t)> func, bool wait_for_jobs) {
  PStatTimer timer(parallel_proc_per_item_pcollector);

  if (count == 0) {
    return;

  } else if (count == 1) {
    func(0);
    return;

  } else if (get_num_threads() <= 0) {
    // No worker threads to prohibit scheduling jobs.
    for (size_t i = 0; i < count; ++i) {
      func(i);
    }
    return;
  }
  
  // Allocate the array for our jobs.
  ParallelProcessJob **jobs = new ParallelProcessJob *[count + 1]();
  
  // Setup all of our jobs.
  for (size_t i = 0; i < count; ++i) {
    ParallelProcessJob *job = new ParallelProcessJob();
    job->local_object();
    job->_first_item = i;
    job->_num_items = 1;
    job->_function = func;
    jobs[i] = job;
  }
  
  // Schedule all of the indivdual jobs.
  schedule((Job **)jobs, count, wait_for_jobs);
  
  // Free our now completed jobs.
  for (size_t i = 0; i < count; ++i) {
    ParallelProcessJob *job = jobs[i];
    delete job;
  }
  delete[] jobs;
}

/**
 * Blocks until the indicated job executes to completion.
 *
 * While waiting, this thread will attempt to service other jobs
 * in the queue.
 */
void JobSystem::
wait_job(Job *job, Thread *thread) {
  nassertv(job != nullptr);
  
  PStatTimer timer(wait_job_pcollector);

  if (job->get_state() == Job::S_fresh) {
    return;
  }

#ifdef THREADED_PIPELINE
  int orig_pipeline_stage = thread->get_pipeline_stage();
#endif

  bool is_worker = (thread->get_type() == JobWorkerThread::get_class_type());
  if (js_workers_only && !is_worker) {
    while (job->get_state() != Job::S_complete) { continue; }
    return;
  }

  while (job->get_state() != Job::S_complete) {
    //Thread::relax();
    
    Job *job2 = pop_job(thread, is_worker);
    if (job2 == nullptr) { continue; }
    
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
      delete job2;
    }

    //push_event(JobSystemEvent::ET_finish_job);

    exec_job_pcollector.stop();
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
