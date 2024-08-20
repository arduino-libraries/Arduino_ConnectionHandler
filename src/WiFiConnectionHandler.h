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

#ifndef ARDUINO_WIFI_CONNECTION_HANDLER_H_
#define ARDUINO_WIFI_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#ifdef ARDUINO_SAMD_MKR1000
  #include <WiFi101.h>
  #include <WiFiUdp.h>
#elif defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || \
  defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined (ARDUINO_NANO_RP2040_CONNECT)
  #include <WiFiNINA.h>
  #include <WiFiUdp.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M7) || \
  defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)
  #include <WiFi.h>
  #include <WiFiUdp.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include <WiFiC3.h>
  #include <WiFiUdp.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
#elif defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <WiFiUdp.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiS3.h>
#endif

#ifndef BOARD_HAS_WIFI
  #error "Board doesn't support WIFI"
#endif

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class WiFiConnectionHandler : public ConnectionHandler
{
  public:

    WiFiConnectionHandler(char const * ssid, char const * pass, bool const keep_alive = true);


    virtual unsigned long getTime() override;
    virtual Client & getClient() override { return _wifi_client; }
    virtual UDP & getUDP() override { return _wifi_udp; }


  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;

  private:

    char const * _ssid;
    char const * _pass;

    WiFiUDP _wifi_udp;
    WiFiClient _wifi_client;
};

#endif /* ARDUINO_WIFI_CONNECTION_HANDLER_H_ */
