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

int
main(int argc, char *argv[]) {
  JobSystem *sys = JobSystem::get_global_ptr();
  AtomicAdjust::Integer count = 0;

  Thread::sleep(1.0);

  sys->parallel_process(200, [&count, sys] (int i) {
    Thread::sleep(0.05);

    uint32_t outer = Thread::get_current_thread_id();

    sys->parallel_process(50, [outer, &count] (int i) {
      std::cerr << "outer: " << outer << ", inner: " << Thread::get_current_thread_id() << "\n";
      AtomicAdjust::inc(count);
    });

    Thread::sleep(0.05);
  });

  std::cout << "Count: " << AtomicAdjust::get(count) << "\n";

  return 0;
}
