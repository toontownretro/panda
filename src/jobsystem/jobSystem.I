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

#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector parallel_proc_iter_pcollector("JobSystem:ParallelProcessIter");

/**
 *
 */
template<typename T>
INLINE void JobSystem::
parallel_process(T begin, int count, std::function<void(const T &)> func, int count_threshold) {
  PStatTimer timer(parallel_proc_iter_pcollector);

  if (count == 0) {
    return;

  } else if (count == 1) {
    func(begin);
    return;

  } else if (count < count_threshold || _worker_threads.empty()) {
    // No worker threads or not enough items to prohibit scheduling jobs.
    T it = begin;
    for (int i = 0; i < count; ++it, ++i) {
      func(it);
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

  int job_count = std::min(count, thread_count);

  pvector<ParallelProcessIterJob<T> > jobs;
  jobs.resize(job_count);
  if (num_per_thread > 0) {
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
      jobs[i]._begin = begin;
      jobs[i]._first = first;
      jobs[i]._count = count;
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
      jobs[i]._begin = begin;
      jobs[i]._first = i;
      jobs[i]._count = 1;
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
template<class T, class Pr>
INLINE void
parallel_quicksort(T *data, size_t size, Pr pred, int count_threshold) {
  JobSystem *js = JobSystem::get_global_ptr();
  r_parallel_quicksort(data, pred, 0, (int)size - 1, count_threshold, js);
}

/**
 *
 */
template<class T, class Pr>
INLINE void
r_serial_quicksort(T *data, Pr pred, int left, int right) {
  if ((right - left) < 2) {
    return;
  }

  int i = left;
  int j = right - 1;

  T &pivot = data[i + (j - i) / 2];
  T tmp;

  if (pred(data[i], pivot)) {
    while (pred(data[++i], pivot));
  }
  if (pred(pivot, data[j])) {
    while (pred(pivot, data[--j]));
  }

  while (i < j) {
    tmp = std::move(data[i]);
    data[i] = std::move(data[j]);
    data[j] = std::move(tmp);

    while (pred(data[++i], pivot));
    while (pred(pivot, data[--j]));
  }
  ++j;

  r_serial_quicksort(data, pred, left, j);
  r_serial_quicksort(data, pred, j, right);
}

/**
 *
 */
template<class T, class Pr>
INLINE void
r_parallel_quicksort(T *data, Pr pred, int left, int right, int cutoff, JobSystem *js) {

  if ((right - left) < cutoff) {
    r_serial_quicksort(data, pred, left, right);

  } else {

    int i = left;
    int j = right - 1;
    T &pivot = data[i + (j - i) / 2];
    T tmp;

    if (pred(data[i], pivot)) {
      while (pred(data[++i], pivot));
    }
    if (pred(pivot, data[j])) {
      while (pred(pivot, data[--j]));
    }

    while (i < j) {
      tmp = std::move(data[i]);
      data[i] = std::move(data[j]);
      data[j] = std::move(tmp);

      while (pred(data[++i], pivot));
      while (pred(pivot, data[--j]));
    }
    ++j;

    // Sort both sides in parallel.

    GenericJob left_job([=] () {
      r_parallel_quicksort(data, pred, left, j, cutoff, js);
    });
    left_job.local_object();
    GenericJob right_job([=] () {
      r_parallel_quicksort(data, pred, j, right, cutoff, js);
    });
    right_job.local_object();

    Job *jobs[2] = { &left_job, &right_job };
    js->schedule(jobs, 2, true);
  }
}

/**
 *
 */
ALWAYS_INLINE Job *JobSystem::
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
      int index = js_steal_idx++;//random.random_int((int)_worker_threads.size());
      if (js_steal_idx >= (int)_worker_threads.size()) {
        js_steal_idx = 0;
      }
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

/**
 *
 */
ALWAYS_INLINE Job *JobSystem::
pop_job(Thread *thread, bool is_worker) {
  Job *job = get_job_for_thread(thread, is_worker);
  if (job != nullptr) {
    AtomicAdjust::dec(_queued_jobs);
  }
  return job;
}
