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

#if defined __has_include
  #if __has_include (<Notecard.h>)
    #define BOARD_HAS_NOTECARD
  #endif
#endif

#include <Arduino.h>

#ifndef BOARD_HAS_NOTECARD

#ifdef ARDUINO_SAMD_MKR1000
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || \
  defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined (ARDUINO_NANO_RP2040_CONNECT)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_MODULE
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_LATEST_VERSION
#endif

#if defined(ARDUINO_PORTENTA_H7_M7)
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

#if defined(ARDUINO_PORTENTA_C33)
  #define BOARD_HAS_WIFI
  #define BOARD_HAS_ETHERNET
  #define BOARD_HAS_CELLULAR
  #define BOARD_HAS_PORTENTA_VISION_SHIELD_ETHERNET
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_NICLA_VISION)
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_OPTA)
  #define BOARD_HAS_WIFI
  #define BOARD_HAS_ETHERNET
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#if defined(ARDUINO_GIGA)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#ifdef ARDUINO_SAMD_MKRGSM1400
  #define BOARD_HAS_GSM
  #define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
  #define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
#endif

#ifdef ARDUINO_SAMD_MKRNB1500
  #define BOARD_HAS_NB
  #define NETWORK_HARDWARE_ERROR
  #define NETWORK_IDLE_STATUS NB_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED NB_NetworkStatus_t::GPRS_READY
#endif

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  #define BOARD_HAS_LORA
#endif

#if defined(ARDUINO_ARCH_ESP8266)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_ARCH_ESP32)
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_UNOR4_WIFI)

  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_LATEST_VERSION
#endif

#ifdef ARDUINO_EDGE_CONTROL
  #define BOARD_HAS_CATM1_NBIOT
  #define BOARD_HAS_PORTENTA_CATM1_NBIOT_SHIELD
  #define NETWORK_HARDWARE_ERROR
#endif

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
#endif

#endif // BOARD_HAS_NOTECARD

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
  CELL,
  NOTECARD
};

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static unsigned int const CHECK_INTERVAL_TABLE[] =
{
#if defined(BOARD_HAS_NOTECARD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  /* INIT    */ 4000,
#else
  /* INIT          */ 500,
#endif
  /* CONNECTING    */ 500,
  /* CONNECTED     */ 10000,
  /* DISCONNECTING */ 100,
  /* DISCONNECTED  */ 1000,
  /* CLOSED        */ 1000,
  /* ERROR         */ 1000
};
