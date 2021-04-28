/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodSoundCache.I
 * @author brian
 * @date 2021-04-28
 */

/**
 * Returns the global FMODSoundCache pointer.
 */
INLINE FMODSoundCache *FMODSoundCache::
get_global_ptr() {
  if (_ptr == nullptr) {
    _ptr = new FMODSoundCache;
  }

  return _ptr;
}

/**
 * Clears the cache of sounds.  From this point on, any sounds that were in the
 * cache will be released when the last reference to them goes away.
 */
INLINE void FMODSoundCache::
clear_sounds() {
  _sounds.clear();
}
