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

#ifndef ARDUINO_CONNECTION_HANDLER_H_
#define ARDUINO_CONNECTION_HANDLER_H_

#include <Arduino.h>

#ifdef ARDUINO_SAMD_MKR1000
  #include <WiFi101.h>
  #include <WiFiUdp.h>

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || \
  defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined (ARDUINO_NANO_RP2040_CONNECT)
  #include <WiFiNINA.h>
  #include <WiFiUdp.h>

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_MODULE
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_LATEST_VERSION
#endif

#if defined(ARDUINO_PORTENTA_H7_M7)
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include <Ethernet.h>
  #include <PortentaEthernet.h>

  #define BOARD_HAS_WIFI
  #define BOARD_HAS_ETHERNET
  #define BOARD_HAS_PORTENTA_VISION_SHIELD_ETHERNET
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_NICLA_VISION)
  #include <WiFi.h>
  #include <WiFiUdp.h>

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#ifdef ARDUINO_SAMD_MKRGSM1400
  #include <MKRGSM.h>
  #define BOARD_HAS_GSM
  #define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
  #define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
#endif

#ifdef ARDUINO_SAMD_MKRNB1500
  #include <MKRNB.h>
  #define BOARD_HAS_NB
  #define NETWORK_HARDWARE_ERROR
  #define NETWORK_IDLE_STATUS NB_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED NB_NetworkStatus_t::GPRS_READY
#endif

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  #include <MKRWAN.h>
  #define BOARD_HAS_LORA
#endif

#if defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <WiFiUdp.h>
  
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

/******************************************************************************
   INCLUDES
 ******************************************************************************/

#if !defined(__AVR__)
#  include <Arduino_DebugUtils.h>
#endif

/******************************************************************************
   TYPEDEFS
 ******************************************************************************/

enum class NetworkConnectionState : unsigned int {
  INIT          = 0,
  CONNECTING    = 1,
  CONNECTED     = 2,
  DISCONNECTING = 3,
  DISCONNECTED  = 4,
  CLOSED        = 5,
  ERROR         = 6
};

enum class NetworkConnectionEvent {
  CONNECTED,
  DISCONNECTED,
  ERROR
};

enum class NetworkAdapter {
  WIFI,
  ETHERNET,
  NB,
  GSM,
  LORA
};

typedef void (*OnNetworkEventCallback)();

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static unsigned int const CHECK_INTERVAL_TABLE[] =
{
  /* INIT          */ 100,
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  /* CONNECTING    */ 4000,
#else
  /* CONNECTING    */ 500,
#endif
  /* CONNECTED     */ 10000,
  /* DISCONNECTING */ 100,
  /* DISCONNECTED  */ 1000,
  /* CLOSED        */ 1000,
  /* ERROR         */ 1000
};

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ConnectionHandler {
  public:

    ConnectionHandler(bool const keep_alive, NetworkAdapter interface);


    NetworkConnectionState check();

    #if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB) || defined(BOARD_HAS_ETHERNET)
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

#if defined(BOARD_HAS_WIFI)
  #include "Arduino_WiFiConnectionHandler.h"
#elif defined(BOARD_HAS_GSM)
  #include "Arduino_GSMConnectionHandler.h"
#elif defined(BOARD_HAS_NB)
  #include "Arduino_NBConnectionHandler.h"
#elif defined(BOARD_HAS_LORA)
  #include "Arduino_LoRaConnectionHandler.h"
#endif

#if defined(BOARD_HAS_ETHERNET)
  #include "Arduino_EthernetConnectionHandler.h"
#endif

#endif /* CONNECTION_HANDLER_H_ */
