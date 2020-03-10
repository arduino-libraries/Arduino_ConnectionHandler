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

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

GSMConnectionHandler::GSMConnectionHandler(const char *pin, const char *apn, const char *login, const char *pass, bool _keepAlive) :
  pin(pin),
  apn(apn),
  login(login),
  pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE),
  keepAlive(_keepAlive) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long GSMConnectionHandler::getTime() {
  return gsmAccess.getTime();
}

NetworkConnectionState GSMConnectionHandler::check() {
  unsigned long const now = millis();
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState)
    {
      case NetworkConnectionState::INIT:          netConnectionState = update_handleInit();          break;
      case NetworkConnectionState::CONNECTING:    netConnectionState = update_handleConnecting();    break;
      case NetworkConnectionState::CONNECTED:     netConnectionState = update_handleConnected();     break;
      case NetworkConnectionState::GETTIME:       /* Unused */                                       break;
      case NetworkConnectionState::DISCONNECTING: netConnectionState = update_handleDisconnecting(); break;
      case NetworkConnectionState::DISCONNECTED:  netConnectionState = update_handleDisconnected();  break;
    }
    lastConnectionTickTime = now;
  }

  return netConnectionState;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState GSMConnectionHandler::update_handleInit()
{
  if (gsmAccess.begin(pin) == GSM_READY)
  {
    Debug.print(DBG_INFO, "SIM card ok");
    gsmAccess.setTimeout(CHECK_INTERVAL_RETRYING);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_ERROR, "SIM not present or wrong PIN");
    execCallback(NetworkConnectionEvent::ERROR, 0);
    return NetworkConnectionState::ERROR;
  }
}

NetworkConnectionState GSMConnectionHandler::update_handleConnecting()
{
  GSM3_NetworkStatus_t const network_status = gprs.attachGPRS(apn, login, pass, true);
  Debug.print(DBG_DEBUG, "GPRS.attachGPRS(): %d", network_status);
  if (network_status == GSM3_NetworkStatus_t::ERROR)
  {
    execCallback(NetworkConnectionEvent::ERROR, 0);
    return NetworkConnectionState::ERROR;
  }
  Debug.print(DBG_INFO, "Sending PING to outer space...");
  int const ping_result = gprs.ping("time.arduino.cc");
  Debug.print(DBG_INFO, "GPRS.ping(): %d", ping_result);
  if (ping_result < 0)
  {
    Debug.print(DBG_ERROR, "PING failed");
    Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_INFO, "Connected to GPRS Network");
    execCallback(NetworkConnectionEvent::CONNECTED, 0);
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState GSMConnectionHandler::update_handleConnected()
{
  int const is_gsm_access_alive = gsmAccess.isAccessAlive();
  Debug.print(DBG_VERBOSE, "GPRS.isAccessAlive(): %d", is_gsm_access_alive);
  if (is_gsm_access_alive != 1)
  {
    execCallback(NetworkConnectionEvent::DISCONNECTED, 0);
    return NetworkConnectionState::DISCONNECTED;
  }
  Debug.print(DBG_VERBOSE, "Connected to Cellular Network");
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnecting()
{
  execCallback(NetworkConnectionEvent::DISCONNECTED, 0);
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnected()
{
  if (keepAlive)
  {
    Debug.print(DBG_VERBOSE, "keep alive > INIT");
    return NetworkConnectionState::INIT;
  }
  else
  {
   return NetworkConnectionState::CLOSED;
  }
}

void GSMConnectionHandler::changeConnectionState(NetworkConnectionState _newState) {
  int newInterval = CHECK_INTERVAL_IDLE;
  switch (_newState) {
    case NetworkConnectionState::INIT: {
        newInterval = CHECK_INTERVAL_INIT;
      }
      break;
    case NetworkConnectionState::CONNECTING: {
        Debug.print(DBG_INFO, "Connecting to Cellular Network");
        newInterval = CHECK_INTERVAL_CONNECTING;
      }
      break;
    case NetworkConnectionState::CONNECTED: {
        execCallback(NetworkConnectionEvent::CONNECTED, 0);
        newInterval = CHECK_INTERVAL_CONNECTED;
      }
      break;
    case NetworkConnectionState::GETTIME: {
      }
      break;
    case NetworkConnectionState::DISCONNECTING: {
        Debug.print(DBG_VERBOSE, "Disconnecting from Cellular Network");
        gsmAccess.shutdown();
      }
    case NetworkConnectionState::DISCONNECTED: {
        if (netConnectionState == NetworkConnectionState::CONNECTED) {
          execCallback(NetworkConnectionEvent::DISCONNECTED, 0);
          Debug.print(DBG_ERROR, "Disconnected from Cellular Network");
          Debug.print(DBG_ERROR, "Attempting reconnection");
          if (keepAlive) {
            Debug.print(DBG_ERROR, "Attempting reconnection");
          }
        }
        newInterval = CHECK_INTERVAL_DISCONNECTED;
      }
      break;
    case NetworkConnectionState::ERROR: {
        execCallback(NetworkConnectionEvent::ERROR, 0);
        Debug.print(DBG_ERROR, "GPRS attach failed\n\rMake sure the antenna is connected and reset your board.");
      }
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
}


void GSMConnectionHandler::connect() {
  if (netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING) {
    return;
  }
  keepAlive = true;
  changeConnectionState(NetworkConnectionState::INIT);

}
void GSMConnectionHandler::disconnect() {
  //WiFi.end();

  changeConnectionState(NetworkConnectionState::DISCONNECTING);
  keepAlive = false;
}

#endif /* #ifdef BOARD_HAS_GSM  */