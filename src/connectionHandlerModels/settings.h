/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ConnectionHandlerDefinitions.h"
#include <stdint.h>
#include <IPAddress.h>

namespace models {
  constexpr size_t WifiSsidLength = 33;         // Max length of wifi ssid is 32 + \0
  constexpr size_t WifiPwdLength = 64;          // Max length of wifi password is 63 + \0

  constexpr size_t CellularPinLength = 9;
  constexpr size_t CellularApnLength = 101;     // Max length of apn is 100 + \0
  constexpr size_t CellularLoginLength = 65;
  constexpr size_t CellularPassLength = 65;

  constexpr size_t LoraAppeuiLength = 17;       // appeui is 8 octets * 2 (hex format) + \0
  constexpr size_t LoraAppkeyLength = 33;       // appeui is 16 octets * 2 (hex format) + \0
  constexpr size_t LoraChannelMaskLength = 13;

  #if defined(BOARD_HAS_WIFI)
  struct WiFiSetting {
    char ssid[WifiSsidLength];
    char pwd[WifiPwdLength];
  };
  #endif //defined(BOARD_HAS_WIFI)

  #if defined(BOARD_HAS_ETHERNET)
  // this struct represents an ip address in its simplest form.
  // FIXME this should be available from ArduinoCore-api IPAddress
  struct ip_addr {
    IPType type;
    union {
        uint8_t bytes[16];
        uint32_t dword[4];
    };
  };

  struct EthernetSetting {
    ip_addr       ip;
    ip_addr       dns;
    ip_addr       gateway;
    ip_addr       netmask;
    unsigned long timeout;
    unsigned long response_timeout;
  };
  #endif // BOARD_HAS_ETHERNET

  #if defined(BOARD_HAS_NB) || defined(BOARD_HAS_GSM) ||defined(BOARD_HAS_CELLULAR)
  struct CellularSetting {
    char pin[CellularPinLength];
    char apn[CellularApnLength];
    char login[CellularLoginLength];
    char pass[CellularPassLength];
  };
  #endif // defined(BOARD_HAS_NB) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_CATM1_NBIOT) || defined(BOARD_HAS_CELLULAR)

  #if defined(BOARD_HAS_GSM)
  typedef CellularSetting GSMSetting;
  #endif //defined(BOARD_HAS_GSM)

  #if defined(BOARD_HAS_NB)
  typedef CellularSetting NBSetting;
  #endif //defined(BOARD_HAS_NB)

  #if defined(BOARD_HAS_CATM1_NBIOT)
  struct CATM1Setting {
    char      pin[CellularPinLength];
    char      apn[CellularApnLength];
    char      login[CellularLoginLength];
    char      pass[CellularPassLength];
    uint32_t  band;
    uint8_t   rat;
  };
  #endif //defined(BOARD_HAS_CATM1_NBIOT)

#if defined(BOARD_HAS_LORA)
  struct LoraSetting {
    char          appeui[LoraAppeuiLength];
    char          appkey[LoraAppkeyLength];
    uint8_t       band;
    char          channelMask[LoraChannelMaskLength];
    uint8_t       deviceClass;
  };
#endif

  struct NetworkSetting {
    NetworkAdapter type;
    union {
      #if defined(BOARD_HAS_WIFI)
      WiFiSetting     wifi;
      #endif

      #if defined(BOARD_HAS_ETHERNET)
      EthernetSetting eth;
      #endif

      #if defined(BOARD_HAS_NB)
      NBSetting       nb;
      #endif

      #if defined(BOARD_HAS_GSM)
      GSMSetting      gsm;
      #endif

      #if defined(BOARD_HAS_CATM1_NBIOT)
      CATM1Setting    catm1;
      #endif

      #if defined(BOARD_HAS_CELLULAR)
      CellularSetting cell;
      #endif

      #if defined(BOARD_HAS_LORA)
      LoraSetting     lora;
      #endif
    };
  };
}

#include "settings_default.h"
