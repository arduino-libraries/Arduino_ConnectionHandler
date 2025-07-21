/*
  This file is part of the ArduinoIoTCloud library.

  Copyright 2024 Blues (http://www.blues.com/)

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerDefinitions.h"

#if defined(BOARD_HAS_NOTECARD) // Only compile if the Notecard is present

#include "NotecardConnectionHandler.h"

#include <Arduino.h>
#include <Arduino_DebugUtils.h>
#include <Wire.h>

/******************************************************************************
   DEFINES
 ******************************************************************************/

#define NO_INBOUND_POLLING -1

#define NOTEFILE_BASE_NAME "arduino_iot_cloud"

// Notecard LoRa requires us to choose an arbitrary port between 1-99
#define NOTEFILE_INBOUND_LORA_PORT 2
#define NOTEFILE_OUTBOUND_LORA_PORT 3

// Note that we use "s" versions of the Notefile extensions to ensure that
// traffic always happens on a secure transport
#define NOTEFILE_SECURE_INBOUND NOTEFILE_BASE_NAME ".qis"
#define NOTEFILE_SECURE_OUTBOUND NOTEFILE_BASE_NAME ".qos"

/******************************************************************************
   STLINK DEBUG OUTPUT
 ******************************************************************************/

// Provide Notehub debug output via STLINK serial port when available
#if defined(ARDUINO_SWAN_R5) || defined(ARDUINO_CYGNET)
  #define STLINK_DEBUG
  HardwareSerial stlinkSerial(PIN_VCP_RX, PIN_VCP_TX);
#endif

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

struct NotecardConnectionStatus
{
  NotecardConnectionStatus(void) : transport_connected(0), connected_to_notehub(0), notecard_error(0), host_error(0), reserved(0) { }
  NotecardConnectionStatus(uint_fast8_t x) : transport_connected(x & 0x01), connected_to_notehub(x & 0x02), notecard_error(x & 0x04), host_error(x & 0x08), reserved(x & 0xF0) { }
  NotecardConnectionStatus & operator=(uint_fast8_t x) {
      transport_connected  = (x & 0x01);
      connected_to_notehub = (x & 0x02);
      notecard_error       = (x & 0x04);
      host_error           = (x & 0x08);
      reserved             = (x & 0xF0);
      return *this;
  }
  operator uint_fast8_t () const {
      return ((reserved << 4) | (host_error << 3) | (notecard_error << 2) | (connected_to_notehub << 1) | (transport_connected));
  }

  bool transport_connected  : 1;
  bool connected_to_notehub : 1;
  bool notecard_error       : 1;
  bool host_error           : 1;
  uint_fast8_t reserved     : 4;
};
static_assert(sizeof(NotecardConnectionStatus) == sizeof(uint_fast8_t));

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

NotecardConnectionHandler::NotecardConnectionHandler(
  const String & project_uid_,
  uint32_t i2c_address_,
  uint32_t i2c_max_,
  TwoWire & wire_,
  bool keep_alive_
) :
  ConnectionHandler{keep_alive_, NetworkAdapter::NOTECARD},
  _notecard{},
  _device_id{},
  _notecard_uid{},
  _project_uid(project_uid_),
  _serial(nullptr),
  _wire(&wire_),
  _inbound_buffer(nullptr),
  _conn_start_ms(0),
  _i2c_address(i2c_address_),
  _i2c_max(i2c_max_),
  _inbound_buffer_index(0),
  _inbound_buffer_size(0),
  _inbound_polling_interval_min(NO_INBOUND_POLLING),
  _uart_baud(0),
  _en_hw_int(false),
  _topic_type{TopicType::Invalid}
{ }

