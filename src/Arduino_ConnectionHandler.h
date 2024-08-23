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

/******************************************************************************
   INCLUDES
 ******************************************************************************/

#if !defined(__AVR__)
#  include <Arduino_DebugUtils.h>
#endif

#include <Arduino.h>
#include "ConnectionHandlerDefinitions.h"

#if defined(BOARD_HAS_NOTECARD)
  #include "NotecardConnectionHandler.h"
#else

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

#endif // BOARD_HAS_NOTECARD

#endif /* CONNECTION_HANDLER_H_ */
