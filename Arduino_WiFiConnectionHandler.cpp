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

#include "Arduino_WiFiConnectionHandler.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;   /*    NOT USED    */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

Arduino_WiFiConnectionHandler::Arduino_WiFiConnectionHandler(const char *_ssid, const char *_pass, bool _keepAlive) :
  ssid(_ssid),
  pass(_pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE),
  keepAlive(_keepAlive),
  _on_connect_event_callback(NULL),
  _on_disconnect_event_callback(NULL),
  _on_error_event_callback(NULL){
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void Arduino_WiFiConnectionHandler::init() {
}

void Arduino_WiFiConnectionHandler::addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback){
  switch (event) {
    case NetworkConnectionEvent::CONNECTED:       _on_connect_event_callback       = callback; break;
    case NetworkConnectionEvent::DISCONNECTED:    _on_disconnect_event_callback    = callback; break;
    case NetworkConnectionEvent::ERROR:           _on_error_event_callback         = callback; break;
  }
}

void Arduino_WiFiConnectionHandler::execNetworkEventCallback(OnNetworkEventCallback & callback, void * callback_arg){
  if(callback){
    (*callback)(callback_arg);
  }
}

unsigned long Arduino_WiFiConnectionHandler::getTime() {
#ifdef GETTIME_MISSING
  return 0;
#else
  return WiFi.getTime();
#endif
}

void Arduino_WiFiConnectionHandler::update() {

  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) { /*  time bracket  */

    switch (netConnectionState) {
    case NetworkConnectionState::INIT: {
      debugMessage(DebugLevel::Verbose, "::INIT");
#if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32)
      networkStatus = WiFi.status();
      
      debugMessage(DebugLevel::Info, "WiFi.status(): %d", networkStatus);
      if (networkStatus == NETWORK_HARDWARE_ERROR) {
        // NO FURTHER ACTION WILL FOLLOW THIS
        changeConnectionState(NetworkConnectionState::ERROR);
        lastConnectionTickTime = now;
        return;
      }
      debugMessage(DebugLevel::Error, "Current WiFi Firmware: %s", WiFi.firmwareVersion());
      if (WiFi.firmwareVersion() < WIFI_FIRMWARE_VERSION_REQUIRED) {
        debugMessage(DebugLevel::Error, "Latest WiFi Firmware: %s", WIFI_FIRMWARE_VERSION_REQUIRED);
        debugMessage(DebugLevel::Error, "Please update to the latest version for best performance.");
        delay(5000);
      }
#else
      debugMessage(DebugLevel::Error, "WiFi status ESP: %d", WiFi.status());
      WiFi.disconnect();
      networkStatus = WiFi.begin(ssid, pass);
#endif
      
      changeConnectionState(NetworkConnectionState::CONNECTING);
    }
    break;
    case NetworkConnectionState::CONNECTING: {
      debugMessage(DebugLevel::Verbose, "::CONNECTING");
      networkStatus = WiFi.status();

#if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32)

      if (networkStatus != WL_CONNECTED) {
        networkStatus = WiFi.begin(ssid, pass);
      }

#else

      networkStatus = WiFi.status();

#endif

      debugMessage(DebugLevel::Verbose, "WiFi.status(): %d", networkStatus);
      if (networkStatus != NETWORK_CONNECTED) {
        debugMessage(DebugLevel::Error, "Connection to \"%s\" failed", ssid);
        debugMessage(DebugLevel::Info, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);

        return;
      } else {
        debugMessage(DebugLevel::Info, "Connected to \"%s\"", ssid);
        changeConnectionState(NetworkConnectionState::CONNECTED);
        return;
      }
    }
    break;
    case NetworkConnectionState::CONNECTED: {
      
      networkStatus = WiFi.status();
      debugMessage(DebugLevel::Verbose, "WiFi.status(): %d", networkStatus);
      if (networkStatus != WL_CONNECTED) {
        changeConnectionState(NetworkConnectionState::DISCONNECTED);
        return;
      }
      debugMessage(DebugLevel::Verbose, "Connected to \"%s\"", ssid);
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
#if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32)
      WiFi.end();
#endif
      if (keepAlive) {
        changeConnectionState(NetworkConnectionState::INIT);
      }else{
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

void Arduino_WiFiConnectionHandler::changeConnectionState(NetworkConnectionState _newState) {
  if(_newState == netConnectionState) return;
  int newInterval = CHECK_INTERVAL_INIT;
  switch (_newState) {
  case NetworkConnectionState::INIT: {
    debugMessage(DebugLevel::Verbose, "CHANGING STATE TO ::INIT");
    newInterval = CHECK_INTERVAL_INIT;
  }
  break;
  case NetworkConnectionState::CONNECTING: {
    debugMessage(DebugLevel::Info, "Connecting to \"%s\"", ssid);
    newInterval = CHECK_INTERVAL_CONNECTING;
  }
  break;
  case NetworkConnectionState::CONNECTED: {
    execNetworkEventCallback(_on_connect_event_callback,0);
    newInterval = CHECK_INTERVAL_CONNECTED;
  }
  break;
  case NetworkConnectionState::GETTIME: {
  }
  break;
  case NetworkConnectionState::DISCONNECTING: {
    debugMessage(DebugLevel::Verbose, "Disconnecting from \"%s\"", ssid);
    WiFi.disconnect();
  }
  break;
  case NetworkConnectionState::DISCONNECTED: {
    execNetworkEventCallback(_on_disconnect_event_callback,0);
    debugMessage(DebugLevel::Verbose, "WiFi.status(): %d", WiFi.status());

    debugMessage(DebugLevel::Error, "Connection to \"%s\" lost.", ssid);
    debugMessage(DebugLevel::Error, "Attempting reconnection");
    newInterval = CHECK_INTERVAL_DISCONNECTED;
  }
  break;
  case NetworkConnectionState::CLOSED: {
    
    #if !defined(ARDUINO_ESP8266_ESP12) && !defined(ARDUINO_ARCH_ESP32)
      WiFi.end();
    #endif
    
    debugMessage(DebugLevel::Verbose, "Connection to \"%s\" closed", ssid);
  }
  break;
  case NetworkConnectionState::ERROR: {
    execNetworkEventCallback(_on_error_event_callback,0);
    debugMessage(DebugLevel::Error, "WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield.");
    debugMessage(DebugLevel::Error, "Then reset and retry.");
  }
  break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
  connectionStateChanged(netConnectionState);
}

void Arduino_WiFiConnectionHandler::connect() {
  if(netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING){
    return;
  }
  keepAlive = true;
  changeConnectionState(NetworkConnectionState::INIT);

}
void Arduino_WiFiConnectionHandler::disconnect() {
  //WiFi.end();
  
  changeConnectionState(NetworkConnectionState::DISCONNECTING);
  keepAlive = false;
}
#endif /* #ifdef BOARD_HAS_WIFI */