NotecardConnectionHandler::NotecardConnectionHandler(
  const String & project_uid_,
  HardwareSerial & serial_,
  uint32_t baud_,
  bool keep_alive_
) :
  ConnectionHandler{keep_alive_, NetworkAdapter::NOTECARD},
  _notecard{},
  _device_id{},
  _notecard_uid{},
  _project_uid(project_uid_),
  _serial(&serial_),
  _wire(nullptr),
  _inbound_buffer(nullptr),
  _conn_start_ms(0),
  _i2c_address(0),
  _i2c_max(0),
  _inbound_buffer_index(0),
  _inbound_buffer_size(0),
  _inbound_polling_interval_min(NO_INBOUND_POLLING),
  _uart_baud(baud_),
  _en_hw_int(false),
  _topic_type{TopicType::Invalid}
{ }

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int NotecardConnectionHandler::initiateNotehubSync (SyncType type_) const
{
  int result;

  DEBUG_DEBUG(F("NotecardConnectionHandler::%s initiating Notehub sync..."), __FUNCTION__);
  if (J *req = _notecard.newRequest("hub.sync")) {
    if (type_ == SyncType::Inbound) {
      JAddBoolToObject(req, "in", true);
    } else if (type_ == SyncType::Outbound) {
      JAddBoolToObject(req, "out", true);
    }
    if (J *rsp = _notecard.requestAndResponse(req)) {
      // Check the response for errors
      if (NoteResponseError(rsp)) {
        const char *err = JGetString(rsp, "err");
        DEBUG_ERROR(F("%s"), err);
        result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
      } else {
        DEBUG_DEBUG(F("NotecardConnectionHandler::%s successfully initiated Notehub sync."), __FUNCTION__);
        result = NotecardCommunicationError::NOTECARD_ERROR_NONE;
      }
      JDelete(rsp);
    } else {
      DEBUG_ERROR(F("Failed to receive response from Notecard."));
      result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
    }
  } else {
    DEBUG_ERROR("Failed to allocate request: hub.sync");
    result = NotecardCommunicationError::HOST_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

int NotecardConnectionHandler::setWiFiCredentials (const String & ssid_, const String & password_)
{
  int result;

  // Validate the connection state is not in an initialization state
  const NetworkConnectionState current_net_connection_state = check();
  if (NetworkConnectionState::INIT == current_net_connection_state)
  {
    DEBUG_ERROR(F("Unable to set Wi-Fi credentials. Connection to Notecard uninitialized."));
    result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
  } else if (J *req = _notecard.newRequest("card.wifi")) {
    JAddStringToObject(req, "ssid", ssid_.c_str());
    JAddStringToObject(req, "password", password_.c_str());
    if (J *rsp = _notecard.requestAndResponse(req)) {
      // Check the response for errors
      if (NoteResponseError(rsp)) {
        const char *err = JGetString(rsp, "err");
        DEBUG_ERROR(F("%s"), err);
        DEBUG_ERROR(F("Failed to set Wi-Fi credentials."));
        result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
      } else {
        DEBUG_INFO(F("Wi-Fi credentials updated. ssid: \"%s\" password: \"%s\"."), ssid_.c_str(), password_.length() ? "**********" : "");
        result = NotecardCommunicationError::NOTECARD_ERROR_NONE;
      }
      JDelete(rsp);
    } else {
      DEBUG_ERROR(F("Failed to receive response from Notecard."));
      result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
    }
  } else {
    DEBUG_ERROR(F("Failed to allocate request: wifi.set"));
    result = NotecardCommunicationError::HOST_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

/******************************************************************************
   PUBLIC INTERFACE MEMBER FUNCTIONS
 ******************************************************************************/

bool NotecardConnectionHandler::available()
{
  bool buffered_data = (_inbound_buffer_index < _inbound_buffer_size);
  bool flush_required = !buffered_data && _inbound_buffer_size;

  // When the buffer is empty, look for a Note in the
  // NOTEFILE_SECURE_INBOUND file to reload the buffer.
  if (!buffered_data) {
    // Reset the buffer
    free(_inbound_buffer);
    _inbound_buffer = nullptr;
    _inbound_buffer_index = 0;
    _inbound_buffer_size = 0;

    // Do NOT attempt to buffer the next Note immediately after buffer
    // exhaustion (a.k.a. flush required). Returning `false` between Notes,
    // will break the read loop, force the CBOR buffer to be parsed, and the
    // property containers to be updated.
    if (!flush_required) {
      // Reload the buffer
      J *note = getNote(true);
      if (note) {
        if (J *body = JGetObject(note, "body")) {
          _topic_type = static_cast<TopicType>(JGetInt(body, "topic"));
          if (_topic_type == TopicType::Invalid) {
            DEBUG_WARNING(F("Note does not contain a topic"));
          } else {
            buffered_data = JGetBinaryFromObject(note, "payload", &_inbound_buffer, &_inbound_buffer_size);
            if (!buffered_data) {
              DEBUG_WARNING(F("Note does not contain payload data"));
            } else {
              DEBUG_DEBUG(F("NotecardConnectionHandler::%s buffered payload with size: %d"), __FUNCTION__, _inbound_buffer_size);
            }
          }
        } else {
          _topic_type = TopicType::Invalid;
        }
        JDelete(note);
      }
    }
  }

  return buffered_data;
}

unsigned long NotecardConnectionHandler::getTime()
{
  unsigned long result;

  if (J *rsp = _notecard.requestAndResponse(_notecard.newRequest("card.time"))) {
    if (NoteResponseError(rsp)) {
      const char *err = JGetString(rsp, "err");
      DEBUG_ERROR(F("%s\n"), err);
      result = 0;
    } else {
      result = JGetInt(rsp, "time");
    }
    JDelete(rsp);
  } else {
    result = 0;
  }

  return result;
}

int NotecardConnectionHandler::read()
{
  int result;

  if (_inbound_buffer_index < _inbound_buffer_size) {
    result = _inbound_buffer[_inbound_buffer_index++];
  } else {
    result = NotecardCommunicationError::NOTECARD_ERROR_NO_DATA_AVAILABLE;
  }

  return result;
}

int NotecardConnectionHandler::write(const uint8_t * buf_, size_t size_)
{
  int result;

  // Validate the connection state is not uninitialized or in error state
  const NetworkConnectionState current_net_connection_state = check();
  if ((NetworkConnectionState::INIT == current_net_connection_state)
  || (NetworkConnectionState::ERROR == current_net_connection_state))
  {
    DEBUG_ERROR(F("Unable to write message. Connection to Notecard uninitialized or in error state."));
    result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
  } else if (J * req = _notecard.newRequest("note.add")) {
    JAddStringToObject(req, "file", NOTEFILE_SECURE_OUTBOUND);
    if (buf_) {
      JAddBinaryToObject(req, "payload", buf_, size_);
    }
    // Queue the Note when `_keep_alive` is disabled or not connected to Notehub
    if (_keep_alive && (NetworkConnectionState::CONNECTED == current_net_connection_state)) {
      JAddBoolToObject(req, "live", true);
      JAddBoolToObject(req, "sync", true);
    }
    if (J *body = JAddObjectToObject(req, "body")) {
      JAddIntToObject(body, "topic", static_cast<int>(_topic_type));
      J * rsp = _notecard.requestAndResponse(req);
      if (NoteResponseError(rsp)) {
        const char *err = JGetString(rsp, "err");
        if (NoteErrorContains(err, "{hub-not-connected}")) {
          // _current_net_connection_state = NetworkConnectionState::DISCONNECTED;
        }
        DEBUG_ERROR(F("%s\n"), err);
        result = NotecardCommunicationError::NOTECARD_ERROR_GENERIC;
      } else {
        result = NotecardCommunicationError::NOTECARD_ERROR_NONE;
        DEBUG_INFO(F("Message sent correctly!"));
      }
      JDelete(rsp);
    } else {
      JFree(req);
      result = NotecardCommunicationError::HOST_ERROR_OUT_OF_MEMORY;
    }
  } else {
    result = NotecardCommunicationError::HOST_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

/******************************************************************************
   PROTECTED STATE MACHINE FUNCTIONS
 ******************************************************************************/

NetworkConnectionState NotecardConnectionHandler::update_handleInit()
{
  NetworkConnectionState result = NetworkConnectionState::INIT;

 // Configure Hardware
///////////////////////

#if defined(STLINK_DEBUG)
  // Output Notecard logs to the STLINK serial port
  stlinkSerial.end();  // necessary to handle multiple initializations (e.g. reconnections)
  stlinkSerial.begin(115200);
  const size_t usb_timeout_ms = 3000;
  for (const size_t start_ms = millis(); !stlinkSerial && (millis() - start_ms) < usb_timeout_ms;);
  _notecard.setDebugOutputStream(stlinkSerial);
#endif

  // Initialize the Notecard based on the configuration
  if (_serial) {
    _notecard.begin(*_serial, _uart_baud);
  } else {
    _notecard.begin(_i2c_address, _i2c_max, *_wire);
  }

   // Configure `note-c`
  ///////////////////////

  // Set the user agent
  NoteSetUserAgent((char *) ("arduino-iot-cloud " NOTECARD_CONNECTION_HANDLER_VERSION));

  // Configure the ATTN pin to be used as an interrupt to indicate when a Note
  // is available to read. `getNote()` will only arm the interrupt if no old
  // Notes are available. If `ATTN` remains unarmed, it signals the user
  // application that outstanding Notes are queued and need to be processed.
  if (J *note = getNote(false)) {
    JDelete(note);
  }

   // Configure the Notecard
  ///////////////////////////

  // Set the project UID
  if (NetworkConnectionState::INIT == result) {
    if (configureConnection(true)) {
      result = NetworkConnectionState::INIT;
    } else {
      result = NetworkConnectionState::ERROR;
    }
  }

#if defined(ARDUINO_OPTA)
  // The Opta Extension has an onboard Li-Ion capacitor, that can be utilized
  // to monitor the power state of the device and automatically report loss of
  // power to Notehub. The following command enables that detection by default
  // for the Opta Wirelss Extension.
  if (NetworkConnectionState::INIT == result) {
    if (J *req = _notecard.newRequest("card.voltage")) {
      JAddStringToObject(req, "mode", "lic");
      JAddBoolToObject(req, "alert", true);
      JAddBoolToObject(req, "sync", true);
      JAddBoolToObject(req, "usb", true);
      if (J *rsp = _notecard.requestAndResponse(req)) {
        // Check the response for errors
        if (NoteResponseError(rsp)) {
          const char *err = JGetString(rsp, "err");
          DEBUG_ERROR(F("%s"), err);
          result = NetworkConnectionState::ERROR;
        } else {
          result = NetworkConnectionState::INIT;
        }
        JDelete(rsp);
      } else {
        DEBUG_ERROR(F("Failed to receive response from Notecard."));
        result = NetworkConnectionState::ERROR; // Assume the worst
      }
    } else {
      DEBUG_ERROR("Failed to allocate request: card.voltage");
      result = NetworkConnectionState::ERROR; // Assume the worst
    }
  }
#endif

  // Set inbound template to support LoRa/Satellite Notecard
  if (NetworkConnectionState::INIT == result) {
    if (J *req = _notecard.newRequest("note.template")) {
      JAddStringToObject(req, "file", NOTEFILE_SECURE_INBOUND);
      JAddStringToObject(req, "format", "compact");              // Support LoRa/Satellite Notecards
      JAddIntToObject(req, "port", NOTEFILE_INBOUND_LORA_PORT);  // Support LoRa/Satellite Notecards
      if (J *body = JAddObjectToObject(req, "body")) {
        JAddIntToObject(body, "topic", TUINT8);
        if (J *rsp = _notecard.requestAndResponse(req)) {
          // Check the response for errors
          if (NoteResponseError(rsp)) {
            const char *err = JGetString(rsp, "err");
            DEBUG_ERROR(F("%s"), err);
            result = NetworkConnectionState::ERROR;
          } else {
            result = NetworkConnectionState::INIT;
          }
          JDelete(rsp);
        } else {
          DEBUG_ERROR(F("Failed to receive response from Notecard."));
          result = NetworkConnectionState::ERROR; // Assume the worst
        }
      } else {
        DEBUG_ERROR("Failed to allocate request: note.template:body");
        JFree(req);
        result = NetworkConnectionState::ERROR; // Assume the worst
      }
    } else {
      DEBUG_ERROR("Failed to allocate request: note.template");
      result = NetworkConnectionState::ERROR; // Assume the worst
    }
  }

  // Set outbound template to remove payload size restrictions
  if (NetworkConnectionState::INIT == result) {
    if (J *req = _notecard.newRequest("note.template")) {
      JAddStringToObject(req, "file", NOTEFILE_SECURE_OUTBOUND);
      JAddStringToObject(req, "format", "compact");               // Support LoRa/Satellite Notecards
      JAddIntToObject(req, "port", NOTEFILE_OUTBOUND_LORA_PORT);  // Support LoRa/Satellite Notecards
      if (J *body = JAddObjectToObject(req, "body")) {
        JAddIntToObject(body, "topic", TUINT8);
        if (J *rsp = _notecard.requestAndResponse(req)) {
          // Check the response for errors
          if (NoteResponseError(rsp)) {
            const char *err = JGetString(rsp, "err");
            DEBUG_ERROR(F("%s"), err);
            result = NetworkConnectionState::ERROR;
          } else {
            result = NetworkConnectionState::INIT;
          }
          JDelete(rsp);
        } else {
          DEBUG_ERROR(F("Failed to receive response from Notecard."));
          result = NetworkConnectionState::ERROR; // Assume the worst
        }
      } else {
        DEBUG_ERROR("Failed to allocate request: note.template:body");
        JFree(req);
        result = NetworkConnectionState::ERROR; // Assume the worst
      }
    } else {
      DEBUG_ERROR("Failed to allocate request: note.template");
      result = NetworkConnectionState::ERROR; // Assume the worst
    }
  }

  // Get the device UID
  if (NetworkConnectionState::INIT == result) {
    if (!updateUidCache()) {
      result = NetworkConnectionState::ERROR;
    } else {
      DEBUG_INFO(F("Notecard has been initialized."));
      if (_keep_alive) {
        _conn_start_ms = ::millis();
        DEBUG_INFO(F("Starting network connection..."));
        result = NetworkConnectionState::CONNECTING;
      } else {
        DEBUG_INFO(F("Network is disconnected."));
        result = NetworkConnectionState::DISCONNECTED;
      }
    }
  }

  return result;
}

NetworkConnectionState NotecardConnectionHandler::update_handleConnecting()
{
  NetworkConnectionState result;

  // Check the connection status
  const NotecardConnectionStatus conn_status = connected();

  // Update the connection state
  if (!conn_status.connected_to_notehub) {
    if ((::millis() - _conn_start_ms) > NOTEHUB_CONN_TIMEOUT_MS) {
      DEBUG_ERROR(F("Timeout exceeded, connection to the network failed."));
      DEBUG_INFO(F("Retrying in \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
      result = NetworkConnectionState::INIT;
    } else {
      // Continue awaiting the connection to Notehub
      if (conn_status.transport_connected) {
        DEBUG_INFO(F("Establishing connection to Notehub..."));
      } else {
        DEBUG_INFO(F("Connecting to the network..."));
      }
      result = NetworkConnectionState::CONNECTING;
    }
  } else {
    DEBUG_INFO(F("Connected to Notehub!"));
    result = NetworkConnectionState::CONNECTED;
    if (initiateNotehubSync()) {
      DEBUG_ERROR(F("Failed to initiate Notehub sync."));
    }
  }

  return result;
}

NetworkConnectionState NotecardConnectionHandler::update_handleConnected()
{
  NetworkConnectionState result;

  const NotecardConnectionStatus conn_status = connected();
  if (!conn_status.connected_to_notehub) {
    if (!conn_status.transport_connected) {
      DEBUG_ERROR(F("Connection to the network lost."));
    } else {
      DEBUG_ERROR(F("Connection to Notehub lost."));
    }
    result = NetworkConnectionState::DISCONNECTED;
  } else {
    result = NetworkConnectionState::CONNECTED;
  }

  return result;
}

NetworkConnectionState NotecardConnectionHandler::update_handleDisconnecting()
{
  NetworkConnectionState result;

  DEBUG_ERROR(F("Connection to the network lost."));
  result = NetworkConnectionState::DISCONNECTED;

  return result;
}

NetworkConnectionState NotecardConnectionHandler::update_handleDisconnected()
{
  NetworkConnectionState result;

  if (_keep_alive)
  {
    DEBUG_ERROR(F("Attempting reconnection..."));
    result = NetworkConnectionState::INIT;
  }
  else
  {
    if (configureConnection(false)) {
      result = NetworkConnectionState::CLOSED;
      DEBUG_INFO(F("Closing connection..."));
    } else {
      result = NetworkConnectionState::ERROR;
      DEBUG_INFO(F("Error closing connection..."));
    }
  }

  return result;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

bool NotecardConnectionHandler::armInterrupt (void) const
{
  bool result;

  if (J *req = _notecard.newRequest("card.attn")) {
    JAddStringToObject(req, "mode","rearm,files");
    if (J *files = JAddArrayToObject(req, "files")) {
      JAddItemToArray(files, JCreateString(NOTEFILE_SECURE_INBOUND));
      if (J *rsp = _notecard.requestAndResponse(req)) {
        // Check the response for errors
        if (NoteResponseError(rsp)) {
          const char *err = JGetString(rsp, "err");
          DEBUG_ERROR(F("%s\n"), err);
          result = false;
        } else {
          result = true;
        }
        JDelete(rsp);
      } else {
        DEBUG_ERROR(F("Failed to receive response from Notecard."));
        result = false;
      }
    } else {
      DEBUG_ERROR("Failed to allocate request: card.attn:files");
      JFree(req);
      result = false;
    }
  } else {
    DEBUG_ERROR("Failed to allocate request: card.attn");
    result = false;
  }

  return result;
}

bool NotecardConnectionHandler::configureConnection (bool connect_) const
{
  bool result;

  if (J *req = _notecard.newRequest("hub.set")) {
    // Only update the product if it is not empty or the default value
    if (_project_uid.length() > 0 && _project_uid != "com.domain.you:product") {
      JAddStringToObject(req, "product", _project_uid.c_str());
    }

    // Configure the connection mode based on the `connect_` parameter
    if (connect_) {
      JAddStringToObject(req, "mode", "continuous");
      JAddIntToObject(req, "inbound", _inbound_polling_interval_min);
      JAddBoolToObject(req, "sync", true);
    } else {
      JAddStringToObject(req, "mode", "periodic");
      JAddIntToObject(req, "inbound", NO_INBOUND_POLLING);
      JAddIntToObject(req, "outbound", -1);
      JAddStringToObject(req, "vinbound", "-");
      JAddStringToObject(req, "voutbound", "-");
    }

    // Send the request to the Notecard
    if (J *rsp = _notecard.requestAndResponseWithRetry(req, 30)) {
      // Check the response for errors
      if (NoteResponseError(rsp)) {
        const char *err = JGetString(rsp, "err");
        DEBUG_ERROR(F("%s"), err);
        result = false;
      } else {
        result = true;
      }
      JDelete(rsp);
    } else {
      DEBUG_ERROR(F("Failed to receive response from Notecard."));
      result = false; // Assume the worst
    }
  } else {
    DEBUG_ERROR("Failed to allocate request: hub.set");
    result = false; // Assume the worst
  }

  return result;
}

uint_fast8_t NotecardConnectionHandler::connected (void) const
{
  NotecardConnectionStatus result;

  // Query the connection status from the Notecard
  if (J *rsp = _notecard.requestAndResponse(_notecard.newRequest("hub.status"))) {
    // Ensure the transaction doesn't return an error
    if (NoteResponseError(rsp)) {
      const char *err = JGetString(rsp, "err");
      DEBUG_ERROR(F("%s"),err);
      result.notecard_error = true;
    } else {
      // Parse the transport connection status
      result.transport_connected = (strstr(JGetString(rsp,"status"),"{connected}") != nullptr);

      // Parse the status of the connection to Notehub
      result.connected_to_notehub = JGetBool(rsp,"connected");

      // Set the Notecard error status
      result.notecard_error = false;
      result.host_error = false;
    }

    // Free the response
    JDelete(rsp);
  } else {
    DEBUG_ERROR(F("Failed to acquire Notecard connection status."));
    result.transport_connected = false;
    result.connected_to_notehub = false;
    result.notecard_error = false;
    result.host_error = true;
  }

  return result;
}

J * NotecardConnectionHandler::getNote (bool pop_) const
{
  J * result;

  // Look for a Note in the NOTEFILE_SECURE_INBOUND file
  if (J *req = _notecard.newRequest("note.get")) {
    JAddStringToObject(req, "file", NOTEFILE_SECURE_INBOUND);
    if (pop_) {
      JAddBoolToObject(req, "delete", true);
    }
    if (J *note = _notecard.requestAndResponse(req)) {
      // Ensure the transaction doesn't return an error
      if (NoteResponseError(note)) {
        const char *jErr = JGetString(note, "err");
        if (NoteErrorContains(jErr, "{note-noexist}")) {
          // The Notefile is empty, thus no Note is available.
          if (_en_hw_int) {
            armInterrupt();
          }
        } else {
          // Any other error indicates that we were unable to
          // retrieve a Note, therefore no Note is available.
        }
        result = nullptr;
        JDelete(note);
      } else {
        // The Note was successfully retrieved, and it now
        // becomes the callers responsibility to free it.
        result = note;
      }
    } else {
      DEBUG_ERROR(F("Failed to receive response from Notecard."));
      result = nullptr;
    }
  } else {
    DEBUG_ERROR("Failed to allocate request: note.get");
    // Failed to retrieve a Note, therefore no Note is available.
    result = nullptr;
  }

  return result;
}

bool NotecardConnectionHandler::updateUidCache (void)
{
  bool result;

  // This operation is safe to perform before a sync has occurred, because the
  // Notecard UID is static and the cloud value of Serial Number is strictly
  // informational with regard to the host firmware operations.

  // Read the Notecard UID from the Notehub configuration
  if (J *rsp = _notecard.requestAndResponse(_notecard.newRequest("hub.get"))) {
    // Check the response for errors
    if (NoteResponseError(rsp)) {
      const char *err = JGetString(rsp, "err");
      DEBUG_ERROR(F("Failed to read Notecard UID"));
      DEBUG_ERROR(F("Error: %s"), err);
      result = false;
    } else {
      _notecard_uid = JGetString(rsp, "device");
      char device_id[] = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
      if (NoteGetEnv("_arduino_device_id", device_id, device_id, sizeof(device_id))) {
        _device_id = device_id;
      } else {
        DEBUG_DEBUG(F("NotecardConnectionHandler::%s Arduino Device ID not cached on Notecard, using default value: <%s>"), __FUNCTION__, _device_id.c_str());
      }
      DEBUG_DEBUG(F("NotecardConnectionHandler::%s updated local cache with Notecard UID: <%s> and Arduino Device ID: <%s>"), __FUNCTION__, _notecard_uid.c_str(), _device_id.c_str());
      result = true;
    }
    JDelete(rsp);
  } else {
    DEBUG_ERROR(F("Failed to read Notecard UID"));
    result = false;
  }

  return result;
}

#endif /* BOARD_HAS_NOTECARD */
