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

#if defined(BOARD_HAS_LORA) /* Only compile if the board has LoRa */
#include "LoRaConnectionHandler.h"

/******************************************************************************
  TYPEDEF
 ******************************************************************************/

typedef enum
{
  LORA_ERROR_ACK_NOT_RECEIVED     = -1,
  LORA_ERROR_GENERIC              = -2,
  LORA_ERROR_WRONG_PARAM          = -3,
  LORA_ERROR_COMMUNICATION_BUSY   = -4,
  LORA_ERROR_MESSAGE_OVERFLOW     = -5,
  LORA_ERROR_NO_NETWORK_AVAILABLE = -6,
  LORA_ERROR_RX_PACKET            = -7,
  LORA_ERROR_REASON_UNKNOWN       = -8,
  LORA_ERROR_MAX_PACKET_SIZE      = -20
} LoRaCommunicationError;

/******************************************************************************
  CTOR/DTOR
 ******************************************************************************/
LoRaConnectionHandler::LoRaConnectionHandler(char const * appeui, char const * appkey, _lora_band const band, char const * channelMask, _lora_class const device_class)
: ConnectionHandler{false, NetworkAdapter::LORA}
{
  _settings.type = NetworkAdapter::LORA;
  strncpy(_settings.lora.appeui, appeui, sizeof(_settings.lora.appeui)-1);
  strncpy(_settings.lora.appkey, appkey, sizeof(_settings.lora.appkey)-1);
  _settings.lora.band = band;
  strncpy(_settings.lora.channelMask, channelMask, sizeof(_settings.lora.channelMask)-1);
  _settings.lora.deviceClass = device_class;
}

/******************************************************************************
  PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int LoRaConnectionHandler::write(const uint8_t * buf, size_t size)
{
  _modem.beginPacket();
  _modem.write(buf, size);
  int const err = _modem.endPacket(true);

  if (err != size)
  {
    switch (err)
    {
      case LoRaCommunicationError::LORA_ERROR_ACK_NOT_RECEIVED:
        DEBUG_ERROR(F("Message ack was not received, the message could not be delivered"));
        break;
      case LoRaCommunicationError::LORA_ERROR_GENERIC:
        DEBUG_ERROR(F("LoRa generic error (LORA_ERROR)"));
        break;
      case LoRaCommunicationError::LORA_ERROR_WRONG_PARAM:
        DEBUG_ERROR(F("LoRa malformed param error (LORA_ERROR_PARAM"));
        break;
      case LoRaCommunicationError::LORA_ERROR_COMMUNICATION_BUSY:
        DEBUG_ERROR(F("LoRa chip is busy (LORA_ERROR_BUSY)"));
        break;
      case LoRaCommunicationError::LORA_ERROR_MESSAGE_OVERFLOW:
        DEBUG_ERROR(F("LoRa chip overflow error (LORA_ERROR_OVERFLOW)"));
        break;
      case LoRaCommunicationError::LORA_ERROR_NO_NETWORK_AVAILABLE:
        DEBUG_ERROR(F("LoRa no network error (LORA_ERROR_NO_NETWORK)"));
        break;
      case LoRaCommunicationError::LORA_ERROR_RX_PACKET:
        DEBUG_ERROR(F("LoRa rx error (LORA_ERROR_RX)"));
        break;
      case LoRaCommunicationError::LORA_ERROR_REASON_UNKNOWN:
        DEBUG_ERROR(F("LoRa unknown error (LORA_ERROR_UNKNOWN)"));
        break;
      case LoRaCommunicationError::LORA_ERROR_MAX_PACKET_SIZE:
        DEBUG_ERROR(F("Message length is bigger than max LoRa packet!"));
        break;
    }
  }
  else
  {
    DEBUG_INFO(F("Message sent correctly!"));
  }
  return err;
}

int LoRaConnectionHandler::read()
{
  return _modem.read();
}

bool LoRaConnectionHandler::available()
{
  return _modem.available();
}

/******************************************************************************
  PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState LoRaConnectionHandler::update_handleInit()
{
  if (!_modem.begin((_lora_band)_settings.lora.band))
  {
    DEBUG_ERROR(F("Something went wrong; are you indoor? Move near a window, then reset and retry."));
    return NetworkConnectionState::ERROR;
  }
  // Set channelmask based on configuration
  if (_settings.lora.channelMask) {
    _modem.sendMask(_settings.lora.channelMask);
  }
  //A delay is required between _modem.begin(band) and _modem.joinOTAA(appeui, appkey) in order to let the chip to be correctly initialized before the connection attempt
  delay(100);
  _modem.configureClass((_lora_class)_settings.lora.deviceClass);
  delay(100);
  DEBUG_INFO(F("Connecting to the network"));
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState LoRaConnectionHandler::update_handleConnecting()
{
  bool const network_status = _modem.joinOTAA(_settings.lora.appeui, _settings.lora.appkey);
  if (network_status != true)
  {
    DEBUG_ERROR(F("Connection to the network failed"));
    DEBUG_INFO(F("Retrying in \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
    return NetworkConnectionState::INIT;
  }
  else
  {
    DEBUG_INFO(F("Connected to the network"));
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState LoRaConnectionHandler::update_handleConnected()
{
  bool const network_status = _modem.connected();
  if (network_status != true)
  {
    DEBUG_ERROR(F("Connection to the network lost."));
    if (_keep_alive)
    {
      DEBUG_ERROR(F("Attempting reconnection"));
    }
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState LoRaConnectionHandler::update_handleDisconnecting()
{
  DEBUG_ERROR(F("Connection to the network lost."));
  if (_keep_alive)
  {
    DEBUG_ERROR(F("Attempting reconnection"));
  }
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState LoRaConnectionHandler::update_handleDisconnected()
{
  if (_keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
    return NetworkConnectionState::CLOSED;
  }
}

#endif
