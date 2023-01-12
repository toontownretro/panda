/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_steamnet.cxx
 * @author brian
 * @date 2020-03-29
 */

#include "config_steamnet.h"
#include "steamNetworkSystem.h"

NotifyCategoryDef(steamnet, "");

ConfigureDef(config_steamnet);

ConfigureFn(config_steamnet) {
  init_libsteamnet();
}

void
init_libsteamnet() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  // Initialization of GameNetworkingSockets is deferred until
  // SteamNetworkSystem construction.

  initialized = true;

  SteamNetworkSystem::init_type();
}
