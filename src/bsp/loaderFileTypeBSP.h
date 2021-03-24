/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeBSP.h
 * @author lachbr
 * @date 2021-01-02
 */

#ifndef LOADERFILETYPEBSP_H
#define LOADERFILETYPEBSP_H

#include "config_bsp.h"
#include "loaderFileType.h"

/**
 * This defines the Loader interface to read BSP files.
 */
class EXPCL_PANDA_BSP LoaderFileTypeBSP : public LoaderFileType {
public:
  virtual std::string get_name() const;
  virtual std::string get_extension() const;

  virtual void resolve_filename(Filename &path) const;
  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options,
                                  BamCacheRecord *record) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypeBSP",
                  LoaderFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LOADERFILETYPEBSP_H
