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
 * Returns the global JobSystem pointer.  It is an error to call this
 * if the job system has not been initialized yet.
 */
INLINE JobSystem *JobSystem::
get_global_ptr() {
  nassertr(_global_ptr != nullptr, nullptr);
  return _global_ptr;
}

/**
 *
 */
INLINE void JobSystem::
init_global_job_system() {
  // Ensure we call from the main thread only.
  assert(Thread::get_current_thread() == Thread::get_main_thread());
  if (_global_ptr == nullptr) {
    _global_ptr = new JobSystem;
  }
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
INLINE Job *JobSystem::
pop_job(Thread *thread, bool is_worker) {
  JobWorkerThread *jthread = nullptr;
  int queue_index;
  if (is_worker) {
    jthread = DCAST(JobWorkerThread, thread);
    queue_index = jthread->_thread_index + 1;
  } else {
    queue_index = 0;
  }

  std::optional<Job *> job = _job_queues[queue_index].pop();
  if (job.has_value()) {
    _queued_jobs.fetch_sub(1u);
    return job.value();
  }

  int steal_index = _randomizers[queue_index].random_int(_worker_threads.size() + 1);
  if (steal_index != queue_index) {
    job = _job_queues[steal_index].steal();
    if (job.has_value()) {
      _queued_jobs.fetch_sub(1u);
      return job.value();
    }
  }

  return nullptr;
}

/**
 * Returns the number of worker threads.
 */
INLINE int JobSystem::
get_num_threads() const {
  return (int)_worker_threads.size();
}

/**
 *
 */
INLINE void JobSystem::
push_event(JobSystemEvent::EventType type) {
  JobSystemEvent *event = new JobSystemEvent;
  event->time = TrueClock::get_global_ptr()->get_short_time();
  event->type = type;
  event->thread_name = Thread::get_current_thread()->get_name();

  MutexHolder holder(_event_lock);
  if (_events == nullptr) {
    _events = event;
    _events_tail = event;
  } else {
    _events_tail->next = event;
    _events_tail = event;
  }
}

/**
 *
 */
INLINE JobSystem::JobQueue *JobSystem::
get_job_queue(int thread) {
  return &_job_queues[thread];
}
