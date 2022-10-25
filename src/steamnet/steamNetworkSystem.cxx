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
#include "pStatCollector.h"

#ifndef CPPPARSER
#include "steam/isteamnetworkingutils.h"
#endif

IMPLEMENT_CLASS(SteamNetworkSystem);

static PStatCollector copy_datagram_coll("App:SteamNetworking:CopyMessageDatagram");

SteamNetworkSystem *SteamNetworkSystem::_global_ptr = nullptr;

static SteamNetworkSystem *_callback_instance = nullptr;

/**
 *
 */
static void *
sns_malloc(size_t s) {
  return SteamNetworkSystem::get_class_type().allocate_array(s);
}

/**
 *
 */
static void
sns_free(void *p) {
  SteamNetworkSystem::get_class_type().deallocate_array(p);
}

/**
 *
 */
static void *
sns_realloc(void *p, size_t s) {
  return SteamNetworkSystem::get_class_type().reallocate_array(p, s);
}

/**
 *
 */
SteamNetworkSystem::
SteamNetworkSystem() {
  _client_connection = 0;

  //SteamNetworkingSockets_SetCustomMemoryAllocator(sns_malloc, sns_free, sns_realloc);

  SteamNetworkingErrMsg err_msg;
  if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
    steamnet_cat.error()
      << "Unable to initialize SteamNetworkingSockets! (" << std::string(err_msg) << ")\n";
  }

  _interface = SteamNetworkingSockets();
}

/**
 *
 */
SteamNetworkSystem::
~SteamNetworkSystem() {
  GameNetworkingSockets_Kill();
}

/**
 *
 */
SteamNetworkConnectionHandle SteamNetworkSystem::
connect_by_IP_address(const NetAddress &addr) {
  SteamNetworkingIPAddr steam_addr;
  steam_addr.Clear();

  SteamNetworkingIPAddr_ParseString(
    &steam_addr, addr.get_addr().get_ip_port().c_str());

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
             (void *)OnSteamNetConnectionStatusChanged);

  SteamNetworkConnectionHandle handle = _interface->ConnectByIPAddress(steam_addr, 1, &opt);
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
  SteamNetworkingIPAddr_ToString(&s_info.m_addrRemote, pBuf, 100, false);
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
  _callback_instance = this;
  _interface->RunCallbacks();
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

  copy_datagram_coll.start();
  msg.set_datagram(Datagram(in_msg->m_pData, in_msg->m_cbSize));
  copy_datagram_coll.stop();

  msg.set_connection(in_msg->GetConnection());

  in_msg->Release();

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

  copy_datagram_coll.start();
  msg.set_datagram(Datagram(in_msg->m_pData, in_msg->m_cbSize));
  copy_datagram_coll.stop();

  msg.set_connection(in_msg->GetConnection());

  in_msg->Release();

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

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
             (void *)OnSteamNetConnectionStatusChanged);

  return _interface->CreateListenSocketIP(steam_addr, 1, &opt);
}

/**
 *
 */
void SteamNetworkSystem::
OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pCallback) {
  nassertv(_callback_instance != nullptr);

  PT(SteamNetworkEvent) event = new SteamNetworkEvent(
    (SteamNetworkConnectionHandle)pCallback->m_hConn,
    (NetworkConnectionState)pCallback->m_eOldState,
    (NetworkConnectionState)pCallback->m_info.m_eState);
  _callback_instance->_events.push_back(event);
}
