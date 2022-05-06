/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkMessage.I
 * @author lachbr
 * @date 2020-10-11
 */

/**
 *
 */
INLINE SteamNetworkMessage::
SteamNetworkMessage() {
  _connection = INVALID_STEAM_NETWORK_CONNECTION_HANDLE;
}

/**
 *
 */
INLINE void SteamNetworkMessage::
set_datagram(const Datagram &dg) {
  _dg = dg;
  _dgi = DatagramIterator(_dg);
}

/**
 *
 */
INLINE void SteamNetworkMessage::
set_datagram(Datagram &&dg) {
  _dg = std::move(dg);
  _dgi = DatagramIterator(_dg);
}

/**
 *
 */
INLINE const Datagram &SteamNetworkMessage::
get_datagram() const {
  return _dg;
}

/**
 *
 */
INLINE void SteamNetworkMessage::
set_connection(SteamNetworkConnectionHandle connection) {
  _connection = connection;
}

/**
 *
 */
INLINE SteamNetworkConnectionHandle SteamNetworkMessage::
get_connection() const {
  return _connection;
}

/**
 *
 */
INLINE DatagramIterator &SteamNetworkMessage::
get_datagram_iterator() {
  return _dgi;
}
