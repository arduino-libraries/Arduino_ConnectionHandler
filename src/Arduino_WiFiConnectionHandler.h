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

#include "Arduino_TcpIpConnectionHandler.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class WiFiConnectionHandler : public TcpIpConnectionHandler {
  public:
    WiFiConnectionHandler(const char *_ssid, const char *_pass, bool _keepAlive = true);

    virtual void init();
    virtual unsigned long getTime();
    virtual void check();
    virtual Client &getClient() {
      return wifiClient;
    };
    virtual UDP &getUDP() {
      return udp;
    };
    virtual void disconnect();
    virtual void connect();

    WiFiUDP udp;

  private:

    const int CHECK_INTERVAL_IDLE = 100;
    const int CHECK_INTERVAL_INIT = 100;
    const int CHECK_INTERVAL_CONNECTING = 500;
    const int CHECK_INTERVAL_CONNECTED = 10000;
    const int CHECK_INTERVAL_DISCONNECTED = 1000;

    const char *ssid, *pass;
    unsigned long lastConnectionTickTime;

    WiFiClient wifiClient;
    int connectionTickTimeInterval;

    bool keepAlive;

    NetworkConnectionState update_handleInit         ();
    NetworkConnectionState update_handleConnecting   ();
    NetworkConnectionState update_handleConnected    ();
    NetworkConnectionState update_handleGetTime      ();
    NetworkConnectionState update_handleDisconnecting();
    NetworkConnectionState update_handleDisconnected ();

};

#endif /* #ifdef BOARD_HAS_WIFI */

#endif /* ARDUINO_WIFI_CONNECTION_HANDLER_H_ */
