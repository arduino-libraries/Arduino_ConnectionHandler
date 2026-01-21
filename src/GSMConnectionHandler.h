/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef GSM_CONNECTION_MANAGER_H_
#define GSM_CONNECTION_MANAGER_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#if defined(ARDUINO_SAMD_MKRGSM1400)
  #include <MKRGSM.h>
#endif

#ifndef BOARD_HAS_GSM
  #error "Board doesn't support GSM"
#endif

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class GSMConnectionHandler : public ConnectionHandler
{
  public:
    GSMConnectionHandler();
    GSMConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive = true);


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

    GSM _gsm;
    GPRS _gprs;
    GSMUDP _gsm_udp;
    GSMClient _gsm_client;
};

#endif /* #ifndef GSM_CONNECTION_MANAGER_H_ */
