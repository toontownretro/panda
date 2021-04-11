/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlLoader.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLLOADER_H
#define PMDLLOADER_H

#include "pandabase.h"
#include "pointerTo.h"

class PMDLData;
class PandaNode;

/**
 * This class converts a high-level model description from a .pmdl file into a
 * scene graph suitable for rendering or saving to a Bam file.
 */
class EXPCL_PANDA_EGG2PG PMDLLoader {
PUBLISHED:
  PMDLLoader(PMDLData *data);

  void build_graph();

public:
  PT(PMDLData) _data;
  PT(PandaNode) _root;
};

#include "pmdlLoader.I"

#endif // PMDLLOADER_H
