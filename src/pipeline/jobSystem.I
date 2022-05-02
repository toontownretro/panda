/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobSystem.I
 * @author brian
 * @date 2022-04-30
 */

/**
 *
 */
INLINE void JobSystem::
schedule(Job *job) {
  _queue_lock.acquire();
  _job_queue.push_back(job);
  _queue_lock.release();
  _cv_work_available.notify();
}

/**
 *
 */
INLINE void JobSystem::
pop_job(PT(Job) &job) {
  MutexHolder holder(_queue_lock);
  if (_job_queue.empty()) {
    job = nullptr;
  } else {
    job = _job_queue.front();
    _job_queue.pop_front();
  }
}

/**
 *
 */
INLINE void JobSystem::
wait_for_work() {
  MutexHolder holder(_cv_mutex);
  _cv_work_available.wait();
}

/**
 * Returns the global JobSystem pointer.
 */
INLINE JobSystem *JobSystem::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new JobSystem;
  }

  return _global_ptr;
}
