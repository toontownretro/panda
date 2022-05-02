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

class TestJob : public Job {
public:
  TestJob(int start, int end, float *array) :
    _start(start),
    _end(end),
    _array(array)
  {
  }

  virtual void execute() override {
    for (int i = _start; i < _end; ++i) {
      _array[i] = csqrt(_array[i]);
    }

    std::cerr << "done from " << Thread::get_current_thread_id() << "\n";
  }

public:
  int _start, _end;
  float *_array;
};

int
main(int argc, char *argv[]) {
  JobSystem *sys = JobSystem::get_global_ptr();
  sys->initialize();

  float *array = new float[1000000];
  for (int i = 0; i < 1000000; ++i) {
    array[i] = (float)rand() / (float)(RAND_MAX * 0.1f);
  }

  int num_workers = 10000;
  int num_per_job = 1000000 / num_workers;
  int remainder = 1000000 % num_workers;
  pvector<PT(TestJob)> jobs;
  for (int i = 0; i < num_workers; ++i) {
    int start = num_per_job * i;
    int end = num_per_job * (i + 1);
    if (i == num_workers - 1) {
      end += remainder;
    }
    std::cout << start << " -> " << end << "\n";
    PT(TestJob) j = new TestJob(start, end, array);
    jobs.push_back(j);
    //jobs.push_back(new TestJob(start, end, array));
  }

  for (TestJob *j : jobs) {
    sys->schedule(j);
  }

  int i = 0;
  while (true) {
    ++i;
    Thread::sleep(5.0);
  }

  return 0;
}
