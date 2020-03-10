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

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310) /* Only compile if the board has LoRa */

#include "Arduino_LoRaConnectionHandler.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;   /*    NOT USED    */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

LoRaConnectionHandler::LoRaConnectionHandler(char const * appeui, char const * appkey, _lora_band const band, _lora_class const device_class)
: _appeui(appeui)
, _appkey(appkey)
, _band(band)
, _device_class(device_class)
, _lastConnectionTickTime(millis())
, _connectionTickTimeInterval(CHECK_INTERVAL_IDLE)
, _keep_alive(false)
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int LoRaConnectionHandler::write(const uint8_t *buf, size_t size) {
  int err;
  _modem.beginPacket();
  _modem.write(buf, size);
  err = _modem.endPacket(true);
  if (err != size) {
    switch (err) {
      case LoRaCommunicationError::LORA_ERROR_ACK_NOT_RECEIVED: {
          Debug.print(DBG_ERROR, "Message ack was not received, the message could not be delivered");
        } break;
      case LoRaCommunicationError::LORA_ERROR_GENERIC: {
          Debug.print(DBG_ERROR, "LoRa generic error (LORA_ERROR)");
        } break;
      case LoRaCommunicationError::LORA_ERROR_WRONG_PARAM: {
          Debug.print(DBG_ERROR, "LoRa malformed param error (LORA_ERROR_PARAM");
        } break;
      case LoRaCommunicationError::LORA_ERROR_COMMUNICATION_BUSY: {
          Debug.print(DBG_ERROR, "LoRa chip is busy (LORA_ERROR_BUSY)");
        } break;
      case LoRaCommunicationError::LORA_ERROR_MESSAGE_OVERFLOW: {
          Debug.print(DBG_ERROR, "LoRa chip overflow error (LORA_ERROR_OVERFLOW)");
        } break;
      case LoRaCommunicationError::LORA_ERROR_NO_NETWORK_AVAILABLE: {
          Debug.print(DBG_ERROR, "LoRa no network error (LORA_ERROR_NO_NETWORK)");
        } break;
      case LoRaCommunicationError::LORA_ERROR_RX_PACKET: {
          Debug.print(DBG_ERROR, "LoRa rx error (LORA_ERROR_RX)");
        } break;
      case LoRaCommunicationError::LORA_ERROR_REASON_UNKNOWN: {
          Debug.print(DBG_ERROR, "LoRa unknown error (LORA_ERROR_UNKNOWN)");
        } break;
      case LoRaCommunicationError::LORA_ERROR_MAX_PACKET_SIZE: {
          Debug.print(DBG_ERROR, "Message length is bigger than max LoRa packet!");
        } break;
    }
  } else {
    Debug.print(DBG_INFO, "Message sent correctly!");
  }
  return err;
}

int LoRaConnectionHandler::read() {
  return _modem.read();
}

bool LoRaConnectionHandler::available() {
  return _modem.available();
}

NetworkConnectionState LoRaConnectionHandler::check() {

  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - _lastConnectionTickTime > _connectionTickTimeInterval) { /*  time bracket  */

    _lastConnectionTickTime = now;
    switch (netConnectionState) {
      case NetworkConnectionState::INIT:          netConnectionState = update_handleInit(); break;
      case NetworkConnectionState::CONNECTING:    netConnectionState = update_handleConnecting(); break;
      case NetworkConnectionState::CONNECTED:     netConnectionState = update_handleConnected(); break;
      case NetworkConnectionState::DISCONNECTING: netConnectionState = update_handleDisconnecting(); break;
      case NetworkConnectionState::DISCONNECTED:  netConnectionState = update_handleDisconnected(); break;
      case NetworkConnectionState::ERROR:                                                            break;
      case NetworkConnectionState::CLOSED:                                                           break;
    }
  }

  return netConnectionState;
}

void LoRaConnectionHandler::connect()
{
  if (netConnectionState != NetworkConnectionState::INIT && netConnectionState != NetworkConnectionState::CONNECTING)
  {
    _keep_alive = true;
    netConnectionState = NetworkConnectionState::INIT;
  }
}
void LoRaConnectionHandler::disconnect()
{
  /* Do nothing */
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState LoRaConnectionHandler::update_handleInit() {
  Debug.print(DBG_VERBOSE, "::INIT");
  if (!_modem.begin(_band)) {
    Debug.print(DBG_VERBOSE, "Failed to start module");
    execCallback(NetworkConnectionEvent::ERROR, 0);
    Debug.print(DBG_ERROR, "Something went wrong; are you indoor? Move near a window, then reset and retry.");
  };
  //A delay is required between _modem.begin(band) and _modem.joinOTAA(appeui, appkey) in order to let the chip to be correctly initialized before the connection attempt
  delay(100);
  _modem.configureClass(_device_class);
  delay(100);
  Debug.print(DBG_INFO, "Connecting to the network");
  _connectionTickTimeInterval = CHECK_INTERVAL_CONNECTING;
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState LoRaConnectionHandler::update_handleConnecting() {
  Debug.print(DBG_VERBOSE, "::CONNECTING");
  bool const network_status = _modem.joinOTAA(_appeui, _appkey);
  if (network_status != true) {
    execCallback(NetworkConnectionEvent::ERROR, 0);
    Debug.print(DBG_ERROR, "Something went wrong; are you indoor? Move near a window, then reset and retry.");
    return NetworkConnectionState::ERROR;
  }

  Debug.print(DBG_INFO, "Connected to the network");
  _connectionTickTimeInterval = CHECK_INTERVAL_CONNECTED;
  execCallback(NetworkConnectionEvent::CONNECTED, 0);
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState LoRaConnectionHandler::update_handleConnected() {

  bool const network_status = _modem.connected();
  Debug.print(DBG_VERBOSE, "Connection state: %d", network_status);
  if (network_status != true) {
    execCallback(NetworkConnectionEvent::DISCONNECTED, 0);

    Debug.print(DBG_ERROR, "Connection to the network lost.");
    if (_keep_alive) {
      Debug.print(DBG_ERROR, "Attempting reconnection");
    }
    _connectionTickTimeInterval = CHECK_INTERVAL_DISCONNECTED;
    return NetworkConnectionState::DISCONNECTED;
  }
  Debug.print(DBG_VERBOSE, "Connected to the network");

  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState LoRaConnectionHandler::update_handleDisconnecting() {
  execCallback(NetworkConnectionEvent::DISCONNECTED, 0);

  Debug.print(DBG_ERROR, "Connection to the network lost.");
  if (_keep_alive) {
    Debug.print(DBG_ERROR, "Attempting reconnection");
  }
  _connectionTickTimeInterval = CHECK_INTERVAL_DISCONNECTED;
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState LoRaConnectionHandler::update_handleDisconnected() {
  if (_keep_alive) {
    Debug.print(DBG_VERBOSE, "CHANGING STATE TO ::INIT");
    _connectionTickTimeInterval = CHECK_INTERVAL_INIT;
    return NetworkConnectionState::INIT;
  } else {
    Debug.print(DBG_VERBOSE, "Connection to the network terminated");
    return NetworkConnectionState::CLOSED;
  }

}

#endif
