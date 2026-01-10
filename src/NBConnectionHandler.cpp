/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_NB /* Only compile if this is a board with NB */
#include "NBConnectionHandler.h"

/******************************************************************************
  CONSTANTS
 ******************************************************************************/

static int const NB_TIMEOUT = 30000;

/******************************************************************************
  FUNCTION DEFINITION
 ******************************************************************************/

__attribute__((weak)) void mkr_nb_feed_watchdog()
{
    /* This function can be overwritten by a "strong" implementation
     * in a higher level application, such as the ArduinoIoTCloud
     * firmware stack.
     */
}

/******************************************************************************
  CTOR/DTOR
 ******************************************************************************/

NBConnectionHandler::NBConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::NB) {}

NBConnectionHandler::NBConnectionHandler(char const * pin, bool const keep_alive)
: NBConnectionHandler(pin, "", keep_alive)
{

}

NBConnectionHandler::NBConnectionHandler(char const * pin, char const * apn, bool const keep_alive)
: NBConnectionHandler(pin, apn, "", "", keep_alive)
{

}

NBConnectionHandler::NBConnectionHandler(char const * pin, char const * apn, char const * login, char const * pass, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::NB}
{
  _settings.type = NetworkAdapter::NB;
  strncpy(_settings.nb.pin, pin, sizeof(_settings.nb.pin)-1);
  strncpy(_settings.nb.apn, apn, sizeof(_settings.nb.apn)-1);
  strncpy(_settings.nb.login, login, sizeof(_settings.nb.login)-1);
  strncpy(_settings.nb.pass, pass, sizeof(_settings.nb.pass)-1);
}

/******************************************************************************
  PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long NBConnectionHandler::getTime()
{
  return _nb.getTime();
}

/******************************************************************************
  PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState NBConnectionHandler::update_handleInit()
{
  mkr_nb_feed_watchdog();

  if (_nb.begin(_settings.nb.pin,
                _settings.nb.apn,
                _settings.nb.login,
                _settings.nb.pass) == NB_READY)
  {
    DEBUG_INFO(F("SIM card ok"));
    _nb.setTimeout(NB_TIMEOUT);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    DEBUG_ERROR(F("SIM not present or wrong PIN"));
    return NetworkConnectionState::ERROR;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleConnecting()
{
  NB_NetworkStatus_t const network_status = _nb_gprs.attachGPRS(true);
  DEBUG_DEBUG(F("GPRS.attachGPRS(): %d"), network_status);
  if (network_status == NB_NetworkStatus_t::NB_ERROR)
  {
    DEBUG_ERROR(F("GPRS.attachGPRS() failed"));
    return NetworkConnectionState::ERROR;
  }
  else
  {
    DEBUG_INFO(F("Connected to GPRS Network"));
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleConnected()
{
  int const nb_is_access_alive = _nb.isAccessAlive();
  DEBUG_VERBOSE(F("GPRS.isAccessAlive(): %d"), nb_is_access_alive);
  if (nb_is_access_alive != 1)
  {
    DEBUG_INFO(F("Disconnected from cellular network"));
    return NetworkConnectionState::DISCONNECTED;
  }
  else
  {
    DEBUG_VERBOSE(F("Connected to Cellular Network"));
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleDisconnecting()
{
  DEBUG_VERBOSE(F("Disconnecting from Cellular Network"));
  _nb.shutdown();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState NBConnectionHandler::update_handleDisconnected()
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

#endif /* #ifdef BOARD_HAS_NB  */
