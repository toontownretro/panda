/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeBSP.cxx
 * @author lachbr
 * @date 2021-01-02
 */

#include "loaderFileTypeBSP.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "load_bsp_file.h"

TypeHandle LoaderFileTypeBSP::_type_handle;

/**
 *
 */
std::string LoaderFileTypeBSP::
get_name() const {
  return "BSP";
}

/**
 *
 */
std::string LoaderFileTypeBSP::
get_extension() const {
  return "bsp";
}

/**
 * Searches for the indicated filename on whatever paths are appropriate to
 * this file type, and updates it if it is found.
 */
void LoaderFileTypeBSP::
resolve_filename(Filename &path) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeBSP::
load_file(const Filename &path, const LoaderOptions &,
          BamCacheRecord *record) const {

  // Return the resulting node
  return load_BSP_file(path);
}
