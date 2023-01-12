/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkSystem.I
 * @author brian
 * @date 2020-10-11
 */

/**
 *
 */
INLINE PT(SteamNetworkEvent) SteamNetworkSystem::
get_next_event() {
  if (_events.size() == 0) {
    return nullptr;
  }

  PT(SteamNetworkEvent) event = _events.front();
  _events.pop_front();
  return event;
}

/**
 *
 */
INLINE SteamNetworkSystem *SteamNetworkSystem::
get_global_ptr() {
  if (!_global_ptr) {
    _global_ptr = new SteamNetworkSystem;
  }

  return _global_ptr;
}
