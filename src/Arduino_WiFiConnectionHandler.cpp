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
   CTOR/DTOR
 ******************************************************************************/

WiFiConnectionHandler::WiFiConnectionHandler(char const * ssid, char const * pass, char const * user, bool const keep_alive)
: ConnectionHandler{keep_alive}
, _ssid{ssid}
, _pass{pass}
, _user{user}
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long WiFiConnectionHandler::getTime()
{
#if !defined(BOARD_ESP8266) && !defined(ESP32)
  return WiFi.getTime();
#else
  return 0;
#endif
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState WiFiConnectionHandler::update_handleInit()
{
#if !defined(BOARD_ESP8266) && !defined(ESP32)
#if !defined(__AVR__)
  Debug.print(DBG_INFO, F("WiFi.status(): %d"), WiFi.status());
#endif
  if (WiFi.status() == NETWORK_HARDWARE_ERROR)
  {
#if !defined(__AVR__)
    Debug.print(DBG_ERROR, F("WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield."));
    Debug.print(DBG_ERROR, F("Then reset and retry."));
#endif
    return NetworkConnectionState::ERROR;
  }
#if !defined(__AVR__)
  Debug.print(DBG_ERROR, F("Current WiFi Firmware: %s"), WiFi.firmwareVersion());
#endif

#if defined(WIFI_FIRMWARE_VERSION_REQUIRED)
  if (WiFi.firmwareVersion() < WIFI_FIRMWARE_VERSION_REQUIRED)
  {
#if !defined(__AVR__)
    Debug.print(DBG_ERROR, F("Latest WiFi Firmware: %s"), WIFI_FIRMWARE_VERSION_REQUIRED);
    Debug.print(DBG_ERROR, F("Please update to the latest version for best performance."));
#endif
    delay(5000);
  }
#endif

#else
  Debug.print(DBG_ERROR, F("WiFi status ESP: %d"), WiFi.status());
  WiFi.disconnect();
  delay(300);
  if (_user != "default"){
    WiFi.beginEnterprise(_ssid, _user, _pass);
  }
  else{
    WiFi.begin(_ssid, _pass);
  }
  delay(1000);
#endif /* #if !defined(BOARD_ESP8266) && !defined(ESP32) */

  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnecting()
{
#if !defined(BOARD_ESP8266) && !defined(ESP32)
  if (WiFi.status() != WL_CONNECTED)
  {
    if (_user != "default"){
      WiFi.beginEnterprise(_ssid, _user, _pass);
    }
    else{
      WiFi.begin(_ssid, _pass);
    }
  }
#endif /* ifndef BOARD_ESP8266 */

  if (WiFi.status() != NETWORK_CONNECTED)
  {
#if !defined(__AVR__)
    Debug.print(DBG_ERROR, F("Connection to \"%s\" failed"), _ssid);
    Debug.print(DBG_INFO, F("Retrying in  \"%d\" milliseconds"), CHECK_INTERVAL_TABLE[static_cast<unsigned int>(NetworkConnectionState::CONNECTING)]);
#endif
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
#if !defined(__AVR__)
    Debug.print(DBG_INFO, F("Connected to \"%s\""), _ssid);
#endif
#if defined(BOARD_ESP8266) || defined(ESP32)
  configTime(0, 0, "time.arduino.cc", "pool.ntp.org", "time.nist.gov");
#endif
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnected()
{
  if (WiFi.status() != WL_CONNECTED)
  {
#if !defined(__AVR__)
    Debug.print(DBG_VERBOSE, F("WiFi.status(): %d"), WiFi.status());
    Debug.print(DBG_ERROR, F("Connection to \"%s\" lost."), _ssid);
#endif
    if (_keep_alive)
    {
#if !defined(__AVR__)
      Debug.print(DBG_ERROR, F("Attempting reconnection"));
#endif
    }
  
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleDisconnecting()
{
  WiFi.disconnect();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleDisconnected()
{
#if !defined(BOARD_ESP8266) && !defined(ESP32)
  WiFi.end();
#endif /* ifndef BOARD_ESP8266 */
  if (_keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_WIFI */
