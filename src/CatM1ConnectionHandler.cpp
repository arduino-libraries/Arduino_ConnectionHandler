/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2023 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_CATM1_NBIOT /* Only compile if the board has CatM1 BN-IoT */
#include "CatM1ConnectionHandler.h"

/******************************************************************************
  CTOR/DTOR
 ******************************************************************************/

CatM1ConnectionHandler::CatM1ConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::CATM1) { }

CatM1ConnectionHandler::CatM1ConnectionHandler(
  const char * pin, const char * apn, const char * login, const char * pass,
  RadioAccessTechnologyType rat, uint32_t band, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::CATM1}
{
  _settings.type = NetworkAdapter::CATM1;
  // To keep the backward compatibility, the user can call enableCheckInternetAvailability(false) for disabling the check
  _check_internet_availability = true;
  strncpy(_settings.catm1.pin, pin, sizeof(_settings.catm1.pin)-1);
  strncpy(_settings.catm1.apn, apn, sizeof(_settings.catm1.apn)-1);
  strncpy(_settings.catm1.login, login, sizeof(_settings.catm1.login)-1);
  strncpy(_settings.catm1.pass, pass, sizeof(_settings.catm1.pass)-1);
  _settings.catm1.rat  = static_cast<uint8_t>(rat);
  _settings.catm1.band = band;
  _reset = false;
}

/******************************************************************************
  PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long CatM1ConnectionHandler::getTime()
{
  /* It is not safe to call GSM.getTime() since we don't know if modem internal
   * RTC is in sync with current time.
   */
  return 0;
}

int CatM1ConnectionHandler::ping(IPAddress ip, uint8_t ttl, uint8_t count) {
  return GSM.ping(ip);
}

int CatM1ConnectionHandler::ping(const String &hostname, uint8_t ttl, uint8_t count) {
  return GSM.ping(hostname);
}

int CatM1ConnectionHandler::ping(const char* host, uint8_t ttl, uint8_t count) {
  return GSM.ping(host);
}


/******************************************************************************
  PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState CatM1ConnectionHandler::update_handleInit()
{
#if defined (ARDUINO_EDGE_CONTROL)
  /* Power on module */
  pinMode(ON_MKR2, OUTPUT);
  digitalWrite(ON_MKR2, HIGH);
#endif

  if(!GSM.begin(
    _settings.catm1.pin,
    _settings.catm1.apn,
    _settings.catm1.login,
    _settings.catm1.pass,
    static_cast<RadioAccessTechnologyType>(_settings.catm1.rat) ,
    _settings.catm1.band,
    _reset))
  {
    DEBUG_ERROR(F("The board was not able to register to the network..."));
    _reset = true;
    return NetworkConnectionState::DISCONNECTED;
  }
  _reset = false;
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleConnecting()
{
  if (!GSM.isConnected())
  {
    DEBUG_ERROR(F("GSM connection not alive... disconnecting"));
    return NetworkConnectionState::DISCONNECTED;
  }

  if(!_check_internet_availability){
    return NetworkConnectionState::CONNECTED;
  }

  DEBUG_INFO(F("Sending PING to outer space..."));
  int const ping_result = ping("time.arduino.cc");
  DEBUG_INFO(F("GSM.ping(): %d"), ping_result);
  if (ping_result < 0)
  {
    DEBUG_ERROR(F("Internet check failed"));
    DEBUG_INFO(F("Retrying in  \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    DEBUG_INFO(F("Connected to Internet"));
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState CatM1ConnectionHandler::update_handleConnected()
{
  int const is_gsm_access_alive = GSM.isConnected();
  if (is_gsm_access_alive != 1)
  {
    DEBUG_ERROR(F("GSM connection not alive... disconnecting"));
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleDisconnecting()
{
  GSM.disconnect();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleDisconnected()
{
  GSM.end();
  if (_keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_CATM1_NBIOT  */
