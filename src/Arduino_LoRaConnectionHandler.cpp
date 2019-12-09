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

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310) /* Only compile if the board has WiFi */

#include "Arduino_LoRaConnectionHandler.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;   /*    NOT USED    */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

LoRaConnectionHandler::LoRaConnectionHandler(const char *_appeui, const char *_appkey, _lora_band band) :
  appeui(_appeui),
  appkey(_appkey),
  band(band),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE),
  keepAlive(false),
  _on_connect_event_callback(NULL),
  _on_disconnect_event_callback(NULL),
  _on_error_event_callback(NULL) {
    netConnectionState = NetworkConnectionState::INIT;
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void LoRaConnectionHandler::init() {
}

// INIT, CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED, CLOSED, ERROR
void LoRaConnectionHandler::addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback) {
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

void LoRaConnectionHandler::addConnectCallback(OnNetworkEventCallback callback) {
  _on_connect_event_callback = callback;
}
void LoRaConnectionHandler::addDisconnectCallback(OnNetworkEventCallback callback) {
  _on_disconnect_event_callback = callback;
}
void LoRaConnectionHandler::addErrorCallback(OnNetworkEventCallback callback) {
  _on_error_event_callback = callback;
}

void LoRaConnectionHandler::execNetworkEventCallback(OnNetworkEventCallback & callback, void * callback_arg) {
  if (callback) {
    (*callback)(callback_arg);
  }
}

unsigned long LoRaConnectionHandler::getTime() {
  return 0;
}

int LoRaConnectionHandler::write(const uint8_t *buf, size_t size) {
  int err;
  modem.beginPacket();
  modem.write(buf, size);
  err = modem.endPacket(true);
  /*Error manager according pr #68 of MKRWAN repo*/
  if (err != size) {
	switch (err) { 
		case -20: {Serial.println("Message length is bigger than max LoRa packet!");} break;
		case -1: {Serial.println("Message ack was not received, the message could not be delivered");} break;
		case -2: {Serial.println("LoRa generic error (LORA_ERROR)");} break;
		case -3: {Serial.println("LoRa malformed param error (LORA_ERROR_PARAM");} break;
		case -4: {Serial.println("LoRa chip is busy (LORA_ERROR_BUSY)");} break;
		case -5: {Serial.println("LoRa chip overflow error (LORA_ERROR_OVERFLOW)");} break;
		case -6: {Serial.println("LoRa no network error (LORA_ERROR_NO_NETWORK)");} break;
		case -7: {Serial.println("LoRa rx error (LORA_ERROR_RX)");} break;
		case -8: {Serial.println("LoRa unknown error (LORA_ERROR_UNKNOWN)");} break;
		}
	}
	else {
		Serial.println("Message sent correctly!");
	}
	return err;
}

int LoRaConnectionHandler::read() {
  return modem.read();
}

bool LoRaConnectionHandler::available() {
  return modem.available();
}

void LoRaConnectionHandler::update() {

  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) { /*  time bracket  */

    lastConnectionTickTime = now;

    switch (netConnectionState) {
      case NetworkConnectionState::INIT: {
          Debug.print(DBG_VERBOSE, "::INIT");
          if (!modem.begin(band)) {
            Debug.print(DBG_VERBOSE, "Failed to start module");
            changeConnectionState(NetworkConnectionState::ERROR);
          };
          delay(1000);

          changeConnectionState(NetworkConnectionState::CONNECTING);
        }
        break;
      case NetworkConnectionState::CONNECTING: {
          Debug.print(DBG_VERBOSE, "::CONNECTING");
          networkStatus = modem.joinOTAA(appeui, appkey);
          if (networkStatus != true) {
            changeConnectionState(NetworkConnectionState::ERROR);
            return;
          }

          Debug.print(DBG_INFO, "Connected to the network");
          changeConnectionState(NetworkConnectionState::CONNECTED);
          return;
        }
        break;
      case NetworkConnectionState::CONNECTED: {

          networkStatus = modem.connected();
          Debug.print(DBG_VERBOSE, "Connection state: %d", networkStatus);
          if (networkStatus != true) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
            return;
          }
          Debug.print(DBG_VERBOSE, "Connected to the network");
        }
        break;
      case NetworkConnectionState::DISCONNECTING: {
          changeConnectionState(NetworkConnectionState::DISCONNECTED);
        }
        break;
      case NetworkConnectionState::DISCONNECTED: {
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
  } /*  time bracket  */
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void LoRaConnectionHandler::changeConnectionState(NetworkConnectionState _newState) {
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
        Debug.print(DBG_INFO, "Connecting to the network");
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
        Debug.print(DBG_VERBOSE, "Disconnecting from the network");
      }
      break;
    case NetworkConnectionState::DISCONNECTED: {
        execNetworkEventCallback(_on_disconnect_event_callback, 0);
        //Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());

        Debug.print(DBG_ERROR, "Connection to the network lost.");
        if (keepAlive) {
          Debug.print(DBG_ERROR, "Attempting reconnection");
        }

        newInterval = CHECK_INTERVAL_DISCONNECTED;
      }
      break;
    case NetworkConnectionState::CLOSED: {

        Debug.print(DBG_VERBOSE, "Connection to the network terminated");
      }
      break;
    case NetworkConnectionState::ERROR: {
        execNetworkEventCallback(_on_error_event_callback, 0);
        Debug.print(DBG_ERROR, "Something went wrong; are you indoor? Move near a window, then reset and retry.");
      }
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
  //connectionStateChanged(netConnectionState);
}

void LoRaConnectionHandler::connect() {
  if (netConnectionState == NetworkConnectionState::INIT || netConnectionState == NetworkConnectionState::CONNECTING) {
    return;
  }
  keepAlive = true;
  changeConnectionState(NetworkConnectionState::INIT);

}
void LoRaConnectionHandler::disconnect() {
  // do nothing
    return;
}
#endif 
