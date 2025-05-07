/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_GENERIC_CONNECTION_HANDLER_H_
#define ARDUINO_GENERIC_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

/** GenericConnectionHandler class
 * This class aims to wrap a connectionHandler and provide a generic way to
 * instantiate a specific connectionHandler type
 */
class GenericConnectionHandler : public ConnectionHandler
{
  public:

    GenericConnectionHandler(bool const keep_alive=true): ConnectionHandler(keep_alive), _ch(nullptr) {}

    #if defined(BOARD_HAS_NOTECARD) || defined(BOARD_HAS_LORA)
      virtual bool available() = 0;
      virtual int read() = 0;
      virtual int write(const uint8_t *buf, size_t size) = 0;
    #else
      unsigned long getTime() override;

      /*
       * NOTE: The following functions have a huge risk of returning a reference to a non existing memory location
       * It is important to make sure that the internal connection handler is already allocated before calling them
       * When updateSettings is called and the internal connectionHandler is reallocated the references to TCP and UDP
       * handles should be deleted.
       */
      Client & getClient() override;
      UDP & getUDP() override;
    #endif

    bool updateSetting(const models::NetworkSetting& s) override;

    void connect() override;
    void disconnect() override;

    void setKeepAlive(bool keep_alive=true) override;

  protected:

    NetworkConnectionState updateConnectionState() override;

    NetworkConnectionState update_handleInit         () override;
    NetworkConnectionState update_handleConnecting   () override;
    NetworkConnectionState update_handleConnected    () override;
    NetworkConnectionState update_handleDisconnecting() override;
    NetworkConnectionState update_handleDisconnected () override;

  private:

    ConnectionHandler* _ch;
};

#endif /* ARDUINO_GENERIC_CONNECTION_HANDLER_H_ */
