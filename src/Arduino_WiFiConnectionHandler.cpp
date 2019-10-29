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
#if !defined(BOARD_ESP8266)
  return WiFi.getTime();
#else
  return 0;
#endif
}

void WiFiConnectionHandler::update() {

  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) { /*  time bracket  */

    lastConnectionTickTime = now;

    switch (netConnectionState) {
      case NetworkConnectionState::INIT:          netConnectionState = update_handleInit         (networkStatus); break;
      case NetworkConnectionState::CONNECTING:    netConnectionState = update_handleConnecting   (networkStatus); break;
      case NetworkConnectionState::CONNECTED:     netConnectionState = update_handleConnected    ();              break;
      case NetworkConnectionState::GETTIME:       netConnectionState = update_handleGetTime      ();              break;
      case NetworkConnectionState::DISCONNECTING: netConnectionState = update_handleDisconnecting();              break;
      case NetworkConnectionState::DISCONNECTED:  netConnectionState = update_handleDisconnected ();              break;
      case NetworkConnectionState::ERROR:                                                                         break;
      case NetworkConnectionState::CLOSED:                                                                        break;
    }
  } /*  time bracket  */
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void WiFiConnectionHandler::connect() {
  if (netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING) {
    return;
  }
  keepAlive = true;
  connectionTickTimeInterval = CHECK_INTERVAL_INIT;
  netConnectionState = NetworkConnectionState::INIT;
}

void WiFiConnectionHandler::disconnect() {
  keepAlive = false;
  netConnectionState = NetworkConnectionState::DISCONNECTING;
}

NetworkConnectionState WiFiConnectionHandler::update_handleInit(int & networkStatus) {
  Debug.print(DBG_VERBOSE, "::INIT");

#ifndef BOARD_ESP8266
  networkStatus = WiFi.status();

  Debug.print(DBG_INFO, "WiFi.status(): %d", networkStatus);
  if (networkStatus == NETWORK_HARDWARE_ERROR) {
    execNetworkEventCallback(_on_error_event_callback, 0);
    Debug.print(DBG_ERROR, "WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield.");
    Debug.print(DBG_ERROR, "Then reset and retry.");
    return NetworkConnectionState::ERROR;
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
#endif /* ifndef BOARD_ESP8266 */

  connectionTickTimeInterval = CHECK_INTERVAL_CONNECTING;
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnecting(int & networkStatus) {
  Debug.print(DBG_VERBOSE, "::CONNECTING");
  
  networkStatus = WiFi.status();

#ifndef BOARD_ESP8266
  if (networkStatus != WL_CONNECTED) {
    networkStatus = WiFi.begin(ssid, pass);
  }
#else
    networkStatus = WiFi.status();
#endif /* ifndef BOARD_ESP8266 */

  Debug.print(DBG_VERBOSE, "WiFi.status(): %d", networkStatus);
  if (networkStatus != NETWORK_CONNECTED) {
    Debug.print(DBG_ERROR, "Connection to \"%s\" failed", ssid);
    Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
    return NetworkConnectionState::CONNECTING;
  }
  else {
    Debug.print(DBG_INFO, "Connected to \"%s\"", ssid);
    execNetworkEventCallback(_on_connect_event_callback, 0);
    connectionTickTimeInterval = CHECK_INTERVAL_CONNECTED;
    return NetworkConnectionState::GETTIME;
  }
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnected() {

  Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());
  if (WiFi.status() != WL_CONNECTED)
  {
    execNetworkEventCallback(_on_disconnect_event_callback, 0);
   
    Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());
    Debug.print(DBG_ERROR, "Connection to \"%s\" lost.", ssid);
  
    if (keepAlive) {
      Debug.print(DBG_ERROR, "Attempting reconnection");
    }
  
    connectionTickTimeInterval = CHECK_INTERVAL_DISCONNECTED;
    return NetworkConnectionState::DISCONNECTED;
  }
  Debug.print(DBG_VERBOSE, "Connected to \"%s\"", ssid);
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleGetTime() {
  Debug.print(DBG_VERBOSE, "NetworkConnectionState::GETTIME");
#ifdef BOARD_ESP8266
  configTime(0, 0, "time.arduino.cc", "pool.ntp.org", "time.nist.gov");
#endif
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleDisconnecting() {
  Debug.print(DBG_VERBOSE, "Disconnecting from \"%s\"", ssid);
  WiFi.disconnect();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleDisconnected() {
#ifndef BOARD_ESP8266
  WiFi.end();
#endif /* ifndef BOARD_ESP8266 */
  if (keepAlive) {
    connectionTickTimeInterval = CHECK_INTERVAL_INIT;
    return NetworkConnectionState::INIT;
  }
  else {
    Debug.print(DBG_VERBOSE, "Connection to \"%s\" closed", ssid);
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_WIFI */
