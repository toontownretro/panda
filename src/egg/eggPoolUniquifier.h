// Filename: eggPoolUniquifier.h
// Created by:  drose (09Nov00)
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

#ifndef EGGPOOLUNIQUIFIER_H
#define EGGPOOLUNIQUIFIER_H

#include "pandabase.h"

#include "eggNameUniquifier.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPoolUniquifier
// Description : This is a specialization of EggNameUniquifier to
//               generate unique names for textures, materials, and
//               vertex pools prior to writing out an egg file.  It's
//               automatically called by EggData prior to writing out
//               an egg file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPoolUniquifier : public EggNameUniquifier {
PUBLISHED:
  EggPoolUniquifier();

  virtual string get_category(EggNode *node);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNameUniquifier::init_type();
    register_type(_type_handle, "EggPoolUniquifier",
                  EggNameUniquifier::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#endif


