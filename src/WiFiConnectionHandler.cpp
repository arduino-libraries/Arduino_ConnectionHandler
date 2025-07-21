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

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */
#include "WiFiConnectionHandler.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/
#if defined(ARDUINO_ARCH_ESP8266)
static int const ESP_WIFI_CONNECTION_TIMEOUT = 3000;
#endif

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

WiFiConnectionHandler::WiFiConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::WIFI) {
}

WiFiConnectionHandler::WiFiConnectionHandler(char const * ssid, char const * pass, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::WIFI}
{
  _settings.type = NetworkAdapter::WIFI;
  strncpy(_settings.wifi.ssid, ssid, sizeof(_settings.wifi.ssid)-1);
  strncpy(_settings.wifi.pwd, pass, sizeof(_settings.wifi.pwd)-1);
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long WiFiConnectionHandler::getTime()
{
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32)
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
#if !defined(__AVR__)
  DEBUG_INFO(F("WiFi.status(): %d"), WiFi.status());
#endif

#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32)
  if (WiFi.status() == NETWORK_HARDWARE_ERROR)
  {
#if !defined(__AVR__)
    DEBUG_ERROR(F("WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield."));
    DEBUG_ERROR(F("Then reset and retry."));
#endif
    return NetworkConnectionState::ERROR;
  }
#if !defined(__AVR__)
  DEBUG_INFO(F("Current WiFi Firmware: %s"), WiFi.firmwareVersion());
#endif

#if defined(WIFI_FIRMWARE_VERSION_REQUIRED)
  if (String(WiFi.firmwareVersion()) < String(WIFI_FIRMWARE_VERSION_REQUIRED))
  {
#if !defined(__AVR__)
    DEBUG_ERROR(F("Latest WiFi Firmware: %s"), WIFI_FIRMWARE_VERSION_REQUIRED);
    DEBUG_ERROR(F("Please update to the latest version for best performance."));
#endif
    delay(5000);
  }
#endif
#else
  WiFi.mode(WIFI_STA);
#endif /* #if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32) */

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(_settings.wifi.ssid, _settings.wifi.pwd);
#if defined(ARDUINO_ARCH_ESP8266)
    /* Wait connection otherwise board won't connect */
    unsigned long start = millis();
    while((WiFi.status() != WL_CONNECTED) && (millis() - start) < ESP_WIFI_CONNECTION_TIMEOUT) {
      delay(100);
    }
#endif

  }

  if (WiFi.status() != NETWORK_CONNECTED)
  {
#if !defined(__AVR__)
    DEBUG_ERROR(F("Connection to \"%s\" failed"), _settings.wifi.ssid);
    DEBUG_INFO(F("Retrying in  \"%d\" milliseconds"), _timeoutTable.timeout.init);
#endif
    return NetworkConnectionState::INIT;
  }
  else
  {
#if !defined(__AVR__)
    DEBUG_INFO(F("Connected to \"%s\""), _settings.wifi.ssid);
#endif
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  configTime(0, 0, "time.arduino.cc", "pool.ntp.org", "time.nist.gov");
#endif
    return NetworkConnectionState::CONNECTING;
  }
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnecting()
{
  if (WiFi.status() != WL_CONNECTED){
    return NetworkConnectionState::INIT;
  }

  if(!_check_internet_availability){
    return NetworkConnectionState::CONNECTED;
  }

  #if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32)
  int ping_result = WiFi.ping("time.arduino.cc");
  DEBUG_INFO(F("WiFi.ping(): %d"), ping_result);
  if (ping_result < 0)
  {
    DEBUG_ERROR(F("Internet check failed"));
    DEBUG_INFO(F("Retrying in  \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
    return NetworkConnectionState::CONNECTING;
  }
  #endif
  DEBUG_INFO(F("Connected to Internet"));
  return NetworkConnectionState::CONNECTED;

}

NetworkConnectionState WiFiConnectionHandler::update_handleConnected()
{
  if (WiFi.status() != WL_CONNECTED)
  {
#if !defined(__AVR__)
    DEBUG_VERBOSE(F("WiFi.status(): %d"), WiFi.status());
    DEBUG_ERROR(F("Connection to \"%s\" lost."), _settings.wifi.ssid);
#endif
    if (_keep_alive)
    {
#if !defined(__AVR__)
      DEBUG_INFO(F("Attempting reconnection"));
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
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32)
  WiFi.end();
#endif /* #if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32) */
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
