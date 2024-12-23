/*
   This file is part of ArduinoIoTCloud.
   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)
   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html
   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_ETHERNET /* Only compile if the board has ethernet */
#include "EthernetConnectionHandler.h"
#include <Udp.h>

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
  memset(_settings.eth.ip.dword, 0, sizeof(_settings.eth.ip.dword));
  memset(_settings.eth.dns.dword, 0, sizeof(_settings.eth.dns.dword));
  memset(_settings.eth.gateway.dword, 0, sizeof(_settings.eth.gateway.dword));
  memset(_settings.eth.netmask.dword, 0, sizeof(_settings.eth.netmask.dword));
  _settings.eth.timeout = timeout;
  _settings.eth.response_timeout = responseTimeout;
}

EthernetConnectionHandler::EthernetConnectionHandler(
  const IPAddress ip, const IPAddress dns, const IPAddress gateway, const IPAddress netmask,
  unsigned long const timeout, unsigned long const responseTimeout, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::ETHERNET}
{
  fromIPAddress(ip, _settings.eth.ip);
  fromIPAddress(dns, _settings.eth.dns);
  fromIPAddress(gateway, _settings.eth.gateway);
  fromIPAddress(netmask, _settings.eth.netmask);
  _settings.eth.timeout = timeout;
  _settings.eth.response_timeout = responseTimeout;
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState EthernetConnectionHandler::update_handleInit()
{
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Debug.print(DBG_ERROR, F("Error, ethernet shield was not found."));
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

      Debug.print(DBG_ERROR, F("Failed to configure Ethernet, check cable connection"));
      Debug.print(DBG_VERBOSE, "timeout: %d, response timeout: %d",
        _settings.eth.timeout, _settings.eth.response_timeout);
      return NetworkConnectionState::INIT;
    }
  // An ip address is not provided -> dhcp configuration
  } else {
    if (Ethernet.begin(nullptr, _settings.eth.timeout, _settings.eth.response_timeout) == 0) {
      Debug.print(DBG_ERROR, F("Waiting Ethernet configuration from DHCP server, check cable connection"));
      Debug.print(DBG_VERBOSE, "timeout: %d, response timeout: %d",
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
  // Request time from NTP server for testing internet connection
  UDP &udp = getUDP();
  udp.begin(4001);
  uint8_t ntp_packet_buf[48] = {0};
  
  ntp_packet_buf[0]  = 0b11100011;
  ntp_packet_buf[1]  = 0;
  ntp_packet_buf[2]  = 6;
  ntp_packet_buf[3]  = 0xEC;
  ntp_packet_buf[12] = 49;
  ntp_packet_buf[13] = 0x4E;
  ntp_packet_buf[14] = 49;
  ntp_packet_buf[15] = 52;
  
  udp.beginPacket("time.arduino.cc", 123);
  udp.write(ntp_packet_buf, 48);
  udp.endPacket();

  bool is_timeout = false;
  unsigned long const start = millis();
  do
  {
    is_timeout = (millis() - start) >= 1000;
  } while(!is_timeout && !udp.parsePacket());

  if(is_timeout) {
    udp.stop();
    Debug.print(DBG_ERROR, F("Internet check failed"));
    Debug.print(DBG_INFO, F("Retrying in  \"%d\" milliseconds"), CHECK_INTERVAL_TABLE[static_cast<unsigned int>(NetworkConnectionState::CONNECTING)]);
    return NetworkConnectionState::CONNECTING;
  }
  
  udp.read(ntp_packet_buf, 48);
  udp.stop();
  Debug.print(DBG_INFO, F("Connected to Internet"));
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState EthernetConnectionHandler::update_handleConnected()
{
  if (Ethernet.linkStatus() == LinkOFF) {
    Debug.print(DBG_ERROR, F("Ethernet link OFF, connection lost."));
    if (_keep_alive)
    {
      Debug.print(DBG_ERROR, F("Attempting reconnection"));
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
  if (_keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_ETHERNET */
