/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#ifndef ARDUINO_CELLULAR_CONNECTION_HANDLER_H_
#define ARDUINO_CELLULAR_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#if defined(ARDUINO_PORTENTA_C33) || defined(ARDUINO_PORTENTA_H7_M7)
#include <Arduino_Cellular.h>
#endif

#ifndef BOARD_HAS_CELLULAR
  #error "Board doesn't support CELLULAR"
#endif

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CellularConnectionHandler : public ConnectionHandler
{
  public:

    CellularConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive = true);


    virtual unsigned long getTime() override;
    virtual Client & getClient() override { return _gsm_client; };
    virtual UDP & getUDP() override;


  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;


  private:

    const char * _pin;
    const char * _apn;
    const char * _login;
    const char * _pass;

    ArduinoCellular _cellular;
    TinyGsmClient _gsm_client = _cellular.getNetworkClient();
};

#endif /* #ifndef ARDUINO_CELLULAR_CONNECTION_HANDLER_H_ */
