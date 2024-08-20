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
#include "definitions/ConnectionHandlerDefinitions.h"

/******************************************************************************
   TYPEDEFS
 ******************************************************************************/

typedef void (*OnNetworkEventCallback)();

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ConnectionHandler {
  public:

    ConnectionHandler(bool const keep_alive, NetworkAdapter interface);


    NetworkConnectionState check();

    #if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB) || defined(BOARD_HAS_ETHERNET) || defined(BOARD_HAS_CATM1_NBIOT)
      virtual unsigned long getTime() = 0;
      virtual Client &getClient() = 0;
      virtual UDP &getUDP() = 0;
    #endif

    #if defined(BOARD_HAS_LORA)
      virtual int write(const uint8_t *buf, size_t size) = 0;
      virtual int read() = 0;
      virtual bool available() = 0;
    #endif

    NetworkConnectionState getStatus() __attribute__((deprecated)) {
      return _current_net_connection_state;
    }

    NetworkAdapter getInterface() {
      return _interface;
    }

    void connect();
    void disconnect();

    void addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback);
    void addConnectCallback(OnNetworkEventCallback callback) __attribute__((deprecated));
    void addDisconnectCallback(OnNetworkEventCallback callback) __attribute__((deprecated));
    void addErrorCallback(OnNetworkEventCallback callback) __attribute__((deprecated));

  protected:

    bool _keep_alive;
    NetworkAdapter _interface;

    virtual NetworkConnectionState update_handleInit         () = 0;
    virtual NetworkConnectionState update_handleConnecting   () = 0;
    virtual NetworkConnectionState update_handleConnected    () = 0;
    virtual NetworkConnectionState update_handleDisconnecting() = 0;
    virtual NetworkConnectionState update_handleDisconnected () = 0;


  private:

    unsigned long _lastConnectionTickTime;
    NetworkConnectionState _current_net_connection_state;
    OnNetworkEventCallback  _on_connect_event_callback = NULL,
                            _on_disconnect_event_callback = NULL,
                            _on_error_event_callback = NULL;
};

