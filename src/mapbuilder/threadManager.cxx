/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadManager.cxx
 * @author brian
 * @date 2021-07-21
 */

#include "threadManager.h"
#include "config_mapbuilder.h"
#include "lightMutexHolder.h"
#include "clockObject.h"
#include "thread.h"

#include <functional>

int ThreadManager::_work_count = 0;
int ThreadManager::_dispatch = 0;
int ThreadManager::_oldf = 0;
bool ThreadManager::_pacifier = false;
bool ThreadManager::_threaded = false;
double ThreadManager::_thread_start = 0.0;
double ThreadManager::_thread_times[THREAD_TIMES_SIZE];
int ThreadManager::_num_threads = 1;
LightMutex ThreadManager::_lock("mapbuilder-mutex");
ThreadManager::Threads ThreadManager::_thread_handles;
ThreadPriority ThreadManager::_thread_priority = TP_urgent;
ThreadManager::ThreadFunction ThreadManager::_work_function = nullptr;

class BuilderThread : public Thread {
public:
  BuilderThread() :
    Thread("builder-thread", "builder-thread-sync"),
    _func(nullptr),
    _val(0),
    _finished(false)
  {
  }

  virtual void thread_main() {
    nassertv(_func != nullptr);
    _func(_val);
    _finished = true;
  }

  void set_function(ThreadManager::ThreadFunction func) {
    _func = func;
  }

  void set_value(int val) {
    _val = val;
  }

  bool is_finished() const {
    return _finished;
  }

private:
  ThreadManager::ThreadFunction _func;
  int _val;
  bool _finished;
};

/**
 *
 */
int ThreadManager::
get_thread_work() {
  LightMutexHolder holder(_lock);

  int dispatch = _dispatch;

  if (dispatch == 0) {
    _oldf = 0;
  }

  if (dispatch > _work_count) {
    mapbuilder_cat.error()
      << "dispatch > workcount\n";
    return -1;

  } else if (dispatch == _work_count) {
    if (mapbuilder_cat.is_debug()) {
      mapbuilder_cat.debug()
        << "dispatch == workcount\n";
    }
    return -1;

  } else if (dispatch < 0) {
    mapbuilder_cat.error()
      << "dispatch < 0\n";
    return -1;
  }

  int f = THREAD_TIMES_SIZE * ((PN_stdfloat)dispatch / _work_count);
  f = std::clamp(f, _oldf, 40);
  if (f != _oldf) {
    for (int i = _oldf + 1; i <= f; i++) {
      if (!(i % 4)) {
        nout << i / 4;

      } else {
        if (i != 40) {
          nout << ".";
        }
      }
    }
    _oldf = f;
  }

  _dispatch++;

  return dispatch;
}

/**
 *
 */
void ThreadManager::
lock() {
  _lock.acquire();
}

/**
 *
 */
void ThreadManager::
unlock() {
  _lock.release();
}

/**
 *
 */
int ThreadManager::
get_current_thread_number() {
  Thread *th = Thread::get_current_thread();

  for (int i = 0; i < (int)_thread_handles.size(); i++) {
    if (_thread_handles[i] == th) {
      return i;
    }
  }

  return -1;
}

/**
 *
 */
void ThreadManager::
run_threads_on_individual(int work_count, bool show_pacifier,
                          ThreadFunction func) {
  _work_function = func;
  run_threads_on(work_count, show_pacifier, thread_worker_function);
}

/**
 *
 */
void ThreadManager::
run_threads_on_individual(const std::string &name, int work_count, bool pacifier,
                          ThreadFunction func) {
  nout << name << ": ";
  run_threads_on_individual(work_count, pacifier, func);
}

/**
 *
 */
void ThreadManager::
run_threads_on(int work_count, bool show_pacifier,
               ThreadFunction func) {
  _thread_handles.clear();

  _thread_start = ClockObject::get_global_clock()->get_real_time();
  for (int i = 0; i < THREAD_TIMES_SIZE; i++) {
    _thread_times[i] = 0;
  }
  _dispatch = 0;
  _work_count = work_count;
  _oldf = -1;
  _pacifier = show_pacifier;
  _threaded = true;

  if (_work_count < _dispatch) {
    mapbuilder_cat.error()
      << "run_threads_on: work count ("
      << _work_count << ") < dispatch ("
      << _dispatch << ")\n";
  }
  nassertv(_work_count >= _dispatch);

  // Create all the threads
  for (int i = 0; i < _num_threads; i++) {
    PT(BuilderThread) thread = new BuilderThread;
    thread->set_function(func);
    thread->set_value(i);
    _thread_handles.push_back(thread);
  }

  nout << "[";

  // Start all the threads
  for (int i = 0; i < _num_threads; i++) {
    if (!_thread_handles[i]->start(_thread_priority, true)) {
      mapbuilder_cat.error()
        << "Failed to start thread " << i << "\n";
    }
  }

  // Wait for threads to complete
  for (int i = 0; i < _num_threads; i++) {
    _thread_handles[i]->join();
  }

  _threaded = false;
  double end = ClockObject::get_global_clock()->get_real_time();
  nout << "] Done (" << (int)(end - _thread_start) << " seconds)\n";
}

/**
 *
 */
void ThreadManager::
run_threads_on(const std::string &name, int work_count, bool pacifier,
               ThreadFunction func) {
  nout << name << " ";
  run_threads_on(work_count, pacifier, func);
}

/**
 *
 */
void ThreadManager::
thread_worker_function(int) {
  int work;
  while ((work = get_thread_work()) != -1) {
    _work_function(work);
  }
}
