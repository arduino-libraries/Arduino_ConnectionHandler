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

#include "ConnectionHandlerInterface.h"

#if defined(ARDUINO_PORTENTA_H7_M7)
  #include <Ethernet.h>
  #include <PortentaEthernet.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include <EthernetC33.h>
  #include <EthernetUdp.h>
#elif defined(ARDUINO_OPTA)
  #include <Ethernet.h>
  #include <PortentaEthernet.h>
#endif

#ifndef BOARD_HAS_ETHERNET
  #error "Board doesn't support ETHERNET"
#endif

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class EthernetConnectionHandler : public ConnectionHandler
{
  public:

    EthernetConnectionHandler(unsigned long const timeout = 15000, unsigned long const responseTimeout = 4000, bool const keep_alive = true);
    EthernetConnectionHandler(const IPAddress ip, const IPAddress dns, const IPAddress gateway, const IPAddress netmask, unsigned long const timeout = 15000, unsigned long const responseTimeout = 4000, bool const keep_alive = true);
    EthernetConnectionHandler(const char * ip, const char * dns, const char * gateway, const char * netmask, unsigned long const timeout = 15000, unsigned long const responseTimeout = 4000, bool const keep_alive = true);


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

    IPAddress _ip;
    IPAddress _dns;
    IPAddress _gateway;
    IPAddress _netmask;

    unsigned long _timeout;
    unsigned long _response_timeout;

    EthernetUDP _eth_udp;
    EthernetClient _eth_client;

};

#endif /* ARDUINO_ETHERNET_CONNECTION_HANDLER_H_ */
