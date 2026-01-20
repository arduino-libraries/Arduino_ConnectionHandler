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

#include <Arduino.h>

#ifdef ARDUINO_SAMD_MKR1000
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if (defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || \
  defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined (ARDUINO_NANO_RP2040_CONNECT)) && \
  !defined(ARDUINO_ARCH_ZEPHYR)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_MODULE
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_LATEST_VERSION
#endif

#if defined(ARDUINO_PORTENTA_H7_M7) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_WIFI
  #define BOARD_HAS_ETHERNET
  #define BOARD_HAS_CATM1_NBIOT
  #define BOARD_HAS_CELLULAR
  #define BOARD_HAS_PORTENTA_CATM1_NBIOT_SHIELD
  #define BOARD_HAS_PORTENTA_VISION_SHIELD_ETHERNET
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_PORTENTA_C33) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_WIFI
  #define BOARD_HAS_ETHERNET
  #define BOARD_HAS_CELLULAR
  #define BOARD_HAS_PORTENTA_VISION_SHIELD_ETHERNET
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_NICLA_VISION) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_OPTA) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_WIFI
  #define BOARD_HAS_ETHERNET
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_GIGA) && !defined(ARDUINO_ARCH_ZEPHYR)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_SAMD_MKRGSM1400) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_GSM
  #define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
  #define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
#endif

#if defined(ARDUINO_SAMD_MKRNB1500) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_NB
  #define NETWORK_HARDWARE_ERROR
  #define NETWORK_IDLE_STATUS NB_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED NB_NetworkStatus_t::GPRS_READY
#endif

#if (defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)) && \
  !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_LORA
#endif

#if defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ZEPHYR)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_ARCH_ZEPHYR)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_LATEST_VERSION
#endif

#if defined(ARDUINO_EDGE_CONTROL) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_CATM1_NBIOT
  #define BOARD_HAS_PORTENTA_CATM1_NBIOT_SHIELD
  #define NETWORK_HARDWARE_ERROR
#endif

#if defined(ARDUINO_RASPBERRY_PI_PICO_W) && !defined(ARDUINO_ARCH_ZEPHYR)
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_ARCH_ZEPHYR)
  #if CONFIG_WIFI
    #define BOARD_HAS_WIFI
  #endif

  #if DT_HAS_COMPAT_STATUS_OKAY(ethernet_phy)
    #define BOARD_HAS_ETHERNET
  #endif

  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if !defined(BOARD_HAS_WIFI) && \
  !defined(BOARD_HAS_ETHERNET) && \
  !defined(BOARD_HAS_CATM1_NBIOT) && \
  !defined(BOARD_HAS_CELLULAR) && \
  !defined(BOARD_HAS_GSM) && \
  !defined(BOARD_HAS_LORA) && \
  !defined(BOARD_HAS_NB) && \
  !defined(BOARD_HAS_PORTENTA_CATM1_NBIOT_SHIELD) && \
  !defined(BOARD_HAS_PORTENTA_VISION_SHIELD_ETHERNET)

  #error "Current board doesn't support any ConnectionHandler"
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
  NONE,
  WIFI,
  ETHERNET,
  NB,
  GSM,
  LORA,
  CATM1,
  CELL
};

union TimeoutTable {
  struct {
    // Note: order of the following values must be preserved
    // and match against NetworkConnectionState values
    uint32_t init;
    uint32_t connecting;
    uint32_t connected;
    uint32_t disconnecting;
    uint32_t disconnected;
    uint32_t closed;
    uint32_t error;
  } timeout;
  uint32_t intervals[sizeof(timeout) / sizeof(uint32_t)];
};

/******************************************************************************
  CONSTANTS
 ******************************************************************************/

constexpr TimeoutTable DefaultTimeoutTable {
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  4000,   // init
#else
  500,    // init
#endif
  500,    // connecting
  10000,  // connected
  100,    // disconnecting
  1000,   // disconnected
  1000,   // closed
  1000,   // error
};
