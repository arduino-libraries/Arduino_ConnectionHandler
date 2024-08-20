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

#ifndef GSM_CONNECTION_MANAGER_H_
#define GSM_CONNECTION_MANAGER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#if defined(ARDUINO_SAMD_MKRGSM1400)
  #include <MKRGSM.h>
#endif

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class GSMConnectionHandler : public ConnectionHandler
{
  public:

    GSMConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive = true);


    virtual unsigned long getTime() override;
    virtual Client & getClient() override { return _gsm_client; };
    virtual UDP & getUDP() override { return _gsm_udp; };


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

    GSM _gsm;
    GPRS _gprs;
    GSMUDP _gsm_udp;
    GSMClient _gsm_client;
};

#endif /* #ifndef GSM_CONNECTION_MANAGER_H_ */
