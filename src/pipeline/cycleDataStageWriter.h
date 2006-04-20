// Filename: cycleDataStageWriter.h
// Created by:  drose (06Feb06)
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

#ifndef CYCLEDATASTAGEWRITER_H
#define CYCLEDATASTAGEWRITER_H

#include "pandabase.h"

#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataStageReader.h"

////////////////////////////////////////////////////////////////////
//       Class : CycleDataStageWriter
// Description : This class is similar to CycleDataWriter, except it
//               allows writing to a particular stage of the pipeline.
//               Usually this is used to implement writing directly to
//               an upstream pipeline value, to recompute a cached
//               value there (otherwise, the cached value would go
//               away with the next pipeline cycle).
////////////////////////////////////////////////////////////////////
template<class CycleDataType>
class CycleDataStageWriter {
public:
  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              Thread *current_thread = Thread::get_current_thread());
  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              bool force_to_0, Thread *current_thread = Thread::get_current_thread());

  INLINE CycleDataStageWriter(const CycleDataStageWriter<CycleDataType> &copy);
  INLINE void operator = (const CycleDataStageWriter<CycleDataType> &copy);

  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              CycleDataStageReader<CycleDataType> &take_from);
  INLINE CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                              CycleDataStageReader<CycleDataType> &take_from,
                              bool force_to_0);

  INLINE ~CycleDataStageWriter();

  INLINE CycleDataType *operator -> ();
  INLINE const CycleDataType *operator -> () const;

  INLINE operator CycleDataType * ();

  INLINE Thread *get_current_thread() const;

private:
#ifdef DO_PIPELINING
  // This is the data stored for a real pipelining implementation.
  PipelineCycler<CycleDataType> *_cycler;
  Thread *_current_thread;
  CycleDataType *_pointer;
  int _stage;
#else  // !DO_PIPELINING
  // This is all we need for the trivial, do-nothing implementation.
  CycleDataType *_pointer;
#endif  // DO_PIPELINING
};

#include "cycleDataStageWriter.I"

#endif
