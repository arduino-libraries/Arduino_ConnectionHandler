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

#ifndef ARDUINO_LORA_CONNECTION_HANDLER_H_
#define ARDUINO_LORA_CONNECTION_HANDLER_H_

//#ifdef defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Arduino_LPWANConnectionHandler.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class LoRaConnectionHandler : public LPWANConnectionHandler {
  public:
    LoRaConnectionHandler(const char *_appeui, const char *_appkey, _lora_band = EU868);

    void init();
    unsigned long getTime();
    void check() {
      update();
    }
    void update();

    int write(const uint8_t *buf, size_t size);
    int read();
    bool available();

    void disconnect();
    void connect();

    virtual void addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback);
    virtual void addConnectCallback(OnNetworkEventCallback callback);
    virtual void addDisconnectCallback(OnNetworkEventCallback callback);
    virtual void addErrorCallback(OnNetworkEventCallback callback);

  private:

    void changeConnectionState(NetworkConnectionState _newState);

    const int CHECK_INTERVAL_IDLE = 100;
    const int CHECK_INTERVAL_INIT = 100;
    const int CHECK_INTERVAL_CONNECTING = 500;
    const int CHECK_INTERVAL_CONNECTED = 10000;
    const int CHECK_INTERVAL_RETRYING = 5000;
    const int CHECK_INTERVAL_DISCONNECTING = 500;
    const int CHECK_INTERVAL_DISCONNECTED = 1000;
    const int CHECK_INTERVAL_ERROR = 500;

    LoRaModem modem;
    const char *appeui, *appkey;
    _lora_band band;
    unsigned long lastConnectionTickTime;

    int connectionTickTimeInterval;

    bool keepAlive;

    OnNetworkEventCallback  _on_connect_event_callback,
                            _on_disconnect_event_callback,
                            _on_error_event_callback;

    static void execNetworkEventCallback(OnNetworkEventCallback & callback, void * callback_arg);

};

//#endif

#endif /* ARDUINO_LORA_CONNECTION_HANDLER_H_ */
