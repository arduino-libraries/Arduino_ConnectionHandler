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

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
NBConnectionHandler::NBConnectionHandler(const char *pin, bool _keepAlive) :
  NBConnectionHandler(pin, "", _keepAlive) {
}

NBConnectionHandler::NBConnectionHandler(const char *pin, const char *apn, bool _keepAlive) :
  NBConnectionHandler(pin, apn, "", "", _keepAlive) {
}

NBConnectionHandler::NBConnectionHandler(const char *pin, const char *apn, const char *login, const char *pass, bool _keepAlive) :
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

void NBConnectionHandler::init() {
  char msgBuffer[120];
  if (nbAccess.begin(pin, apn, login, pass) == NB_READY) {
    Debug.print(DBG_INFO, "SIM card ok");
    nbAccess.setTimeout(CHECK_INTERVAL_RETRYING);
    changeConnectionState(NetworkConnectionState::CONNECTING);
  } else {
    Debug.print(DBG_ERROR, "SIM not present or wrong PIN");
  }
}

unsigned long NBConnectionHandler::getTime() {
  return nbAccess.getTime();
}

NetworkConnectionState NBConnectionHandler::check() {
  unsigned long const now = millis();
  int nbAccessAlive;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case NetworkConnectionState::INIT: {
          init();
        }
        break;

      case NetworkConnectionState::CONNECTING: {
          // NOTE: Blocking Call when 4th parameter == true
          NB_NetworkStatus_t networkStatus;
          networkStatus = gprs.attachGPRS(true);
          Debug.print(DBG_DEBUG, "GPRS.attachGPRS(): %d", networkStatus);
          if (networkStatus == NB_NetworkStatus_t::ERROR) {
            // NO FURTHER ACTION WILL FOLLOW THIS
            changeConnectionState(NetworkConnectionState::ERROR);
            return netConnectionState;
          }
          Debug.print(DBG_INFO, "Sending PING to outer space...");
          int pingResult;
          // pingResult = gprs.ping("time.arduino.cc");
          // Debug.print(DBG_INFO, "NB.ping(): %d", pingResult);
          // if (pingResult < 0) {
          if (pingResult < 0) {
            Debug.print(DBG_ERROR, "PING failed");
            Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
            return netConnectionState;
          } else {
            Debug.print(DBG_INFO, "Connected to GPRS Network");
            changeConnectionState(NetworkConnectionState::CONNECTED);
            return netConnectionState;
          }
        }
        break;
      case NetworkConnectionState::CONNECTED: {
          nbAccessAlive = nbAccess.isAccessAlive();
          Debug.print(DBG_VERBOSE, "GPRS.isAccessAlive(): %d", nbAccessAlive);
          if (nbAccessAlive != 1) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
            return netConnectionState;
          }
          Debug.print(DBG_VERBOSE, "Connected to Cellular Network");
        }
        break;
      case NetworkConnectionState::DISCONNECTED: {
          //gprs.detachGPRS();
          if (keepAlive) {
            Debug.print(DBG_VERBOSE, "keep alive > INIT");
            changeConnectionState(NetworkConnectionState::INIT);
          } else {
            changeConnectionState(NetworkConnectionState::CLOSED);
          }
          //changeConnectionState(NetworkConnectionState::CONNECTING);
        }
        break;
    }
    lastConnectionTickTime = now;
  }

  return netConnectionState;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void NBConnectionHandler::changeConnectionState(NetworkConnectionState _newState) {
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
        execNetworkEventCallback(_on_connect_event_callback, 0);
        newInterval = CHECK_INTERVAL_CONNECTED;
      }
      break;
    case NetworkConnectionState::GETTIME: {
      }
      break;
    case NetworkConnectionState::DISCONNECTING: {
        Debug.print(DBG_VERBOSE, "Disconnecting from Cellular Network");
        nbAccess.shutdown();
      }
    case NetworkConnectionState::DISCONNECTED: {
        if (netConnectionState == NetworkConnectionState::CONNECTED) {
          execNetworkEventCallback(_on_disconnect_event_callback, 0);
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
        execNetworkEventCallback(_on_error_event_callback, 0);
        Debug.print(DBG_ERROR, "GPRS attach failed\n\rMake sure the antenna is connected and reset your board.");
      }
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
}


void NBConnectionHandler::connect() {
  if (netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING) {
    return;
  }
  keepAlive = true;
  changeConnectionState(NetworkConnectionState::INIT);

}
void NBConnectionHandler::disconnect() {
  //WiFi.end();

  changeConnectionState(NetworkConnectionState::DISCONNECTING);
  keepAlive = false;
}

#endif /* #ifdef BOARD_HAS_NB  */