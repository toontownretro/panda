/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamNetworkSystem.cxx
 * @author lachbr
 * @date 2020-03-29
 */

#include "steamNetworkSystem.h"
#include "steamNetworkMessage.h"
#include "steamNetworkConnectionInfo.h"

SteamNetworkSystem *SteamNetworkSystem::_global_ptr = nullptr;

/**
 *
 */
SteamNetworkSystem::
SteamNetworkSystem() {
  _client_connection = 0;
  SteamNetworkingErrMsg err_msg;
  _interface = GameNetworkingSockets_CreateInstance(nullptr, err_msg);
  if (!_interface) {
    steamnet_cat.error()
      << "Unable to initialize SteamNetworkingSockets! (" << std::string(err_msg) << ")\n";
    return;
  }
}

/**
 *
 */
SteamNetworkSystem::
~SteamNetworkSystem() {
  if (_interface) {
    GameNetworkingSockets_KillInstance(_interface);
    _interface = nullptr;
  }
}

/**
 *
 */
SteamNetworkConnectionHandle SteamNetworkSystem::
connect_by_IP_address(const NetAddress &addr) {
  SteamNetworkingIPAddr steam_addr;
  steam_addr.Clear();
  steam_addr.ParseString(addr.get_addr().get_ip_port().c_str());
  SteamNetworkConnectionHandle handle = _interface->ConnectByIPAddress(steam_addr, 0, nullptr);
  _client_connection = handle;
  _is_client = true;

  return handle;
}

/**
 *
 */
bool SteamNetworkSystem::
get_connection_info(SteamNetworkConnectionHandle conn, SteamNetworkConnectionInfo *info) {
  SteamNetConnectionInfo_t s_info;
  if (!_interface->GetConnectionInfo(conn, &s_info)) {
    return false;
  }

  info->set_listen_socket(s_info.m_hListenSocket);
  info->set_state((SteamNetworkSystem::NetworkConnectionState)s_info.m_eState);
  info->set_end_reason(s_info.m_eEndReason);

  NetAddress addr;
  char pBuf[100];
  s_info.m_addrRemote.ToString(pBuf, 100, false);
  if (!addr.set_host(pBuf, s_info.m_addrRemote.m_port)) {
    steamnet_cat.error()
      << "Unable to set host on NetAddress in get_connection_info()\n";
    return false;
  }
  info->set_net_address(addr);

  return true;
}

/**
 *
 */
void SteamNetworkSystem::
send_datagram(SteamNetworkConnectionHandle conn, const Datagram &dg,
              SteamNetworkSystem::NetworkSendFlags flags) {
  _interface->SendMessageToConnection(
    conn, dg.get_data(),
    dg.get_length(), flags, nullptr);
}

/**
 *
 */
void SteamNetworkSystem::
send_datagram(const Datagram &dg, SteamNetworkSystem::NetworkSendFlags flags) {
  send_datagram(_client_connection, dg, flags);
}

/**
 *
 */
void SteamNetworkSystem::
close_connection(SteamNetworkConnectionHandle conn) {
  _interface->CloseConnection(conn, 0, nullptr, false);
  if (_is_client && conn == _client_connection) {
    _client_connection = 0;
  }
}

/**
 *
 */
void SteamNetworkSystem::
run_callbacks() {
  _interface->RunCallbacks(this);
}

/**
 *
 */
bool SteamNetworkSystem::
accept_connection(SteamNetworkConnectionHandle conn) {
  return _interface->AcceptConnection(conn) == k_EResultOK;
}

/**
 *
 */
bool SteamNetworkSystem::
set_connection_poll_group(SteamNetworkConnectionHandle conn, SteamNetworkPollGroupHandle poll_group) {
  return _interface->SetConnectionPollGroup(conn, poll_group);
}

/**
 *
 */
bool SteamNetworkSystem::
receive_message_on_connection(SteamNetworkConnectionHandle conn, SteamNetworkMessage &msg) {
  ISteamNetworkingMessage *in_msg = nullptr;
  int msg_count = _interface->ReceiveMessagesOnConnection(conn, &in_msg, 1);
  if (!in_msg || msg_count != 1) {
    return false;
  }

  msg.set_datagram(Datagram(in_msg->m_pData, in_msg->m_cbSize));
  msg.set_connection(in_msg->GetConnection());

  return true;
}

/**
 *
 */
bool SteamNetworkSystem::
receive_message_on_poll_group(SteamNetworkPollGroupHandle poll_group, SteamNetworkMessage &msg) {
  ISteamNetworkingMessage *in_msg = nullptr;
  int msg_count = _interface->ReceiveMessagesOnPollGroup(poll_group, &in_msg, 1);
  if (!in_msg || msg_count != 1) {
    return false;
  }

  msg.set_datagram(Datagram(in_msg->m_pData, in_msg->m_cbSize));
  msg.set_connection(in_msg->GetConnection());

  return true;
}

/**
 *
 */
SteamNetworkPollGroupHandle SteamNetworkSystem::
create_poll_group() {
  return _interface->CreatePollGroup();
}

/**
 *
 */
SteamNetworkListenSocketHandle SteamNetworkSystem::
create_listen_socket(int port) {
  SteamNetworkingIPAddr steam_addr;
  steam_addr.Clear();
  steam_addr.m_port = port;

  return _interface->CreateListenSocketIP(steam_addr, 0, nullptr);
}

/**
 *
 */
void SteamNetworkSystem::
OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pCallback) {
  PT(SteamNetworkEvent) event = new SteamNetworkEvent(
    (SteamNetworkConnectionHandle)pCallback->m_hConn,
    (NetworkConnectionState)pCallback->m_eOldState,
    (NetworkConnectionState)pCallback->m_info.m_eState);
  _events.push_back(event);
}
