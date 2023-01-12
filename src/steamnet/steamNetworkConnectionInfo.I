/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkConnectionInfo.I
 * @author brian
 * @date 2020-10-11
 */

/**
 *
 */
INLINE SteamNetworkConnectionInfo::
SteamNetworkConnectionInfo()
{
  _listen_socket = INVALID_STEAM_NETWORK_LISTEN_SOCKET_HANDLE;
  _state = SteamNetworkEnums::NCS_none;
  _end_reason = 0;
}

/**
 *
 */
INLINE void SteamNetworkConnectionInfo::
set_listen_socket(SteamNetworkListenSocketHandle socket) {
  _listen_socket = socket;
}

/**
 *
 */
INLINE SteamNetworkListenSocketHandle SteamNetworkConnectionInfo::
get_listen_socket() const {
  return _listen_socket;
}

/**
 *
 */
INLINE void SteamNetworkConnectionInfo::
set_net_address(const NetAddress &addr) {
  _net_address = addr;
}

/**
 *
 */
INLINE const NetAddress &SteamNetworkConnectionInfo::
get_net_address() const {
  return _net_address;
}

/**
 *
 */
INLINE void SteamNetworkConnectionInfo::
set_state(SteamNetworkEnums::NetworkConnectionState state) {
  _state = state;
}

/**
 *
 */
INLINE SteamNetworkEnums::NetworkConnectionState SteamNetworkConnectionInfo::
get_state() const {
  return _state;
}

/**
 *
 */
INLINE void SteamNetworkConnectionInfo::
set_end_reason(int reason) {
  _end_reason = reason;
}

/**
 *
 */
INLINE int SteamNetworkConnectionInfo::
get_end_reason() const {
  return _state;
}
