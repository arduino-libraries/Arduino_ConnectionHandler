/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

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

/*
  static int const DBG_NONE    = -1;
  static int const DBG_ERROR   =  0;
  static int const DBG_WARNING =  1;
  static int const DBG_INFO    =  2;
  static int const DBG_DEBUG   =  3;
  static int const DBG_VERBOSE =  4;
*/

#include "Arduino_NBConnectionHandler.h"

#ifdef BOARD_HAS_NB /* Only compile if this is a board with NB */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const NB_TIMEOUT = 30000;

static unsigned int const CHECK_INTERVAL_TABLE[] =
{
  /* INIT          */ 100,
  /* CONNECTING    */ 500,
  /* CONNECTED     */ 10000,
  /* GETTIME       */ 100,
  /* DISCONNECTING */ 100,
  /* DISCONNECTED  */ 1000,
  /* CLOSED        */ 1000,
  /* ERROR         */ 1000
};

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
NBConnectionHandler::NBConnectionHandler(char const * pin, bool const keep_alive)
: NBConnectionHandler(pin, "", keep_alive)
{

}

NBConnectionHandler::NBConnectionHandler(char const * pin, char const * apn, bool const keep_alive)
: NBConnectionHandler(pin, apn, "", "", keep_alive)
{

}

NBConnectionHandler::NBConnectionHandler(char const * pin, char const * apn, char const * login, char const * pass, bool const keep_alive)
: _pin(pin)
, _apn(apn)
, _login(login)
, _pass(pass)
, _lastConnectionTickTime(millis())
, _keep_alive(keep_alive)
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long NBConnectionHandler::getTime()
{
  return _nb.getTime();
}

NetworkConnectionState NBConnectionHandler::check()
{
  unsigned long const now = millis();
  unsigned int const connectionTickTimeInterval = CHECK_INTERVAL_TABLE[static_cast<unsigned int>(_netConnectionState)];

  if((now - _lastConnectionTickTime) > connectionTickTimeInterval)
  {
    _lastConnectionTickTime = now;

    switch (_netConnectionState)
    {
      case NetworkConnectionState::INIT:          _netConnectionState = update_handleInit         (); break;
      case NetworkConnectionState::CONNECTING:    _netConnectionState = update_handleConnecting   (); break;
      case NetworkConnectionState::CONNECTED:     _netConnectionState = update_handleConnected    (); break;
      case NetworkConnectionState::GETTIME:       /* Unused */                                        break;
      case NetworkConnectionState::DISCONNECTING: _netConnectionState = update_handleDisconnecting(); break;
      case NetworkConnectionState::DISCONNECTED:  _netConnectionState = update_handleDisconnected (); break;
      case NetworkConnectionState::ERROR:                                                             break;
      case NetworkConnectionState::CLOSED:                                                            break;
    }
  }

  return _netConnectionState;
}

void NBConnectionHandler::connect()
{
  if (_netConnectionState != NetworkConnectionState::INIT && _netConnectionState != NetworkConnectionState::CONNECTING)
  {
    _keep_alive = true;
    _netConnectionState = NetworkConnectionState::INIT;
  }
}

void NBConnectionHandler::disconnect()
{
  _keep_alive = false;
  _netConnectionState = NetworkConnectionState::DISCONNECTING;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState NBConnectionHandler::update_handleInit()
{
  if (_nb.begin(_pin, _apn, _login, _pass) == NB_READY)
  {
    Debug.print(DBG_INFO, "SIM card ok");
    _nb.setTimeout(NB_TIMEOUT);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_ERROR, "SIM not present or wrong PIN");
    return NetworkConnectionState::ERROR;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleConnecting()
{
  NB_NetworkStatus_t const network_status = _nb_gprs.attachGPRS(true);
  Debug.print(DBG_DEBUG, "GPRS.attachGPRS(): %d", network_status);
  if (network_status == NB_NetworkStatus_t::ERROR)
  {
    Debug.print(DBG_ERROR, "GPRS.attachGPRS() failed");
    return NetworkConnectionState::ERROR;
  }
  else
  {
    Debug.print(DBG_INFO, "Connected to GPRS Network");
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleConnected()
{
  int const nb_is_access_alive = _nb.isAccessAlive();
  Debug.print(DBG_VERBOSE, "GPRS.isAccessAlive(): %d", nb_is_access_alive);
  if (nb_is_access_alive != 1)
  {
    Debug.print(DBG_INFO, "Disconnected from cellular network");
    return NetworkConnectionState::DISCONNECTED;
  }
  else
  {
    Debug.print(DBG_VERBOSE, "Connected to Cellular Network");
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleDisconnecting()
{
  Debug.print(DBG_VERBOSE, "Disconnecting from Cellular Network");
  _nb.shutdown();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState NBConnectionHandler::update_handleDisconnected()
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

#endif /* #ifdef BOARD_HAS_NB  */
