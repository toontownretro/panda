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
  _pipeline_stage(0),
  _state(S_fresh)
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

/**
 *
 */
INLINE void Job::
set_state(State state) {
  AtomicAdjust::set(_state, (int)state);
}

/**
 *
 */
INLINE Job::State Job::
get_state() const {
  return (State)AtomicAdjust::get(_state);
}

/**
 *
 */
//INLINE void Job::
//set_parent(Job *parent) {
//  _parent = parent;
//}

/**
 *
 */
//INLINE Job *Job::
//get_parent() const {
//  return _parent;
//}

/**
 *
 */
INLINE ParallelProcessJob::
ParallelProcessJob(int first_item, int num_items, ProcessFunc func) :
  _first_item(first_item),
  _num_items(num_items),
  _function(func)
{
}
