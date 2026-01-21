/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef NB_CONNECTION_MANAGER_H_
#define NB_CONNECTION_MANAGER_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#ifdef ARDUINO_SAMD_MKRNB1500
  #include <MKRNB.h>
#endif

#ifndef BOARD_HAS_NB
  #error "Board doesn't support NB"
#endif

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class NBConnectionHandler : public ConnectionHandler
{
  public:
    NBConnectionHandler();
    NBConnectionHandler(char const * pin, bool const keep_alive = true);
    NBConnectionHandler(char const * pin, char const * apn, bool const keep_alive = true);
    NBConnectionHandler(char const * pin, char const * apn, char const * login, char const * pass, bool const keep_alive = true);


    unsigned long getTime() override;
    Client & getClient() override { return _nb_client; };
    UDP & getUDP() override { return _nb_udp; };


  protected:

    NetworkConnectionState update_handleInit         () override;
    NetworkConnectionState update_handleConnecting   () override;
    NetworkConnectionState update_handleConnected    () override;
    NetworkConnectionState update_handleDisconnecting() override;
    NetworkConnectionState update_handleDisconnected () override;


  private:

    void changeConnectionState(NetworkConnectionState _newState);

    NB _nb;
    GPRS _nb_gprs;
    NBUDP _nb_udp;
    NBClient _nb_client;
};

#endif /* #ifndef NB_CONNECTION_MANAGER_H_ */
