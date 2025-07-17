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

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_CELLULAR /* Only compile if the board has Cellular */
#include "CellularConnectionHandler.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
CellularConnectionHandler::CellularConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::CELL) {}

CellularConnectionHandler::CellularConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::CELL}
{
  _settings.type = NetworkAdapter::CELL;
  strncpy(_settings.cell.pin, pin, sizeof(_settings.cell.pin)-1);
  strncpy(_settings.cell.apn, apn, sizeof(_settings.cell.apn)-1);
  strncpy(_settings.cell.login, login, sizeof(_settings.cell.login)-1);
  strncpy(_settings.cell.pass, pass, sizeof(_settings.cell.pass)-1);

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long CellularConnectionHandler::getTime()
{
  return _cellular.getCellularTime(false).getUNIXTimestamp();
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
  if (strlen(_settings.cell.pin) > 0 && !_cellular.unlockSIM(_settings.cell.pin)) {
    Debug.print(DBG_ERROR, F("SIM not present or wrong PIN"));
    return NetworkConnectionState::ERROR;
  }

  if (!_cellular.connect(String(_settings.cell.apn), String(_settings.cell.login), String(_settings.cell.pass))) {
    Debug.print(DBG_ERROR, F("The board was not able to register to the network..."));
    return NetworkConnectionState::ERROR;
  }
  Debug.print(DBG_INFO, F("Connected to Network"));
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState CellularConnectionHandler::update_handleConnecting()
{
  if (!_cellular.isConnectedToInternet()) {
    return NetworkConnectionState::INIT;
  }

  if (!_check_internet_availability) {
    return NetworkConnectionState::CONNECTED;
  }

  if(getTime() == 0){
    Debug.print(DBG_ERROR, F("Internet check failed"));
    Debug.print(DBG_INFO, F("Retrying in  \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
    return NetworkConnectionState::CONNECTING;
  }

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
