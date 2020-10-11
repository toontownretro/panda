/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkEvent.h
 * @author lachbr
 * @date 2020-10-11
 */

#ifndef STEAMNETWORKEVENT_H
#define STEAMNETWORKEVENT_H

#include "config_steamnet.h"
#include "referenceCount.h"

/**
 * A single callback event for a connection state change.
 */
class EXPCL_PANDA_STEAMNET SteamNetworkEvent : public ReferenceCount {
public:
  INLINE SteamNetworkEvent(SteamNetworkConnectionHandle conn,
                    SteamNetworkEnums::NetworkConnectionState old_state,
                    SteamNetworkEnums::NetworkConnectionState new_state);

PUBLISHED:
  INLINE SteamNetworkConnectionHandle get_connection() const;
  MAKE_PROPERTY(connection, get_connection);

  INLINE SteamNetworkEnums::NetworkConnectionState get_old_state() const;
  MAKE_PROPERTY(old_state, get_old_state);

  INLINE SteamNetworkEnums::NetworkConnectionState get_state() const;
  MAKE_PROPERTY(state, get_state);

private:
  SteamNetworkConnectionHandle _connection;
  SteamNetworkEnums::NetworkConnectionState _old_state;
  SteamNetworkEnums::NetworkConnectionState _state;
};

#include "steamNetworkEvent.I"

#endif // STEAMNETWORKEVENT_H
