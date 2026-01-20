/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
  INCLUDES
 ******************************************************************************/

#if !defined(__AVR__)
#  include <Arduino_DebugUtils.h>
#endif

#include <Arduino.h>
#include <Client.h>
#include <Udp.h>
#include "ConnectionHandlerDefinitions.h"
#include "connectionHandlerModels/settings.h"

#include <utility>

/******************************************************************************
  TYPEDEFS
 ******************************************************************************/

typedef void (*OnNetworkEventCallback)();

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

// forward declaration FIXME
class GenericConnectionHandler;

class ConnectionHandler {
  public:

    ConnectionHandler(bool const keep_alive=true, NetworkAdapter interface=NetworkAdapter::NONE);

    virtual ~ConnectionHandler() {}

    virtual NetworkConnectionState check();

    #if defined(BOARD_HAS_LORA)
      virtual bool available() = 0;
      virtual int read() = 0;
      virtual int write(const uint8_t *buf, size_t size) = 0;
    #else
      virtual Client &getClient() = 0;
      virtual UDP &getUDP() = 0;

      virtual unsigned long getTime() = 0;

      virtual int ping(IPAddress ip, uint8_t ttl = 128, uint8_t count = 1) = 0;
      virtual int ping(const String &hostname, uint8_t ttl = 128, uint8_t count = 1) = 0;
      virtual int ping(const char* host, uint8_t ttl = 128, uint8_t count = 1) = 0;
    #endif

    NetworkConnectionState getStatus() __attribute__((deprecated)) {
      return _current_net_connection_state;
    }

    NetworkAdapter getInterface() {
      return _interface;
    }

    virtual void connect();
    virtual void disconnect();
    void enableCheckInternetAvailability(bool enable) {
      _check_internet_availability = enable;
    }

    virtual void addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback);
    void addConnectCallback(OnNetworkEventCallback callback) __attribute__((deprecated));
    void addDisconnectCallback(OnNetworkEventCallback callback) __attribute__((deprecated));
    void addErrorCallback(OnNetworkEventCallback callback) __attribute__((deprecated));

    /**
     * Update the interface settings. This can be performed only when the interface is
     * in INIT state. otherwise nothing is performed. The type of the interface should match
     * the type of the settings provided
     *
     * @return true if the update is successful, false otherwise
     */
    virtual bool updateSetting(const models::NetworkSetting& s) {
      if(_current_net_connection_state == NetworkConnectionState::INIT && s.type == _interface) {
        memcpy(&_settings, &s, sizeof(s));
        return true;
      }

      return false;
    }

    virtual void getSetting(models::NetworkSetting& s) {
      memcpy(&s, &_settings, sizeof(s));
      return;
    }

    virtual void setKeepAlive(bool keep_alive=true) { this->_keep_alive = keep_alive; }

    inline void updateTimeoutTable(const TimeoutTable& t) { _timeoutTable = t; }
    inline void updateTimeoutTable(TimeoutTable&& t)      { _timeoutTable = std::move(t); }
    inline void updateTimeoutInterval(NetworkConnectionState state, uint32_t interval) {
      _timeoutTable.intervals[static_cast<unsigned int>(state)] = interval;
    }
  protected:

    virtual NetworkConnectionState updateConnectionState();
    virtual void updateCallback(NetworkConnectionState next_net_connection_state);

    bool _keep_alive;
    bool _check_internet_availability;
    NetworkAdapter _interface;

    virtual NetworkConnectionState update_handleInit         () = 0;
    virtual NetworkConnectionState update_handleConnecting   () = 0;
    virtual NetworkConnectionState update_handleConnected    () = 0;
    virtual NetworkConnectionState update_handleDisconnecting() = 0;
    virtual NetworkConnectionState update_handleDisconnected () = 0;

    models::NetworkSetting _settings;

    TimeoutTable _timeoutTable;
  private:

    unsigned long _lastConnectionTickTime;
    NetworkConnectionState _current_net_connection_state;
    OnNetworkEventCallback  _on_connect_event_callback = NULL,
                            _on_disconnect_event_callback = NULL,
                            _on_error_event_callback = NULL;

    friend GenericConnectionHandler;
};
