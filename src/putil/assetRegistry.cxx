/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file assetRegistry.cxx
 * @author lachbr
 * @date 2021-09-14
 */

#include "assetRegistry.h"

AssetRegistry *AssetRegistry::_global_ptr = nullptr;

/**
 * Registers a new asset type.
 */
void AssetRegistry::
register_asset_type(AssetBase *type) {
  auto it = std::find(_types.begin(), _types.end(), type);
  if (it != _types.end()) {
    // Already registered.
    return;
  }

  _types.push_back(type);
}

/**
 * Loads an asset from the given filename.  Uses the filename extension to load
 * the correct asset type.
 */
PT(AssetBase) AssetRegistry::
load(const Filename &filename, const DSearchPath &search_path) {
  PT(AssetBase) asset;

  // Find the correct asset type.
  for (AssetBase *type : _types) {
    if (type->get_source_extension() == filename.get_extension()) {
      asset = type->make_new();
      break;
    }
  }

  if (asset == nullptr) {
    util_cat.error()
      << "Unkown asset type extension: " << filename.get_extension() << "\n";
    return nullptr;
  }

  if (!asset->load(filename, search_path)) {
    util_cat.error()
      << "Failed to load " << asset->get_name() << " asset from " << filename << "\n";
    return nullptr;
  }

  return asset;
}

/**
 *
 */
AssetRegistry *AssetRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new AssetRegistry;
  }

  return _global_ptr;
}
