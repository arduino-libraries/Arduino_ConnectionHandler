/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

int WiFiConnectionHandler::ping(IPAddress ip, uint8_t ttl, uint8_t count) {
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ZEPHYR)
  return WiFi.ping(ip);
#else
  return 0;
#endif
}

int WiFiConnectionHandler::ping(const String &hostname, uint8_t ttl, uint8_t count) {
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ZEPHYR)
  return WiFi.ping(hostname);
#else
  return 0;
#endif
}

int WiFiConnectionHandler::ping(const char* host, uint8_t ttl, uint8_t count) {
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ZEPHYR)
  return WiFi.ping(host);
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
  int ping_result = ping("time.arduino.cc");
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
