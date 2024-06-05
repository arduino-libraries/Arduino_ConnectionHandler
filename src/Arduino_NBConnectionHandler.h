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

#ifndef NB_CONNECTION_MANAGER_H_
#define NB_CONNECTION_MANAGER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Arduino_ConnectionHandlerInterface.h"

#ifdef ARDUINO_SAMD_MKRNB1500
  #include <MKRNB.h>
#endif

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class NBConnectionHandler : public ConnectionHandler
{
  public:

    NBConnectionHandler(char const * pin, bool const keep_alive = true);
    NBConnectionHandler(char const * pin, char const * apn, bool const keep_alive = true);
    NBConnectionHandler(char const * pin, char const * apn, char const * login, char const * pass, bool const keep_alive = true);


    virtual unsigned long getTime() override;
    virtual Client & getClient() override { return _nb_client; };
    virtual UDP & getUDP() override { return _nb_udp; };


  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;


  private:

    void changeConnectionState(NetworkConnectionState _newState);

    char const * _pin;
    char const * _apn;
    char const * _login;
    char const * _pass;

    NB _nb;
    GPRS _nb_gprs;
    NBUDP _nb_udp;
    NBClient _nb_client;
};

#endif /* #ifndef NB_CONNECTION_MANAGER_H_ */
