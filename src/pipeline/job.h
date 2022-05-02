/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file job.h
 * @author brian
 * @date 2022-04-30
 */

#ifndef JOB_H
#define JOB_H

#include "pandabase.h"
#include "typedReferenceCount.h"

/**
 *
 */
class EXPCL_PANDA_PIPELINE Job : public TypedReferenceCount {
  DECLARE_CLASS(Job, TypedReferenceCount);

public:
  INLINE Job();

  virtual void execute() = 0;

  INLINE void set_pipeline_stage(int stage);
  INLINE int get_pipeline_stage() const;

private:
  int _pipeline_stage;
};

#include "job.I"

#endif // JOB_H
