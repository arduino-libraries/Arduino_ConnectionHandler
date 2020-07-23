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

#include "Arduino_GSMConnectionHandler.h"

#ifdef BOARD_HAS_GSM /* Only compile if this is a board with GSM */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const GSM_TIMEOUT = 30000;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

GSMConnectionHandler::GSMConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive)
: ConnectionHandler{keep_alive}
, _pin(pin)
, _apn(apn)
, _login(login)
, _pass(pass)
{

}

GSMConnectionHandler::GSMConnectionHandler(bool const keep_alive)
: ConnectionHandler{keep_alive}
, _pin("")
, _apn("")
, _login("")
, _pass("")
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long GSMConnectionHandler::getTime()
{
  return _gsm.getTime();
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState GSMConnectionHandler::update_handleInit()
{
  _gsm.setTimeout(GSM_TIMEOUT);
  if (_gsm.begin(_pin) == GSM_READY)
  {
    Debug.print(DBG_INFO, "SIM card ok");
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_ERROR, "SIM not present or wrong PIN");
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
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState GSMConnectionHandler::update_handleConnected()
{
  int const is_gsm_access_alive = _gsm.isAccessAlive();
  if (is_gsm_access_alive != 1)
  {
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnecting()
{
  _gsm.shutdown();
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
