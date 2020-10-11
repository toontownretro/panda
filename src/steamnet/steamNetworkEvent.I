/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkEvent.I
 * @author lachbr
 * @date 2020-10-11
 */

/**
 *
 */
INLINE SteamNetworkEvent::
SteamNetworkEvent(SteamNetworkConnectionHandle conn,
                  SteamNetworkEnums::NetworkConnectionState old_state,
                  SteamNetworkEnums::NetworkConnectionState new_state)
{
  _connection = conn;
  _old_state = old_state;
  _state = new_state;
}

/**
 *
 */
INLINE SteamNetworkConnectionHandle SteamNetworkEvent::
get_connection() const {
  return _connection;
}

/**
 *
 */
INLINE SteamNetworkEnums::NetworkConnectionState SteamNetworkEvent::
get_old_state() const {
  return _old_state;
}

/**
 *
 */
INLINE SteamNetworkEnums::NetworkConnectionState SteamNetworkEvent::
get_state() const {
  return _state;
}
