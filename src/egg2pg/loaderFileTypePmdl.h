/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypePmdl.h
 * @author lachbr
 * @date 2021-02-14
 */

#ifndef LOADERFILETYPEPMDL_H
#define LOADERFILETYPEPMDL_H

#include "pandabase.h"

#include "loaderFileType.h"

class EXPCL_PANDA_EGG2PG LoaderFileTypePMDL : public LoaderFileType {
public:
  LoaderFileTypePMDL();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool supports_load() const;
  virtual bool supports_save() const;

  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &optoins,
                                  BamCacheRecord *record) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypePMDL",
                  LoaderFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LOADERFILETYPEPMDL_H
