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

    #if not defined(BOARD_HAS_LORA)
      virtual unsigned long getTime() = 0;
    #endif

    #if defined(BOARD_HAS_NOTECARD) || defined(BOARD_HAS_LORA)
      virtual bool available() = 0;
      virtual int read() = 0;
      virtual int write(const uint8_t *buf, size_t size) = 0;
    #else
      virtual Client &getClient() = 0;
      virtual UDP &getUDP() = 0;
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

    virtual void setKeepAlive(bool keep_alive=true) { this->_keep_alive = keep_alive; }

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

  private:

    unsigned long _lastConnectionTickTime;
    NetworkConnectionState _current_net_connection_state;
    OnNetworkEventCallback  _on_connect_event_callback = NULL,
                            _on_disconnect_event_callback = NULL,
                            _on_error_event_callback = NULL;

    friend GenericConnectionHandler;
};

