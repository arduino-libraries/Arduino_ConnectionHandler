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

EthernetConnectionHandler::EthernetConnectionHandler(uint8_t * mac, bool const keep_alive)
: ConnectionHandler{keep_alive}
, _mac{mac}
{

}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState EthernetConnectionHandler::update_handleInit()
{
  if (Ethernet.begin(const_cast<uint8_t *>(_mac)) == 0) {
    Debug.print(DBG_ERROR, F("Failed to configure Ethernet using DHCP"));

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Debug.print(DBG_ERROR, F("Error, ethernet shield was not found."));
        return NetworkConnectionState::ERROR;
    }

    if (Ethernet.linkStatus() == LinkOFF) {
        Debug.print(DBG_ERROR, F("Error, ethernet cable is not connected."));
        return NetworkConnectionState::ERROR;
    }

    return NetworkConnectionState::ERROR;
  }

  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState EthernetConnectionHandler::update_handleConnecting()
{
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState EthernetConnectionHandler::update_handleConnected()
{
  if (Ethernet.linkStatus() == LinkON)
    return NetworkConnectionState::CONNECTED;
  else
    return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState EthernetConnectionHandler::update_handleDisconnecting()
{
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState EthernetConnectionHandler::update_handleDisconnected()
{
  return NetworkConnectionState::INIT;
}

#endif /* #ifdef BOARD_HAS_ETHERNET */
