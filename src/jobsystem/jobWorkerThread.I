/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobWorkerThread.I
 * @author brian
 * @date 2022-04-30
 */

/**
 * Returns the job that this worker thread is currently executing, or
 * nullptr if the thread is idle.
 */
INLINE Job *JobWorkerThread::
get_current_job() const {
  return _current_job;
}
