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
#include "clockObject.h"

AtomicAdjust::Integer result;

PStatCollector main_coll("App:Main");

int
main(int argc, char *argv[]) {
  JobSystem *sys = JobSystem::get_global_ptr();

  Thread::sleep(1.0);

  std::string blah;
  std::cout << "enter to start ";
  std::cin >> blah;

  PStatClient::connect();


  int array[500];

  for (int i = 0; i < 500; ++i) {
    array[i] = rand() % 500;
  }

  TrueClock *clock = TrueClock::get_global_ptr();

  double start = clock->get_short_time();

  while (true) {
    ClockObject::get_global_clock()->tick();
    PStatClient::main_tick();
    sys->new_frame();

    main_coll.start();

    sys->parallel_process(500, [&] (int i) {


      int answer = array[i];
      AtomicAdjust::add(result, answer);
      Thread::sleep(0.0005);


    });
    std::cout << "done\n";

    main_coll.stop();
  }



  double end = clock->get_short_time();

  std::cout << "Elapsed: " << end - start << "\n";

  std::cout << "Result: " << result << "\n";

  std::cout << "any key to exit ";
  std::cin >> blah;

  return 0;
}
