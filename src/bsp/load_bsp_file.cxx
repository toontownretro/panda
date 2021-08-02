/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_bsp_file.cxx
 * @author lachbr
 * @date 2021-01-02
 */

#include "load_bsp_file.h"
#include "pandaNode.h"
#include "bspData.h"
#include "virtualFileSystem.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bspLoader.h"
#include "datagramInputFile.h"

/**
 * A convenience function; the primary interface to this package.  Loads up the
 * indicated BSP file, and returns the root of a scene graph.  Returns NULL if
 * the file cannot be read for some reason.
 */
PT(PandaNode)
load_BSP_file(const Filename &filename) {
  Filename bsp_filename = filename;
  bsp_filename.set_binary();
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  PT(VirtualFile) vfile = vfs->get_file(bsp_filename);
  if (vfile == nullptr) {
    return nullptr;
  }

  vector_uchar bytes;
  if (!vfile->read_file(bytes, true)) {
    return nullptr;
  }

  Datagram dg(bytes);
  DatagramIterator dgi(dg);
  PT(BSPData) data = new BSPData;
  if (!data->read_datagram(dgi)) {
    return nullptr;
  }

  BSPLoader loader(data);
  return loader.load();
}
