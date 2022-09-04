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

IMPLEMENT_CLASS(Job);
IMPLEMENT_CLASS(GenericJob);
IMPLEMENT_CLASS(ParallelProcessJob);

/**
 *
 */
void GenericJob::
execute() {
  _func();
}

/**
 *
 */
void ParallelProcessJob::
execute() {
  for (int i = 0; i < _num_items; ++i) {
    _function(_first_item + i);
  }
}