/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_bsp_file.h
 * @author lachbr
 * @date 2021-01-02
 */

#ifndef LOAD_BSP_FILE_H
#define LOAD_BSP_FILE_H

#include "config_bsp.h"
#include "pointerTo.h"

class BSPData;
class PandaNode;

BEGIN_PUBLISH

/**
 * A convenience function; the primary interface to this package.  Loads up the
 * indicated BSP file, and returns the root of a scene graph.  Returns NULL if
 * the file cannot be read for some reason.
 */
EXPCL_PANDA_BSP PT(PandaNode)
load_BSP_file(const Filename &filename);

/**
 * Loads up the indicated BSP file but does not create any geometry from it.
 * It simply reads in the lump data.  Returns the BSPData object on success, or
 * NULL if the file cannot be read for some reason.
 */
//EXPCL_PANDA_BSP PT(BSPData)
//load_BSP_data(const Filename &filename);

END_PUBLISH

#endif // LOAD_BSP_FILE_H
