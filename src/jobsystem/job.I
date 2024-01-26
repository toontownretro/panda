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
INLINE GenericJob::
GenericJob(Func func) :
  _func(func)
{
}

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

/**
 *
 */
template<class T>
void ParallelProcessIterJob<T>::
execute() {
  if (_num_items > 1) {
    int left_count = _num_items / 2;
    int right_count = _num_items - left_count;

    ParallelProcessIterJob<T> left_job;
    left_job.local_object();
    left_job._iter_start = _iter_start;
    left_job._first_item = _first_item;
    left_job._num_items = left_count;
    left_job._function = _function;
    
    ParallelProcessIterJob<T> right_job;
    right_job.local_object();
    right_job._iter_start = _iter_start;
    right_job._first_item = _first_item + left_count;
    right_job._num_items = right_count;
    right_job._function = _function;

    JobSystem *js = JobSystem::get_global_ptr();
    js->schedule(&left_job);
    js->schedule(&right_job);
    js->wait_job(&left_job);
    js->wait_job(&right_job);
  } else {
    T it = std::next(_iter_start, _first_item);
    for (size_t i = 0; i < _num_items; ++it, ++i) {
      _function(it);
    }
  }
}