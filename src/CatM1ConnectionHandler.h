/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2023 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CATM1_CONNECTION_HANDLER_H_
#define ARDUINO_CATM1_CONNECTION_HANDLER_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_EDGE_CONTROL)
  #include <GSM.h>
#endif

#ifndef BOARD_HAS_CATM1_NBIOT
  #error "Board doesn't support CATM1_NBIOT"
#endif

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class CatM1ConnectionHandler : public ConnectionHandler
{
  public:

    CatM1ConnectionHandler();
    CatM1ConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, RadioAccessTechnologyType rat = CATM1, uint32_t band = BAND_3 | BAND_20 | BAND_19, bool const keep_alive = true);


    unsigned long getTime() override;
    Client & getClient() override { return _gsm_client; };
    UDP & getUDP() override { return _gsm_udp; };


  protected:

    NetworkConnectionState update_handleInit         () override;
    NetworkConnectionState update_handleConnecting   () override;
    NetworkConnectionState update_handleConnected    () override;
    NetworkConnectionState update_handleDisconnecting() override;
    NetworkConnectionState update_handleDisconnected () override;


  private:

    bool _reset;

    GSMUDP _gsm_udp;
    GSMClient _gsm_client;
};

#endif /* #ifndef ARDUINO_CATM1_CONNECTION_HANDLER_H_ */
