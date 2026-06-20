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
  ~JobSystem();

  void initialize();

  void new_frame();

  void schedule(Job *job);
  void schedule(Job **jobs, size_t count, bool wait);

  void parallel_process(size_t count, std::function<void(size_t)> func, size_t count_threshold = 2);
  void parallel_process_per_item(size_t count, std::function<void(size_t)> func, bool wait_for_jobs = true);

  template<typename T>
  INLINE void parallel_process(T begin, size_t count, std::function<void(const T &)> func, size_t count_threshold = 2);

  void wait_job(Job *job, Thread *thread = Thread::get_current_thread());

  INLINE static JobSystem *get_global_ptr();
  INLINE static void init_global_job_system();

  INLINE int32_t get_num_threads() const;

  INLINE void push_event(JobSystemEvent::EventType type);
  void write_events(const Filename &filename);

public:
  INLINE Job *pop_job(Thread *thread, bool is_worker);

  INLINE JobQueue *get_job_queue(size_t thread);

public:
  friend class JobWorkerThread;
  typedef PT(JobWorkerThread) *WorkerThreads;
  typedef Randomizer *Randomizers;
  
  WorkerThreads _worker_threads;
  Randomizers _randomizers;
  JobQueue *_job_queues;
  JobSystemEvent *_events;
  JobSystemEvent *_events_tail;
  // We need to protect pushes onto this queue because jobs may be queued
  // by more than one non-worker threads, i.e. App and Cull.
  Mutex _queue_lock;
  Mutex _event_lock;
  patomic_unsigned_lock_free _queued_jobs;
  int32_t _num_workers;
  bool _initialized;

private:
  static JobSystem *_global_ptr;
};

extern thread_local int js_steal_idx;

template<class T, class Pr>
INLINE void parallel_quicksort(T *data, size_t size, Pr pred, int count_threshold = 10);

#include "jobSystem.I"

#endif // JOBSYSTEM_H
