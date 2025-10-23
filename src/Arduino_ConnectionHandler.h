/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CONNECTION_HANDLER_H_
#define ARDUINO_CONNECTION_HANDLER_H_

/******************************************************************************
  INCLUDES
 ******************************************************************************/

#if !defined(__AVR__)
#  include <Arduino_DebugUtils.h>
#endif

#include <Arduino.h>
#include "ConnectionHandlerDefinitions.h"

#if defined(BOARD_HAS_WIFI)
  #include "WiFiConnectionHandler.h"
#endif

#if defined(BOARD_HAS_GSM)
  #include "GSMConnectionHandler.h"
#endif

#if defined(BOARD_HAS_NB)
  #include "NBConnectionHandler.h"
#endif

#if defined(BOARD_HAS_LORA)
  #include "LoRaConnectionHandler.h"
#endif

#if defined(BOARD_HAS_ETHERNET)
  #include "EthernetConnectionHandler.h"
#endif

#if defined(BOARD_HAS_CATM1_NBIOT)
  #include "CatM1ConnectionHandler.h"
#endif

#if defined(BOARD_HAS_CELLULAR)
  #include "CellularConnectionHandler.h"
#endif

#endif /* CONNECTION_HANDLER_H_ */
