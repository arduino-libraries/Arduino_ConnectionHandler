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

WiFiConnectionHandler::WiFiConnectionHandler(const char *_ssid, const char *_pass, bool _keepAlive) :
  ssid(_ssid),
  pass(_pass),
  lastConnectionTickTime(millis()),
  keepAlive(_keepAlive) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void WiFiConnectionHandler::init() {
}

unsigned long WiFiConnectionHandler::getTime() {
#if !defined(BOARD_ESP8266)
  return WiFi.getTime();
#else
  return 0;
#endif
}

NetworkConnectionState WiFiConnectionHandler::check() {

  unsigned long const now = millis();
  unsigned int const connectionTickTimeInterval = CHECK_INTERVAL_TABLE[static_cast<unsigned int>(netConnectionState)];

  if((now - lastConnectionTickTime) > connectionTickTimeInterval)
  {
    lastConnectionTickTime = now;

    switch (netConnectionState) {
      case NetworkConnectionState::INIT:          netConnectionState = update_handleInit         (); break;
      case NetworkConnectionState::CONNECTING:    netConnectionState = update_handleConnecting   (); break;
      case NetworkConnectionState::CONNECTED:     netConnectionState = update_handleConnected    (); break;
      case NetworkConnectionState::GETTIME:       netConnectionState = update_handleGetTime      (); break;
      case NetworkConnectionState::DISCONNECTING: netConnectionState = update_handleDisconnecting(); break;
      case NetworkConnectionState::DISCONNECTED:  netConnectionState = update_handleDisconnected (); break;
      case NetworkConnectionState::ERROR:                                                            break;
      case NetworkConnectionState::CLOSED:                                                           break;
    }
  }

  return netConnectionState;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void WiFiConnectionHandler::connect() {
  if (netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING) {
    return;
  }
  keepAlive = true;
  netConnectionState = NetworkConnectionState::INIT;
}

void WiFiConnectionHandler::disconnect() {
  keepAlive = false;
  netConnectionState = NetworkConnectionState::DISCONNECTING;
}

NetworkConnectionState WiFiConnectionHandler::update_handleInit() {
  Debug.print(DBG_VERBOSE, "::INIT");

#ifndef BOARD_ESP8266
  Debug.print(DBG_INFO, "WiFi.status(): %d", WiFi.status());
  if (WiFi.status() == NETWORK_HARDWARE_ERROR) {
    execCallback(NetworkConnectionEvent::ERROR, 0);
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
  WiFi.begin(ssid, pass);
  delay(1000);
#endif /* ifndef BOARD_ESP8266 */

  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnecting() {
  Debug.print(DBG_VERBOSE, "::CONNECTING");
  
#ifndef BOARD_ESP8266
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
  }
#endif /* ifndef BOARD_ESP8266 */

  Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());
  if (WiFi.status() != NETWORK_CONNECTED) {
    Debug.print(DBG_ERROR, "Connection to \"%s\" failed", ssid);
    Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", CHECK_INTERVAL_TABLE[static_cast<unsigned int>(NetworkConnectionState::CONNECTING)]);
    return NetworkConnectionState::CONNECTING;
  }
  else {
    Debug.print(DBG_INFO, "Connected to \"%s\"", ssid);
    execCallback(NetworkConnectionEvent::CONNECTED, 0);
    return NetworkConnectionState::GETTIME;
  }
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnected() {

  Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());
  if (WiFi.status() != WL_CONNECTED)
  {
    execCallback(NetworkConnectionEvent::DISCONNECTED, 0);
   
    Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());
    Debug.print(DBG_ERROR, "Connection to \"%s\" lost.", ssid);
  
    if (keepAlive) {
      Debug.print(DBG_ERROR, "Attempting reconnection");
    }
  
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
    return NetworkConnectionState::INIT;
  }
  else {
    Debug.print(DBG_VERBOSE, "Connection to \"%s\" closed", ssid);
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_WIFI */
