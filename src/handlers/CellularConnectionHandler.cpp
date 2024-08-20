/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "definitions/ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_CELLULAR /* Only compile if the board has Cellular */
#include "handlers/CellularConnectionHandler.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

CellularConnectionHandler::CellularConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::CELL}
, _pin(pin)
, _apn(apn)
, _login(login)
, _pass(pass)
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long CellularConnectionHandler::getTime()
{
  return _cellular.getCellularTime().getUNIXTimestamp();
}

UDP & CellularConnectionHandler::getUDP()
{
  Debug.print(DBG_ERROR, F("CellularConnectionHandler has no UDP support"));
  while(1) {};
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState CellularConnectionHandler::update_handleInit()
{
  _cellular.begin();
  _cellular.setDebugStream(Serial);
  if (String(_pin).length() > 0 && !_cellular.unlockSIM(_pin)) {
    Debug.print(DBG_ERROR, F("SIM not present or wrong PIN"));
    return NetworkConnectionState::ERROR;
  }
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState CellularConnectionHandler::update_handleConnecting()
{
  if (!_cellular.connect(_apn, _login, _pass)) {
    Debug.print(DBG_ERROR, F("The board was not able to register to the network..."));
    return NetworkConnectionState::ERROR;
  }
  Debug.print(DBG_INFO, F("Connected to Network"));
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState CellularConnectionHandler::update_handleConnected()
{
  if (!_cellular.isConnectedToInternet()) {
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState CellularConnectionHandler::update_handleDisconnecting()
{
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState CellularConnectionHandler::update_handleDisconnected()
{
  if (_keep_alive) {
    return NetworkConnectionState::INIT;
  }
  return NetworkConnectionState::CLOSED;
}

#endif /* #ifdef BOARD_HAS_CELLULAR  */
