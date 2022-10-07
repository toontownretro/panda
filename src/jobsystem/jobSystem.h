/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobSystem.h
 * @author brian
 * @date 2022-04-30
 */

#ifndef JOBSYSTEM_H
#define JOBSYSTEM_H

#include "pandabase.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "pvector.h"
#include "jobWorkerThread.h"
#include "job.h"
#include "pointerTo.h"
#include "pdeque.h"
#include "mutexHolder.h"
#include "dcast.h"
#include "workStealingQueue.h"
#include "psemaphore.h"
#include "randomizer.h"

#include <functional>

/**
 *
 */
class EXPCL_PANDA_JOBSYSTEM JobSystem {
PUBLISHED:
  JobSystem();

  void initialize();

  void schedule(Job *job);
  void schedule(Job **jobs, int count, bool wait);

  void parallel_process(int count, std::function<void(int)> func, int count_threshold = 2, bool job_per_item = false);

  template<typename T>
  INLINE void parallel_process(T begin, int count, std::function<void(const T &)> func, int count_threshold = 2);

  void wait_job(Job *job, Thread *thread = Thread::get_current_thread());

  INLINE static JobSystem *get_global_ptr();

public:
  ALWAYS_INLINE Job *get_job_for_thread(Thread *thread, bool is_worker);
  ALWAYS_INLINE Job *pop_job(Thread *thread, bool is_worker);

public:
  Mutex _cv_mutex;
  // Signals to worker threads that a job has been added to the queue.
  ConditionVar _cv_work_available;
  // The condition is that we have at least 1 queued job on any thread
  // queue.
  AtomicAdjust::Integer _queued_jobs;

  typedef pvector<PT(JobWorkerThread)> WorkerThreads;
  WorkerThreads _worker_threads;

  typedef WorkStealingQueue<Job *> JobQueue;
  JobQueue _job_queue;
  // We need to protect pushes onto this queue because jobs may be queued
  // by more than one non-worker threads, i.e. App and Cull.
  Mutex _queue_lock;

  bool _initialized;

  friend class JobWorkerThread;

private:
  static JobSystem *_global_ptr;
};

extern thread_local int js_steal_idx;

template<class T, class Pr>
INLINE void parallel_quicksort(T *data, size_t size, Pr pred, int count_threshold = 10);

#include "jobSystem.I"

#endif // JOBSYSTEM_H
