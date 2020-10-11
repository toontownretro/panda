/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkEnums.h
 * @author lachbr
 * @date 2020-10-11
 */

#ifndef STEAMNETWORKENUMS_H
#define STEAMNETWORKENUMS_H

#include "config_steamnet.h"

/**
 * Various enumerations used throughout the SteamNetworkingSockets
 * implementation.
 */
class SteamNetworkEnums {
PUBLISHED:
  // Copy of k_nSteamNetworkingSend
  enum NetworkSendFlags {
    NSF_unreliable = 0,
    NSF_no_nagle = 1,
    NSF_unreliable_no_nagle = NSF_unreliable | NSF_no_nagle,
    NSF_no_delay = 4,
    NSF_unreliable_no_delay = NSF_unreliable | NSF_no_delay | NSF_no_nagle,
    NSF_reliable = 8,
    NSF_reliable_no_nagle = NSF_reliable | NSF_no_nagle,
    NSF_use_current_thread = 16,
  };

  // Copy of ESteamNetworkingConnectionState
  enum NetworkConnectionState {
    NCS_none = 0,
    NCS_connecting = 1,
    NCS_finding_route = 2,
    NCS_connected = 3,
    NCS_closed_by_peer = 4,
    NCS_problem_detected_locally = 5,
  };
};

#endif // STEAMNETWORKENUMS_H
