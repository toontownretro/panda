/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkConnectionInfo.h
 * @author brian
 * @date 2020-10-11
 */

#ifndef STEAMNETWORKCONNECTIONINFO_H
#define STEAMNETWORKCONNECTIONINFO_H

#include "config_steamnet.h"
#include "netAddress.h"
#include "steamnet_includes.h"
#include "steamNetworkEnums.h"

/**
 * Information about a single SteamNetworkingSockets connection.
 */
class EXPCL_PANDA_STEAMNET SteamNetworkConnectionInfo {
PUBLISHED:
  INLINE SteamNetworkConnectionInfo();
  virtual ~SteamNetworkConnectionInfo() = default;

  INLINE void set_listen_socket(SteamNetworkListenSocketHandle socket);
  INLINE SteamNetworkListenSocketHandle get_listen_socket() const;
  MAKE_PROPERTY(listen_socket, get_listen_socket);

  INLINE void set_net_address(const NetAddress &addr);
  INLINE const NetAddress &get_net_address() const;
  MAKE_PROPERTY(net_address, get_net_address);

  INLINE void set_state(SteamNetworkEnums::NetworkConnectionState state);
  INLINE SteamNetworkEnums::NetworkConnectionState get_state() const;
  MAKE_PROPERTY(state, get_state);

  INLINE void set_end_reason(int reason);
  INLINE int get_end_reason() const;
  MAKE_PROPERTY(end_reason, get_end_reason);

private:
  SteamNetworkListenSocketHandle _listen_socket;
  NetAddress _net_address;
  SteamNetworkEnums::NetworkConnectionState _state;
  int _end_reason;
};

#include "steamNetworkConnectionInfo.I"

#endif // STEAMNETWORKCONNECTIONINFO_H
