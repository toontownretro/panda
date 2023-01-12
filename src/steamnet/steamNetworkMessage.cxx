/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkMessage.cxx
 * @author brian
 * @date 2022-10-04
 */

#include "steamNetworkMessage.h"
#include "config_steamnet.h"

/**
 *
 */
SteamNetworkMessage::
~SteamNetworkMessage() {
#ifndef NDEBUG
  if (steamnet_cat.is_debug()) {
    steamnet_cat.debug()
      << "Destruct SteamNetworkMessage " << this << "\n";
  }
#endif
}
