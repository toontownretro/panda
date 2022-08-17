/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_jobs.cxx
 * @author brian
 * @date 2022-04-29
 */

#include "pandabase.h"
#include "pdeque.h"
#include "typeHandle.h"
#include <functional>
#include "typedReferenceCount.h"
#include "atomicAdjust.h"
#include "thread.h"
#include "pointerTo.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "cmath.h"
#include "conditionVar.h"
#include "jobSystem.h"
#include "job.h"
#include "trueClock.h"

int
main(int argc, char *argv[]) {
  JobSystem *sys = JobSystem::get_global_ptr();
  AtomicAdjust::Integer count = 0;
  int job_count = 500;

  Mutex lock("print-lock");

  Thread::sleep(1.0);

  TrueClock *clock = TrueClock::get_global_ptr();

  double start = clock->get_long_time();

  double wait_time = 0.05;

  pset<Thread *> unique_threads;

  std::cerr << "Start now\n";

  sys->parallel_process(job_count, [&count, wait_time, &lock, &unique_threads] (int i) {
    lock.acquire();
    unique_threads.insert(Thread::get_current_thread());
    lock.release();
    AtomicAdjust::inc(count);
    Thread::sleep(wait_time);
  });
  //for (int i = 0; i < job_count; ++i) {
  //  AtomicAdjust::inc(count);
  //  Thread::sleep(wait_time);
  //}

  double end = clock->get_long_time();

  std::cout << "Count: " << AtomicAdjust::get(count) << "\n";
  std::cout << "Elapsed: " << end - start << "\n";
  std::cout << "Estimated serial time: " << job_count * wait_time << "\n";
  std::cout << unique_threads.size() << " / " << sys->_worker_threads.size() << " did work\n";

  return 0;
}
