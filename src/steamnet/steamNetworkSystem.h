/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkSystem.h
 * @author lachbr
 * @date 2020-03-29
 */

#ifndef NETWORKSYSTEM_H
#define NETWORKSYSTEM_H

#include "config_steamnet.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "datagram.h"
#include "netAddress.h"
#include "datagramIterator.h"
#include "pdeque.h"
#include "steamnet_includes.h"
#include "steamNetworkEnums.h"
#include "steamNetworkEvent.h"
#include "typedObject.h"

class SteamNetworkConnectionInfo;
class SteamNetworkMessage;

/**
 * Main interface to the SteamNetworkingSockets implementation.
 */
class EXPCL_PANDA_STEAMNET SteamNetworkSystem : public SteamNetworkEnums, public TypedObject {
  DECLARE_CLASS(SteamNetworkSystem, TypedObject);

PUBLISHED:
  SteamNetworkSystem();
  virtual ~SteamNetworkSystem();

  SteamNetworkConnectionHandle connect_by_IP_address(const NetAddress &addr);
  bool get_connection_info(SteamNetworkConnectionHandle conn, SteamNetworkConnectionInfo *info);
  void send_datagram(SteamNetworkConnectionHandle conn, const Datagram &dg,
                     NetworkSendFlags flags = NSF_reliable_no_nagle);
  // Only valid for client connections. Sends a datagram to the server.
  void send_datagram(const Datagram &dg, NetworkSendFlags flags = NSF_reliable_no_nagle);
  void close_connection(SteamNetworkConnectionHandle conn);
  void run_callbacks();
  bool accept_connection(SteamNetworkConnectionHandle conn);
  bool set_connection_poll_group(SteamNetworkConnectionHandle conn, SteamNetworkPollGroupHandle poll_group);
  bool receive_message_on_connection(SteamNetworkConnectionHandle conn, SteamNetworkMessage &msg);
  bool receive_message_on_poll_group(SteamNetworkPollGroupHandle poll_group, SteamNetworkMessage &msg);
  SteamNetworkPollGroupHandle create_poll_group();
  SteamNetworkListenSocketHandle create_listen_socket(int port);

  INLINE PT(SteamNetworkEvent) get_next_event();

public:
  static void OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t *callback);

PUBLISHED:
  INLINE static SteamNetworkSystem *get_global_ptr();

private:
  ISteamNetworkingSockets *_interface;
  static SteamNetworkSystem *_global_ptr;

  // Connection to the server if we are a client.
  SteamNetworkConnectionHandle _client_connection;
  bool _is_client;

  pdeque<PT(SteamNetworkEvent)> _events;
};

#include "steamNetworkSystem.I"

#endif // NETWORKSYSTEM_H
