// Filename: pipelineCyclerBase.h
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PIPELINECYCLERBASE_H
#define PIPELINECYCLERBASE_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipeline.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PipelineCyclerBase
// Description : This is the non-template part of the implementation
//               of PipelineCycler.  See PipelineCycler.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PipelineCyclerBase {
public:
  PipelineCyclerBase(CycleData *initial_data, Pipeline *pipeline = NULL);
  ~PipelineCyclerBase();

  INLINE const CycleData *read() const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write();
  INLINE void increment_write(CycleData *pointer);
  INLINE void release_write(CycleData *pointer);

private:
  PT(CycleData) _data;
  Pipeline *_pipeline;
  short _read_count, _write_count;
};

#include "pipelineCyclerBase.I"

#endif

