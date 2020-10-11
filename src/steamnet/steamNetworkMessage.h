/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkMessage.h
 * @author lachbr
 * @date 2020-10-11
 */

#ifndef STEAMNETWORKMESSAGE_H
#define STEAMNETWORKMESSAGE_H

#include "config_steamnet.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "steamnet_includes.h"

/**
 * A single incoming message received on the SteamNetworkingSockets layer.
 */
class EXPCL_PANDA_STEAMNET SteamNetworkMessage {
PUBLISHED:
  INLINE SteamNetworkMessage();

  INLINE void set_datagram(const Datagram &dg);
  INLINE const Datagram &get_datagram() const;
  MAKE_PROPERTY(dg, get_datagram);

  INLINE void set_connection(SteamNetworkConnectionHandle connection);
  INLINE SteamNetworkConnectionHandle get_connection() const;
  MAKE_PROPERTY(connection, get_connection);

  INLINE DatagramIterator &get_datagram_iterator();
  MAKE_PROPERTY(dgi, get_datagram_iterator);

private:
  Datagram _dg;
  DatagramIterator _dgi;
  SteamNetworkConnectionHandle _connection;
};

#include "steamNetworkMessage.I"

#endif // STEAMNETWORKMESSAGE_H
