/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file job.I
 * @author brian
 * @date 2022-04-30
 */

/**
 *
 */
INLINE Job::
Job() :
  _pipeline_stage(0)
{
}

/**
 * Sets the pipeline stage of the job.  This should be the pipeline stage
 * of the thread that scheduled the job.
 */
INLINE void Job::
set_pipeline_stage(int stage) {
  _pipeline_stage = stage;
}

/**
 * Returns the pipeline stage of the job.  This is the pipeline stage of the
 * thread that originally scheduled this job.  The worker thread's pipeline
 * stage will be set to this value, so the job reads/writes to the correct
 * pipeline stage from any worker thread.
 */
INLINE int Job::
get_pipeline_stage() const {
  return _pipeline_stage;
}
