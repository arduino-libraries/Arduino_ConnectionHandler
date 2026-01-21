/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_ETHERNET /* Only compile if the board has ethernet */
#include "EthernetConnectionHandler.h"

/******************************************************************************
  CTOR/DTOR
 ******************************************************************************/

static inline void fromIPAddress(const IPAddress src, models::ip_addr& dst) {
  if(src.type() == IPv4) {
    dst.dword[IPADDRESS_V4_DWORD_INDEX] = (uint32_t)src;
  } else if(src.type() == IPv6) {
    for(uint8_t i=0; i<sizeof(dst.bytes); i++) {
      dst.bytes[i] = src[i];
    }
  }
}

EthernetConnectionHandler::EthernetConnectionHandler(
  unsigned long const timeout,
  unsigned long const responseTimeout,
  bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::ETHERNET}
{
  _settings.type = NetworkAdapter::ETHERNET;
  memset(_settings.eth.ip.dword, 0, sizeof(_settings.eth.ip.dword));
  memset(_settings.eth.dns.dword, 0, sizeof(_settings.eth.dns.dword));
  memset(_settings.eth.gateway.dword, 0, sizeof(_settings.eth.gateway.dword));
  memset(_settings.eth.netmask.dword, 0, sizeof(_settings.eth.netmask.dword));
  _settings.eth.timeout = timeout;
  _settings.eth.response_timeout = responseTimeout;

  _flags.settings_provided = true;
}

EthernetConnectionHandler::EthernetConnectionHandler(
  const IPAddress ip, const IPAddress dns, const IPAddress gateway, const IPAddress netmask,
  unsigned long const timeout, unsigned long const responseTimeout, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::ETHERNET}
{
  _settings.type = NetworkAdapter::ETHERNET;
  fromIPAddress(ip, _settings.eth.ip);
  fromIPAddress(dns, _settings.eth.dns);
  fromIPAddress(gateway, _settings.eth.gateway);
  fromIPAddress(netmask, _settings.eth.netmask);
  _settings.eth.timeout = timeout;
  _settings.eth.response_timeout = responseTimeout;

  _flags.settings_provided = true;
}

/******************************************************************************
  PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState EthernetConnectionHandler::update_handleInit()
{
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    DEBUG_ERROR(F("Error, ethernet shield was not found."));
    return NetworkConnectionState::ERROR;
  }
  IPAddress ip(_settings.eth.ip.type, _settings.eth.ip.bytes);

  // An ip address is provided -> static ip configuration
  if (ip != INADDR_NONE) {
    if (Ethernet.begin(nullptr, ip,
        IPAddress(_settings.eth.dns.type, _settings.eth.dns.bytes),
        IPAddress(_settings.eth.gateway.type, _settings.eth.gateway.bytes),
        IPAddress(_settings.eth.netmask.type, _settings.eth.netmask.bytes),
        _settings.eth.timeout,
        _settings.eth.response_timeout) == 0) {

      DEBUG_ERROR(F("Failed to configure Ethernet, check cable connection"));
      DEBUG_VERBOSE("timeout: %d, response timeout: %d",
        _settings.eth.timeout, _settings.eth.response_timeout);
      return NetworkConnectionState::INIT;
    }
  // An ip address is not provided -> dhcp configuration
  } else {
    if (Ethernet.begin(nullptr, _settings.eth.timeout, _settings.eth.response_timeout) == 0) {
      DEBUG_ERROR(F("Waiting Ethernet configuration from DHCP server, check cable connection"));
      DEBUG_VERBOSE("timeout: %d, response timeout: %d",
        _settings.eth.timeout, _settings.eth.response_timeout);

      return NetworkConnectionState::INIT;
    }
  }

  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState EthernetConnectionHandler::update_handleConnecting()
{
  if (Ethernet.linkStatus() == LinkOFF) {
    return NetworkConnectionState::INIT;
  }

  if (!_flags.check_internet_availability) {
    return NetworkConnectionState::CONNECTED;
  }

  int ping_result = Ethernet.ping("time.arduino.cc");
  DEBUG_INFO(F("Ethernet.ping(): %d"), ping_result);
  if (ping_result < 0)
  {
    DEBUG_ERROR(F("Internet check failed"));
    DEBUG_INFO(F("Retrying in  \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    DEBUG_INFO(F("Connected to Internet"));
    return NetworkConnectionState::CONNECTED;
  }

}

NetworkConnectionState EthernetConnectionHandler::update_handleConnected()
{
  if (Ethernet.linkStatus() == LinkOFF) {
    DEBUG_ERROR(F("Ethernet link OFF, connection lost."));
    if (_flags.keep_alive)
    {
      DEBUG_ERROR(F("Attempting reconnection"));
    }
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState EthernetConnectionHandler::update_handleDisconnecting()
{
  Ethernet.disconnect();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState EthernetConnectionHandler::update_handleDisconnected()
{
  if (_flags.keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_ETHERNET */
