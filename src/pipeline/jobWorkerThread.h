/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobWorkerThread.h
 * @author brian
 * @date 2022-04-30
 */

#ifndef JOBWORKERTHREAD_H
#define JOBWORKERTHREAD_H

#include "pandabase.h"
#include "thread.h"

/**
 *
 */
class EXPCL_PANDA_PIPELINE JobWorkerThread : public Thread {
  DECLARE_CLASS(JobWorkerThread, Thread);

public:
  JobWorkerThread(const std::string &name);

  virtual void thread_main() override;
};

#include "jobWorkerThread.I"

#endif // JOBWORKERTHREAD_H
