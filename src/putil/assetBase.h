/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file assetBase.h
 * @author lachbr
 * @date 2021-09-14
 */

#ifndef ASSETBASE_H
#define ASSETBASE_H

#include "pandabase.h"
#include "vector_string.h"
#include "dSearchPath.h"
#include "filename.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"

/**
 * This is an abstract base class for a source asset/resource file that is part
 * of the model build pipeline.  It has the ability to list other files that the
 * asset depends on, so we can correctly list Makefile dependencies.
 */
class EXPCL_PANDA_PUTIL AssetBase : public TypedReferenceCount {
public:
  /**
   * Returns a vector of filenames that this asset depends on.  These will be
   * used as dependencies of the Makefile rule that builds this asset.
   */
  virtual void get_dependencies(vector_string &filenames)=0;

  /**
   * Returns the name of this type of asset, for instance a material or model.
   */
  virtual std::string get_name()=0;

  /**
   * Returns the filename extension, without a leading dot, of the source file
   * of this asset type.  For instance, source material files use the pmat
   * extension.
   */
  virtual std::string get_source_extension()=0;

  /**
   * Returns the filename extension, without a leading dot, of the built file
   * of this asset type.  For instance, built material files use the mto
   * extension.
   */
  virtual std::string get_built_extension()=0;

  /**
   * Loads a source file of this asset type from the indicated filename.
   *
   * Returns true on success, or false if the file could not be loaded for some
   * reason.
   */
  virtual bool load(const Filename &filename, const DSearchPath &search_path)=0;

  /**
   * Returns a new instance of this asset type.
   */
  virtual PT(AssetBase) make_new() const=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AssetBase",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "assetBase.I"

#endif // ASSETBASE_H
