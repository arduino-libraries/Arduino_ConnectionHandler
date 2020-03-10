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

#include "Arduino_GSMConnectionHandler.h"

#ifdef BOARD_HAS_GSM /* Only compile if this is a board with GSM */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const GSM_TIMEOUT = 30000;

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

GSMConnectionHandler::GSMConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive)
: _pin(pin)
, _apn(apn)
, _login(login)
, _pass(pass)
, _keep_alive(keep_alive)
, _lastConnectionTickTime(millis())
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long GSMConnectionHandler::getTime()
{
  return _gsm.getTime();
}

NetworkConnectionState GSMConnectionHandler::check()
{
  unsigned long const now = millis();
  unsigned int const connectionTickTimeInterval = CHECK_INTERVAL_TABLE[static_cast<unsigned int>(_netConnectionState)];

  if((now - _lastConnectionTickTime) > connectionTickTimeInterval)
  {
    _lastConnectionTickTime = now;

    switch (_netConnectionState)
    {
      case NetworkConnectionState::INIT:          _netConnectionState = update_handleInit();          break;
      case NetworkConnectionState::CONNECTING:    _netConnectionState = update_handleConnecting();    break;
      case NetworkConnectionState::CONNECTED:     _netConnectionState = update_handleConnected();     break;
      case NetworkConnectionState::GETTIME:       /* Unused */                                        break;
      case NetworkConnectionState::DISCONNECTING: _netConnectionState = update_handleDisconnecting(); break;
      case NetworkConnectionState::DISCONNECTED:  _netConnectionState = update_handleDisconnected();  break;
      case NetworkConnectionState::ERROR:                                                             break;
      case NetworkConnectionState::CLOSED:                                                            break;
    }
  }

  return _netConnectionState;
}

void GSMConnectionHandler::disconnect()
{
  _gsm.shutdown();
  _keep_alive = false;
  _netConnectionState = NetworkConnectionState::DISCONNECTING;
}

void GSMConnectionHandler::connect()
{
  if (_netConnectionState != NetworkConnectionState::INIT && _netConnectionState != NetworkConnectionState::CONNECTING)
  {
    _keep_alive = true;
    _netConnectionState = NetworkConnectionState::INIT;
  }
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState GSMConnectionHandler::update_handleInit()
{
  if (_gsm.begin(_pin) == GSM_READY)
  {
    Debug.print(DBG_INFO, "SIM card ok");
    _gsm.setTimeout(GSM_TIMEOUT);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_ERROR, "SIM not present or wrong PIN");
    execCallback(NetworkConnectionEvent::ERROR);
    return NetworkConnectionState::ERROR;
  }
}

NetworkConnectionState GSMConnectionHandler::update_handleConnecting()
{
  GSM3_NetworkStatus_t const network_status = _gprs.attachGPRS(_apn, _login, _pass, true);
  Debug.print(DBG_DEBUG, "GPRS.attachGPRS(): %d", network_status);
  if (network_status == GSM3_NetworkStatus_t::ERROR)
  {
    Debug.print(DBG_ERROR, "GPRS attach failed");
    Debug.print(DBG_ERROR, "Make sure the antenna is connected and reset your board.");
    execCallback(NetworkConnectionEvent::ERROR);
    return NetworkConnectionState::ERROR;
  }
  Debug.print(DBG_INFO, "Sending PING to outer space...");
  int const ping_result = _gprs.ping("time.arduino.cc");
  Debug.print(DBG_INFO, "GPRS.ping(): %d", ping_result);
  if (ping_result < 0)
  {
    Debug.print(DBG_ERROR, "PING failed");
    Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", CHECK_INTERVAL_TABLE[static_cast<unsigned int>(NetworkConnectionState::CONNECTING)]);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_INFO, "Connected to GPRS Network");
    execCallback(NetworkConnectionEvent::CONNECTED);
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState GSMConnectionHandler::update_handleConnected()
{
  int const is_gsm_access_alive = _gsm.isAccessAlive();
  if (is_gsm_access_alive != 1)
  {
    execCallback(NetworkConnectionEvent::DISCONNECTED);
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnecting()
{
  execCallback(NetworkConnectionEvent::DISCONNECTED);
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnected()
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

#endif /* #ifdef BOARD_HAS_GSM  */
