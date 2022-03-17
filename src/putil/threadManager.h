/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadManager.h
 * @author brian
 * @date 2021-07-21
 */

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include "pandabase.h"
#include "atomicAdjust.h"
#include "lightMutex.h"
#include "pvector.h"
#include "pointerTo.h"
#include "thread.h"

#include <functional>

#define THREAD_TIMES_SIZE 40
#define THREAD_TIMES_SIZEF (float)THREAD_TIMES_SIZE

//
// 5.59 + 8.59 + ((1.85+3.95+2.49+5.19) / 2) = 20.92
// 6.99 + 9.25 + ((2.34+4.5+1.49+6.27) / 2) = 23.54
// 14.39 + ((2.18 + 4.5 + 2.49 + 7.26) / 2) = 22.60
// 2.15 + 4.79 + 5.15 + ((1.56 + 3.34 + 2.49 + 4.44) / 2) = 18
//

/**
 *
 */
class EXPCL_PANDA_PUTIL ThreadManager {
public:
  typedef std::function<void(int)> ThreadFunction;

  static int get_thread_work();

  static void lock();
  static void unlock();

  static int get_current_thread_number();

  static void run_threads_on_individual(int work_count, bool show_pacifier,
                                        ThreadFunction func);
  static void run_threads_on_individual(const std::string &name, int work_count,
                                        bool show_pacifier, ThreadFunction func);
  static void run_threads_on(int work_count, bool show_pacifier,
                             ThreadFunction func);
  static void run_threads_on(const std::string &name, int work_count,
                             bool show_pacifier, ThreadFunction func);

private:
  static void thread_worker_function(int);

public:

  static int _work_count;
  static int _dispatch;
  static int _oldf;
  static bool _pacifier;
  static bool _threaded;
  static double _thread_start;
  static double _thread_times[THREAD_TIMES_SIZE];
  static LightMutex _lock;

  static int _num_threads;

  static ThreadPriority _thread_priority;

  typedef pvector<PT(Thread)> Threads;
  static Threads _thread_handles;

  static ThreadFunction _work_function;
};

#include "threadManager.I"

#endif // THREADMANAGER_H
