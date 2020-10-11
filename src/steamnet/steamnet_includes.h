/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamnet_includes.h
 * @author lachbr
 * @date 2020-10-11
 */

#ifndef STEAMNET_INCLUDES_H
#define STEAMNET_INCLUDES_H

#ifndef CPPPARSER
#include <steam/steamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>
#else
class ISteamNetworkingSocketsCallbacks;
class ISteamNetworkingSockets;
#endif // CPPPARSER

typedef uint32_t SteamNetworkListenSocketHandle;
typedef uint32_t SteamNetworkPollGroupHandle;
typedef uint32_t SteamNetworkConnectionHandle;
static constexpr SteamNetworkConnectionHandle INVALID_STEAM_NETWORK_CONNECTION_HANDLE = 0U;
static constexpr SteamNetworkListenSocketHandle INVALID_STEAM_NETWORK_LISTEN_SOCKET_HANDLE = 0U;
static constexpr SteamNetworkPollGroupHandle INVALID_STEAM_NETWORK_POLL_GROUP_HANDLe = 0U;

#endif // STEAMNET_INCLUDES_H
