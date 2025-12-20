/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobHelper.h
 * @author brian
 * @date 2025-12-20
 */

#ifndef JOBHELPER_H
#define JOBHELPER_H

class Job;

// Stupid thing to work around circular include in include template.
class JobHelper {
public:
  static void schedule_job(Job *job);
  static void wait_job(Job *job);
};

#include "jobHelper.I"

#endif // JOBHELPER_H
