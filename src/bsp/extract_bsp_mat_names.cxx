/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file extract_bsp_mat_names.cxx
 * @author brian
 * @date 2021-07-04
 */

/**
 * This program reads a BSP file and outputs the names of all
 * referenced materials/textures to standard output.
 */

#include "virtualFileSystem.h"
#include "bspData.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "vector_uchar.h"
#include "filename.h"

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "You must specify a BSP filename\n";
    return 1;
  }

  Filename filename(argv[1]);
  filename.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  vector_uchar result;
  if (!vfs->read_file(filename, result, true)) {
    std::cerr << "Could not read BSP file\n";
    return 1;
  }
  Datagram dg(result);
  DatagramIterator dgi(dg);

  PT(BSPData) data = new BSPData;
  if (!data->read_datagram(dgi)) {
    std::cerr << "Could not read BSP data\n";
    return 1;
  }

  for (size_t i = 0; i < data->tex_data_string_table.size(); i++) {
    std::cout << data->get_string(i) << "\n";
  }

  return 0;
}
