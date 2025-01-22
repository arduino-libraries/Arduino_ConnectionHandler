/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include "settings.h"

namespace models {

  /*
   * if the cpp version is older than cpp14 then a constexpr function cannot include
   * other than a simple return statement, thsu we can define it only as inline
   */
  #if __cplusplus > 201103L
  constexpr NetworkSetting settingsDefault(NetworkAdapter type) {
  #else
  inline NetworkSetting settingsDefault(NetworkAdapter type) {
  #endif

    NetworkSetting res = {type};

    switch(type) {
    #if defined(BOARD_HAS_WIFI)
    case NetworkAdapter::WIFI: // nothing todo, default optional values are fine with 0
        break;
    #endif  //defined(BOARD_HAS_WIFI)

    #if defined(BOARD_HAS_NB)
    case NetworkAdapter::NB:  // nothing todo, default optional values are fine with 0
      break;
    #endif  //defined(BOARD_HAS_NB)

    #if defined(BOARD_HAS_GSM)
    case NetworkAdapter::GSM: // nothing todo, default optional values are fine with 0
      break;
    #endif  //defined(BOARD_HAS_GSM)

    #if defined(BOARD_HAS_CELLULAR)
    case NetworkAdapter::CELL: // nothing todo, default optional values are fine with 0
      break;
    #endif  //defined(BOARD_HAS_CELLULAR)

    #if defined(BOARD_HAS_ETHERNET)
    case NetworkAdapter::ETHERNET:
        res.eth.timeout = 15000;
        res.eth.response_timeout = 4000;
        break;
    #endif  //defined(BOARD_HAS_ETHERNET)

    #if defined(BOARD_HAS_CATM1_NBIOT)
    case NetworkAdapter::CATM1:
      res.catm1.rat = 7; // CATM1
      res.catm1.band = 0x04 | 0x80000 | 0x40000; // BAND_3 | BAND_20 | BAND_19
      break;
    #endif  //defined(BOARD_HAS_CATM1_NBIOT)

    #if defined(BOARD_HAS_LORA)
    case NetworkAdapter::LORA:
      res.lora.band = 5; // _lora_band::EU868
      res.lora.channelMask[0] = '\0';
      res.lora.deviceClass = 'A'; // _lora_class::CLASS_A
      break;
    #endif  //defined(BOARD_HAS_LORA)
    }

    return res;
  }
}
