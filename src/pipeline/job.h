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
#include "atomicAdjust.h"
#include "deletedChain.h"
#include <functional>

/**
 *
 */
class EXPCL_PANDA_PIPELINE Job : public TypedReferenceCount {
  DECLARE_CLASS(Job, TypedReferenceCount);

public:
  INLINE Job();

  enum State {
    S_fresh,
    S_queued,
    S_working,
    S_complete,
  };

  virtual void execute() = 0;

  INLINE void set_pipeline_stage(int stage);
  INLINE int get_pipeline_stage() const;

  INLINE void set_state(State state);
  INLINE State get_state() const;

  //INLINE void set_parent(Job *parent);
  //INLINE Job *get_parent() const;

private:
  int _pipeline_stage;
  //int _depth;
  AtomicAdjust::Integer _state;

  //PT(Job) _parent;
};

/**
 *
 */
class EXPCL_PANDA_PIPELINE ParallelProcessJob : public Job {
  DECLARE_CLASS(ParallelProcessJob, Job);

public:
  ALLOC_DELETED_CHAIN(ParallelProcessJob);

  typedef std::function<void(int)> ProcessFunc;

  INLINE ParallelProcessJob(int first_item, int num_items, ProcessFunc func);

  virtual void execute() override;

protected:
  int _first_item;
  int _num_items;

  ProcessFunc _function;
};

#include "job.I"

#endif // JOB_H
