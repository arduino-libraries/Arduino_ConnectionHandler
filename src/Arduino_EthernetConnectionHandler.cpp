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

#include "Arduino_EthernetConnectionHandler.h"

#ifdef BOARD_HAS_ETHERNET /* Only compile if the board has ethernet */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

EthernetConnectionHandler::EthernetConnectionHandler(bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::ETHERNET}
,_ip{INADDR_NONE}
,_dns{INADDR_NONE}
,_gateway{INADDR_NONE}
,_netmask{INADDR_NONE}
{

}

EthernetConnectionHandler::EthernetConnectionHandler(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress netmask, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::ETHERNET}
,_ip{ip}
,_dns{dns}
,_gateway{gateway}
,_netmask{netmask}
{

}

EthernetConnectionHandler::EthernetConnectionHandler(const char * ip, const char * dns, const char * gateway, const char * netmask, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::ETHERNET}
,_ip{INADDR_NONE}
,_dns{INADDR_NONE}
,_gateway{INADDR_NONE}
,_netmask{INADDR_NONE}
{
  if(!_ip.fromString(ip)) {
    _ip = INADDR_NONE;
  }
  if(!_dns.fromString(dns)) {
    _dns = INADDR_NONE;
  }
  if(!_gateway.fromString(gateway)) {
    _gateway = INADDR_NONE;
  }
  if(!_netmask.fromString(netmask)) {
    _netmask = INADDR_NONE;
  }
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
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState EthernetConnectionHandler::update_handleConnecting()
{
  if (_ip != INADDR_NONE) {
    if (Ethernet.begin(nullptr, _ip, _dns, _gateway, _netmask, 15000, 4000) == 0) {
      Debug.print(DBG_ERROR, F("Failed to configure Ethernet, check cable connection"));
      return NetworkConnectionState::CONNECTING;
    }
  } else {
    if (Ethernet.begin(nullptr, 15000, 4000) == 0) {
      Debug.print(DBG_ERROR, F("Waiting Ethernet configuration from DHCP server, check cable connection"));
      return NetworkConnectionState::CONNECTING;
    }
  }

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
