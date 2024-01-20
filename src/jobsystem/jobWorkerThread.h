/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobWorkerThread.h
 * @author brian
 * @date 2022-04-30
 */

#ifndef JOBWORKERTHREAD_H
#define JOBWORKERTHREAD_H

#include "pandabase.h"
#include "thread.h"
#include "workStealingQueue.h"
#include "atomicAdjust.h"

class Job;
class JobSystem;

/**
 *
 */
class EXPCL_PANDA_JOBSYSTEM JobWorkerThread : public Thread {
  DECLARE_CLASS(JobWorkerThread, Thread);

public:
  enum State {
    S_idle,
    S_busy,
  };

  JobWorkerThread(const std::string &name, int index, JobSystem *mgr);

  virtual void thread_main() override;

  INLINE Job *get_current_job() const;

public:
  Job *_current_job;

  AtomicAdjust::Integer _state;

  patomic_flag _pstats_tick_signal;

  int _thread_index;

  JobSystem *_mgr;
};

#include "jobWorkerThread.I"

#endif // JOBWORKERTHREAD_H
