/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypePmdl.cxx
 * @author brian
 * @date 2021-02-14
 */

#include "loaderFileTypePmdl.h"
#include "load_egg_file.h"

TypeHandle LoaderFileTypePMDL::_type_handle;

/**
 *
 */
LoaderFileTypePMDL::
LoaderFileTypePMDL() {
}

/**
 *
 */
std::string LoaderFileTypePMDL::
get_name() const {
  return "Panda Model";
}

/**
 *
 */
std::string LoaderFileTypePMDL::
get_extension() const {
  return "pmdl";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypePMDL::
supports_compressed() const {
  return true;
}

/**
 * Returns true if the file type can be used to load files, and load_file() is
 * supported.  Returns false if load_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypePMDL::
supports_load() const {
  return true;
}

/**
 * Returns true if the file type can be used to save files, and save_file() is
 * supported.  Returns false if save_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypePMDL::
supports_save() const {
  return false;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypePMDL::
load_file(const Filename &path, const LoaderOptions &,
          BamCacheRecord *record) const {
  PT(PandaNode) result = load_pmdl_file(path, CS_default);
  return result;
}
