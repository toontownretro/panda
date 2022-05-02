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

/**
 *
 */
class EXPCL_PANDA_PIPELINE JobSystem {
PUBLISHED:
  JobSystem();

  void initialize();

  INLINE void schedule(Job *job);
  INLINE void wait_for_work();

  INLINE static JobSystem *get_global_ptr();

public:
  INLINE void pop_job(PT(Job) &job);

private:
  Mutex _cv_mutex;
  // Signals to worker threads that a job has been added to the queue.
  ConditionVar _cv_work_available;

  typedef pvector<PT(JobWorkerThread)> WorkerThreads;
  WorkerThreads _worker_threads;

  typedef pdeque<PT(Job)> JobQueue;
  JobQueue _job_queue;
  Mutex _queue_lock;

  bool _initialized;

private:
  static JobSystem *_global_ptr;
};

#include "jobSystem.I"

#endif // JOBSYSTEM_H
