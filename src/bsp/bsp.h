/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bsp.h
 * @author lachbr
 * @date 2020-12-30
 */

#ifndef BSP_H
#define BSP_H

#include "pandabase.h"

// The magic number for a BSP file.
static const int _bsp_header = (('P' << 24) + ('S' << 16) + ('B' << 8) + 'P');
// The current BSP file version.
static const int _bsp_version = 1;
// The minimum BSP version we will read.
static const int _bsp_version_min = 1;

// This is the magic number for a Source Engine BSP file.
static const int _source_bsp_header = (('P' << 24) + ('S' << 16) + ('B' << 8) + 'V');
static const int _source_bsp_version = 21;
static const int _source_bsp_version_min = 19;

#endif // BSP_H
