// Filename: config_linmath.h
// Created by:  drose (23Feb00)
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

#ifndef CONFIG_LINMATH_H
#define CONFIG_LINMATH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(linmath, EXPCL_PANDA, EXPTP_PANDA);

extern const bool paranoid_hpr_quat;
extern const bool temp_hpr_fix;

extern EXPCL_PANDA void init_liblinmath();

#endif
