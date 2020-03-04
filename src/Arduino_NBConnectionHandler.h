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

#include "Arduino_ConnectionHandler.h"

#ifdef BOARD_HAS_NB /* Only compile if this is a board with NB */

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class NBConnectionHandler : public ConnectionHandler {
  public:
    NBConnectionHandler(const char *pin, const bool keepAlive = true);
    NBConnectionHandler(const char *pin, const char *apn, const bool keepAlive = true);
    NBConnectionHandler(const char *pin, const char *apn, const char *login, const char *pass, const bool keepAlive = true);

    virtual void init();
    virtual unsigned long getTime();
    virtual NetworkConnectionState check();
    virtual Client &getClient() {
      return networkClient;
    };
    virtual UDP &getUDP() {
      return udp;
    };

    NBClient networkClient;
    NB  nbAccess;
    GPRS gprs;
    NBUDP udp;

    virtual void disconnect();
    virtual void connect();

  private:

    void changeConnectionState(NetworkConnectionState _newState);

    const int CHECK_INTERVAL_IDLE = 100;
    const int CHECK_INTERVAL_INIT = 100;
    const int CHECK_INTERVAL_CONNECTING = 500;
    const int CHECK_INTERVAL_CONNECTED = 10000;
    const int CHECK_INTERVAL_RETRYING = 30000;
    const int CHECK_INTERVAL_DISCONNECTED = 1000;
    const int CHECK_INTERVAL_ERROR = 500;

    const char *pin, *apn, *login, *pass;
    unsigned long lastConnectionTickTime;
    int connectionTickTimeInterval;


    bool keepAlive;

};

typedef NBConnectionHandler TcpIpConnectionHandler;

#endif /* #ifdef BOARD_HAS_NB  */

#endif /* #ifndef NB_CONNECTION_MANAGER_H_ */
