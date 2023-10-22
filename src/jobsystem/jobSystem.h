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
#include "trueClock.h"

#include <functional>

/**
 * Event log for debugging purposes.
 */
class JobSystemEvent {
public:
  enum EventType {
    // Worker thread states.
    ET_thread_wake,
    ET_thread_sleep,

    // A job is scheduled.
    ET_schedule_job,

    // Job work tracking.
    ET_start_job,
    ET_finish_job,
  };

  EventType type;
  std::string thread_name;
  double time;

  JobSystemEvent *next = nullptr;
};

/**
 *
 */
class EXPCL_PANDA_JOBSYSTEM JobSystem {
public:
  typedef WorkStealingQueue<Job *> JobQueue;

PUBLISHED:
  JobSystem();

  void initialize();

  void new_frame();

  void schedule(Job *job);
  void schedule(Job **jobs, int count, bool wait);

  void parallel_process(int count, std::function<void(int)> func, int count_threshold = 2, bool job_per_item = false);

  template<typename T>
  INLINE void parallel_process(T begin, int count, std::function<void(const T &)> func, int count_threshold = 2);

  void wait_job(Job *job, Thread *thread = Thread::get_current_thread());

  INLINE static JobSystem *get_global_ptr();

  INLINE int get_num_threads() const;

  INLINE void push_event(JobSystemEvent::EventType type);
  void write_events(const Filename &filename);

public:
  INLINE Job *pop_job(Thread *thread, bool is_worker);

  INLINE JobQueue *get_job_queue(int thread);

public:
  typedef pvector<PT(JobWorkerThread)> WorkerThreads;
  WorkerThreads _worker_threads;
  pvector<Randomizer> _randomizers;
  JobQueue *_job_queues;

  patomic_unsigned_lock_free _queued_jobs;

  // We need to protect pushes onto this queue because jobs may be queued
  // by more than one non-worker threads, i.e. App and Cull.
  Mutex _queue_lock;

  bool _initialized;

  friend class JobWorkerThread;

  Mutex _event_lock;
  JobSystemEvent *_events;
  JobSystemEvent *_events_tail;

private:
  static JobSystem *_global_ptr;
};

extern thread_local int js_steal_idx;

template<class T, class Pr>
INLINE void parallel_quicksort(T *data, size_t size, Pr pred, int count_threshold = 10);

#include "jobSystem.I"

#endif // JOBSYSTEM_H
