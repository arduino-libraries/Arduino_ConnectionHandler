/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
#elif defined(ARDUINO_TEENSY41)
  #include <QNEthernet.h>

  using namespace qindesign::network;
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

    EthernetConnectionHandler(
      unsigned long const timeout = 15000,
      unsigned long const responseTimeout = 4000,
      bool const keep_alive = true);

    EthernetConnectionHandler(
      const IPAddress ip,
      const IPAddress dns,
      const IPAddress gateway,
      const IPAddress netmask,
      unsigned long const timeout = 15000,
      unsigned long const responseTimeout = 4000,
      bool const keep_alive = true);

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

    EthernetUDP _eth_udp;
    EthernetClient _eth_client;

};

#endif /* ARDUINO_ETHERNET_CONNECTION_HANDLER_H_ */
