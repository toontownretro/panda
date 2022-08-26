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

  Thread::sleep(1.0);

  int array[1000];

  for (int i = 0; i < 1000; ++i) {
    array[i] = rand() % 1000;
  }

  TrueClock *clock = TrueClock::get_global_ptr();

  double start = clock->get_short_time();
  parallel_quicksort(array, 1000, [](int a, int b) { return a < b; }, 10000);
  double end = clock->get_short_time();

  std::cout << "Elapsed: " << end - start << "\n";

  //for (int i = 0; i < 1000; ++i) {
  //  std::cout << array[i] << "\n";
  //}

  return 0;
}
