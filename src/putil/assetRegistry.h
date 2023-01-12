/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file assetRegistry.h
 * @author brian
 * @date 2021-09-14
 */

#ifndef ASSETREGISTRY_H
#define ASSETREGISTRY_H

#include "pandabase.h"
#include "assetBase.h"
#include "pvector.h"
#include "pointerTo.h"
#include "dSearchPath.h"
#include "filename.h"
#include "config_putil.h"

/**
 * Manages the global registry of asset/resource types.
 */
class EXPCL_PANDA_PUTIL AssetRegistry {
public:
  void register_asset_type(AssetBase *type);
  INLINE int get_num_asset_types() const;
  INLINE AssetBase *get_asset_type(int n) const;

  PT(AssetBase) load(const Filename &filename, const DSearchPath &search_path = get_model_path());

  static AssetRegistry *get_global_ptr();

private:
  typedef pvector<AssetBase *> AssetTypes;
  AssetTypes _types;

  static AssetRegistry *_global_ptr;
};

#include "assetRegistry.I"

#endif // ASSETREGISTRY_H
