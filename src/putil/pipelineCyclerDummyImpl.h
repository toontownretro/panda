// Filename: pipelineCyclerDummyImpl.h
// Created by:  drose (31Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PIPELINECYCLERDUMMYIMPL_H
#define PIPELINECYCLERDUMMYIMPL_H

#include "pandabase.h"

#if defined(DO_PIPELINING) && !defined(HAVE_THREADS)

#include "cycleData.h"
#include "pipeline.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PipelineCyclerDummyImpl
// Description : This is a simple, single-threaded-only implementation
//               of PipelineCyclerBase.  It is only compiled when
//               DO_PIPELINING is defined, but threading is not
//               available, which is usually the case only in
//               development mode.
//
//               This implmentation is similar in principle to
//               PipelineCyclerTrivialImpl, except it does basic
//               sanity checking to ensure that you use the interface
//               in a reasonable way consistent with its design (e.g.,
//               read() is balanced with release_read(), etc.).
//
//               This is defined as a struct instead of a class,
//               mainly to be consistent with
//               PipelineCyclerTrivialImpl.
////////////////////////////////////////////////////////////////////
struct EXPCL_PANDA PipelineCyclerDummyImpl {
public:
  INLINE PipelineCyclerDummyImpl(CycleData *initial_data, Pipeline *pipeline = NULL);
  INLINE PipelineCyclerDummyImpl(const PipelineCyclerDummyImpl &copy);
  INLINE void operator = (const PipelineCyclerDummyImpl &copy);
  INLINE ~PipelineCyclerDummyImpl();

  INLINE const CycleData *read() const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write();
  INLINE CycleData *elevate_read(const CycleData *pointer);
  INLINE void release_write(CycleData *pointer);

  INLINE int get_num_stages();
  INLINE bool is_stage_unique(int n) const;
  INLINE CycleData *write_stage(int n);
  INLINE void release_write_stage(int n, CycleData *pointer);

  INLINE CycleData *cheat() const;
  INLINE int get_read_count() const;
  INLINE int get_write_count() const;

private:
  PT(CycleData) _data;
  Pipeline *_pipeline;
  short _read_count, _write_count;
};

#include "pipelineCyclerDummyImpl.I"

#endif  // DO_PIPELINING && !HAVE_THREADS

#endif

