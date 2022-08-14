/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobSystem.I
 * @author brian
 * @date 2022-04-30
 */

/**
 * Blocks this thread until all queued jobs have been executed and completed.
 */
//INLINE void JobSystem::
//wait_all_jobs() {
//  while (AtomicAdjust::get(_queued_jobs) > 0) {
//  }
//}

/**
 *
 */
INLINE void JobSystem::
schedule(Job *job) {
  job->set_pipeline_stage(Thread::get_current_pipeline_stage());

  if (!_worker_threads.empty()) {
    //Thread *thread = Thread::get_current_thread();
    //if (thread != Thread::get_main_thread()) {
    //  JobWorkerThread *jthread = DCAST(JobWorkerThread, thread);
    //  job->set_parent(jthread->get_current_job());
    //}

    _queue_lock.acquire();
    _job_queue.push_back(job);
    job->set_state(Job::S_queued);
    _queue_lock.release();
    //AtomicAdjust::inc(_queued_jobs);
    _cv_work_available.notify_all();

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
INLINE void JobSystem::
schedule(const pvector<PT(Job)> &jobs, bool wait) {
  for (Job *job : jobs) {
    job->set_pipeline_stage(Thread::get_current_pipeline_stage());
  }

  if (!_worker_threads.empty()) {
    _queue_lock.acquire();
    for (Job *job : jobs) {
      _job_queue.push_back(job);
      job->set_state(Job::S_queued);
      _cv_work_available.notify_all();
    }
    _queue_lock.release();

    //_cv_work_available.notify_all();

    if (wait) {
      for (Job *job : jobs) {
        wait_job(job);
      }
    }

  } else {
    for (Job *job : jobs) {
      job->set_state(Job::S_working);
      job->execute();
      job->set_state(Job::S_complete);
    }
  }
}

/**
 *
 */
INLINE void JobSystem::
pop_job(PT(Job) &job) {
  MutexHolder holder(_queue_lock);
  if (_job_queue.empty()) {
    job = nullptr;
  } else {
    job = _job_queue.front();
    _job_queue.pop_front();
  }
}

/**
 *
 */
INLINE void JobSystem::
wait_for_work() {
  MutexHolder holder(_cv_mutex);
  _cv_work_available.wait();
}

/**
 * Called from a worker thread after it finishes executing a job.
 */
INLINE void JobSystem::
job_finished() {
  //AtomicAdjust::dec(_queued_jobs);
}

/**
 * Returns the global JobSystem pointer.
 */
INLINE JobSystem *JobSystem::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new JobSystem;
  }

  return _global_ptr;
}

/**
 *
 */
INLINE void JobSystem::
parallel_process(int count, std::function<void(int)> func) {
  int num_per_thread = count / (int)_worker_threads.size();
  int remainder = count % (int)_worker_threads.size();

  pvector<PT(Job)> jobs;
  jobs.reserve(_worker_threads.size());
  if (num_per_thread > 0) {
    for (size_t i = 0; i < _worker_threads.size(); ++i) {
      int first = num_per_thread * i;
      int count = num_per_thread;
      if (i == (_worker_threads.size() - 1)) {
        count += remainder;
      }
      PT(ParallelProcessJob) job = new ParallelProcessJob(first, count, func);
      jobs.push_back(std::move(job));
    }
  } else {
    // If there are fewer items than worker threads, create a job for
    // each item.
    for (int i = 0; i < count; ++i) {
      PT(ParallelProcessJob) job = new ParallelProcessJob(i, 1, func);
      jobs.push_back(std::move(job));
    }
  }
  schedule(jobs, true);
}
