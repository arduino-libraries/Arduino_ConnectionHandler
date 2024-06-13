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

#ifndef ARDUINO_WIFI_CONNECTION_HANDLER_DYNAMIC_H_
#define ARDUINO_WIFI_CONNECTION_HANDLER_DYNAMIC_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Arduino_ConnectionHandler.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class WiFiConnectionHandlerDynamic : public ConnectionHandler
{
  public:

    WiFiConnectionHandlerDynamic(bool const keep_alive = true);

    void setWiFiCredentials(String ssid, String pass);
    virtual unsigned long getTime() override;
    virtual Client & getClient() override{ return _wifi_client; }
    virtual UDP & getUDP() override { return _wifi_udp; }


  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;

  private:

    String _ssid;
    String _pass;

    WiFiUDP _wifi_udp;
    WiFiClient _wifi_client;
};

#endif /* #ifdef BOARD_HAS_WIFI */

#endif /* ARDUINO_WIFI_CONNECTION_HANDLER_DYNAMIC_H_ */
