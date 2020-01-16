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

#ifndef ARDUINO_TCPIP_CONNECTION_HANDLER_H_
#define ARDUINO_TCPIP_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDES
 ******************************************************************************/

#include <Client.h>
#include <Udp.h>

#include <Arduino_DebugUtils.h>
#include <Arduino_ConnectionHandler.h>

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class TcpIpConnectionHandler : public ConnectionHandler {
  public:
    virtual void init() = 0;
    virtual void check() = 0;
    virtual void update() = 0;
    virtual unsigned long getTime() = 0;
    virtual Client &getClient() = 0;
    virtual UDP &getUDP() = 0;

    virtual void connect() = 0;
    virtual void disconnect() = 0;

};

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SAMD_NANO_33_IOT)
  #include "Arduino_WiFiConnectionHandler.h"
#elif defined(ARDUINO_SAMD_MKRGSM1400)
  #include "Arduino_GSMConnectionHandler.h"
#elif defined(BOARD_HAS_NB)
  #include "Arduino_NBConnectionHandler.h"
#endif

#endif /* TCPIP_CONNECTION_HANDLER_H_ */
