/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file job.cxx
 * @author brian
 * @date 2022-04-30
 */

#include "job.h"
#include "jobSystem.h"

IMPLEMENT_CLASS(Job);
IMPLEMENT_CLASS(GenericJob);
IMPLEMENT_CLASS(ParallelProcessJob);

/**
 *
 */
void GenericJob::
execute() {
  nassertv(_func != nullptr);
  _func();
}

/**
 *
 */
void ParallelProcessJob::
execute() {
  nassertv(_num_items > 0);
  nassertv(_function != nullptr);
  if (_num_items == 1) { _function(_first_item); return; }
  
  size_t left_count = _num_items / 2;
  size_t right_count = _num_items - left_count;

  ParallelProcessJob left_job;
  left_job.local_object();
  left_job._first_item = _first_item;
  left_job._num_items = left_count;
  left_job._function = _function;
  
  ParallelProcessJob right_job;
  right_job.local_object();
  right_job._first_item = _first_item + left_count;
  right_job._num_items = right_count;
  right_job._function = _function;

  JobSystem *js = JobSystem::get_global_ptr();
  js->schedule(&left_job);
  js->schedule(&right_job);
  js->wait_job(&left_job);
  js->wait_job(&right_job);
}
