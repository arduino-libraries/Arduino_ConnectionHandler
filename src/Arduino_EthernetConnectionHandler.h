/*
   This file is part of ArduinoIoTCloud.
   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)
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

#ifndef ARDUINO_ETHERNET_CONNECTION_HANDLER_H_
#define ARDUINO_ETHERNET_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Arduino_ConnectionHandler.h"

#ifdef BOARD_HAS_ETHERNET /* Only compile if the board has ethernet */

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class EthernetConnectionHandler : public ConnectionHandler
{
  public:

    EthernetConnectionHandler(uint8_t * mac, bool const keep_alive = true);


    virtual unsigned long getTime() override { return 0; }
    virtual Client & getClient() override{ return _eth_client; }
    virtual UDP & getUDP() override { return _eth_udp; }


  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;

  private:

    uint8_t const * _mac;

    EthernetUDP _eth_udp;
    EthernetClient _eth_client;

};

#endif /* #ifdef BOARD_HAS_ETHERNET */

#endif /* ARDUINO_ETHERNET_CONNECTION_HANDLER_H_ */
