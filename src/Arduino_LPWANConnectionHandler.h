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

#ifndef ARDUINO_LPWAN_CONNECTION_HANDLER_H_
#define ARDUINO_LPWAN_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDES
 ******************************************************************************/

#include <Arduino_DebugUtils.h>
#include <Arduino_ConnectionHandler.h>

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class LPWANConnectionHandler : public ConnectionHandler {
  public:
    virtual void init() = 0;
    virtual void check() = 0;
    virtual void update() = 0;
    virtual unsigned long getTime() = 0;

    virtual int write(const uint8_t *buf, size_t size) = 0;
    virtual int read() = 0;
    virtual bool available() = 0;

    virtual void connect() = 0;
    virtual void disconnect() = 0;

};

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  //#error BOARD_HAS_LORA
  #include "Arduino_LoRaConnectionHandler.h"
#endif


#endif /* LPWAN_CONNECTION_HANDLER_H_ */
