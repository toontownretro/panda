// Filename: geomBinUnsorted.h
// Created by:  drose (13Apr00)
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

#ifndef GEOMBINUNSORTED_H
#define GEOMBINUNSORTED_H

#include <pandabase.h>

#include "geomBin.h"

#include <pointerTo.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : GeomBinUnsorted
// Description : This kind of GeomBin will group the GeomNodes
//               together by state (since that's how they come from
//               the CullTraverser, anyway), but won't attempt to
//               render the various different states in any particular
//               order.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinUnsorted : public GeomBin {
PUBLISHED:
  INLINE GeomBinUnsorted(const string &name);
  virtual ~GeomBinUnsorted();

public:
  virtual void clear_current_states();
  virtual void record_current_state(GraphicsStateGuardian *gsg,
                                    CullState *cs, int draw_order,
                                    CullTraverser *trav);

  virtual void draw(CullTraverser *trav);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  typedef vector< PT(CullState) > CullStates;
  CullStates _cull_states;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomBin::init_type();
    register_type(_type_handle, "GeomBinUnsorted",
                  GeomBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomBinUnsorted.I"

#endif
