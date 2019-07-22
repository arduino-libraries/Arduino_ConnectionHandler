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

#include "Arduino_WiFiConnectionHandler.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;   /*    NOT USED    */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

WiFiConnectionHandler::WiFiConnectionHandler(const char *_ssid, const char *_pass, bool _keepAlive) :
  ssid(_ssid),
  pass(_pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE),
  keepAlive(_keepAlive),
  _on_connect_event_callback(NULL),
  _on_disconnect_event_callback(NULL),
  _on_error_event_callback(NULL) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void WiFiConnectionHandler::init() {
}

// INIT, CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED, CLOSED, ERROR
void WiFiConnectionHandler::addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback) {
  switch (event) {
    case NetworkConnectionEvent::CONNECTED:       _on_connect_event_callback       = callback; break;
    case NetworkConnectionEvent::DISCONNECTED:    _on_disconnect_event_callback    = callback; break;
    case NetworkConnectionEvent::ERROR:           _on_error_event_callback         = callback; break;
    case NetworkConnectionEvent::INIT:                                                       ; break;
    case NetworkConnectionEvent::CONNECTING:                                                 ; break;
    case NetworkConnectionEvent::DISCONNECTING:                                              ; break;
    case NetworkConnectionEvent::CLOSED:                                                     ; break;
  }
}

void WiFiConnectionHandler::addConnectCallback(OnNetworkEventCallback callback) {
  _on_connect_event_callback = callback;
}
void WiFiConnectionHandler::addDisconnectCallback(OnNetworkEventCallback callback) {
  _on_disconnect_event_callback = callback;
}
void WiFiConnectionHandler::addErrorCallback(OnNetworkEventCallback callback) {
  _on_error_event_callback = callback;
}

void WiFiConnectionHandler::execNetworkEventCallback(OnNetworkEventCallback & callback, void * callback_arg) {
  if (callback) {
    (*callback)(callback_arg);
  }
}

unsigned long WiFiConnectionHandler::getTime() {
  #ifdef GETTIME_MISSING
  return 0;
  #else
  return WiFi.getTime();
  #endif
}

void WiFiConnectionHandler::update() {

  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) { /*  time bracket  */

    switch (netConnectionState) {
      case NetworkConnectionState::INIT: {
          Debug.print(DBG_VERBOSE, "::INIT");
          #if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32) && !defined(ESP8266)
          networkStatus = WiFi.status();

          Debug.print(DBG_INFO, "WiFi.status(): %d", networkStatus);
          if (networkStatus == NETWORK_HARDWARE_ERROR) {
            // NO FURTHER ACTION WILL FOLLOW THIS
            changeConnectionState(NetworkConnectionState::ERROR);
            lastConnectionTickTime = now;
            return;
          }
          Debug.print(DBG_ERROR, "Current WiFi Firmware: %s", WiFi.firmwareVersion());
          if (WiFi.firmwareVersion() < WIFI_FIRMWARE_VERSION_REQUIRED) {
            Debug.print(DBG_ERROR, "Latest WiFi Firmware: %s", WIFI_FIRMWARE_VERSION_REQUIRED);
            Debug.print(DBG_ERROR, "Please update to the latest version for best performance.");
            delay(5000);
          }
          #else
          Debug.print(DBG_ERROR, "WiFi status ESP: %d", WiFi.status());
          WiFi.disconnect();
          delay(300);
          networkStatus = WiFi.begin(ssid, pass);
          delay(1000);
          #endif

          changeConnectionState(NetworkConnectionState::CONNECTING);
        }
        break;
      case NetworkConnectionState::CONNECTING: {
          Debug.print(DBG_VERBOSE, "::CONNECTING");
          networkStatus = WiFi.status();

          #if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32) &&  !defined(ESP8266)

          if (networkStatus != WL_CONNECTED) {
            networkStatus = WiFi.begin(ssid, pass);
          }

          #else

          networkStatus = WiFi.status();

          #endif

          Debug.print(DBG_VERBOSE, "WiFi.status(): %d", networkStatus);
          if (networkStatus != NETWORK_CONNECTED) {
            Debug.print(DBG_ERROR, "Connection to \"%s\" failed", ssid);
            Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);

            return;
          } else {
            Debug.print(DBG_INFO, "Connected to \"%s\"", ssid);
            changeConnectionState(NetworkConnectionState::CONNECTED);
            return;
          }
        }
        break;
      case NetworkConnectionState::CONNECTED: {

          networkStatus = WiFi.status();
          Debug.print(DBG_VERBOSE, "WiFi.status(): %d", networkStatus);
          if (networkStatus != WL_CONNECTED) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
            return;
          }
          Debug.print(DBG_VERBOSE, "Connected to \"%s\"", ssid);
        }
        break;
      case NetworkConnectionState::GETTIME: {

        }
        break;
      case NetworkConnectionState::DISCONNECTING: {
          if (networkStatus != WL_CONNECTED) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
          }
        }
        break;
      case NetworkConnectionState::DISCONNECTED: {
          #if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32) && !defined(ESP8266)
          WiFi.end();
          #endif
          if (keepAlive) {
            changeConnectionState(NetworkConnectionState::INIT);
          } else {
            changeConnectionState(NetworkConnectionState::CLOSED);
          }

        }
        break;
      case NetworkConnectionState::ERROR: {

        }
        break;
      case NetworkConnectionState::CLOSED: {

        }
        break;
    }
    lastConnectionTickTime = now;

  } /*  time bracket  */
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void WiFiConnectionHandler::changeConnectionState(NetworkConnectionState _newState) {
  if (_newState == netConnectionState) {
    return;
  }
  int newInterval = CHECK_INTERVAL_INIT;
  switch (_newState) {
    case NetworkConnectionState::INIT: {
        Debug.print(DBG_VERBOSE, "CHANGING STATE TO ::INIT");
        newInterval = CHECK_INTERVAL_INIT;
      }
      break;
    case NetworkConnectionState::CONNECTING: {
        Debug.print(DBG_INFO, "Connecting to \"%s\"", ssid);
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
        Debug.print(DBG_VERBOSE, "Disconnecting from \"%s\"", ssid);
        WiFi.disconnect();
      }
      break;
    case NetworkConnectionState::DISCONNECTED: {
        execNetworkEventCallback(_on_disconnect_event_callback, 0);
        Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());

        Debug.print(DBG_ERROR, "Connection to \"%s\" lost.", ssid);
        if (keepAlive) {
          Debug.print(DBG_ERROR, "Attempting reconnection");
        }

        newInterval = CHECK_INTERVAL_DISCONNECTED;
      }
      break;
    case NetworkConnectionState::CLOSED: {

        #if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32) &&  !defined(ESP8266)
        WiFi.end();
        #endif

        Debug.print(DBG_VERBOSE, "Connection to \"%s\" closed", ssid);
      }
      break;
    case NetworkConnectionState::ERROR: {
        execNetworkEventCallback(_on_error_event_callback, 0);
        Debug.print(DBG_ERROR, "WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield.");
        Debug.print(DBG_ERROR, "Then reset and retry.");
      }
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
  //connectionStateChanged(netConnectionState);
}

void WiFiConnectionHandler::connect() {
  if (netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING) {
    return;
  }
  keepAlive = true;
  changeConnectionState(NetworkConnectionState::INIT);

}
void WiFiConnectionHandler::disconnect() {
  //WiFi.end();

  changeConnectionState(NetworkConnectionState::DISCONNECTING);
  keepAlive = false;
}
#endif /* #ifdef BOARD_HAS_WIFI */
