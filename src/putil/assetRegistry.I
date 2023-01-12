/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file assetRegistry.I
 * @author brian
 * @date 2021-09-14
 */

/**
 * Returns the number of registered asset types.
 */
INLINE int AssetRegistry::
get_num_asset_types() const {
  return (int)_types.size();
}

/**
 * Returns the nth registered asset type.
 */
INLINE AssetBase *AssetRegistry::
get_asset_type(int n) const {
  nassertr(n >= 0 && n < (int)_types.size(), nullptr);
  return _types[n];
}
