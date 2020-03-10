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

#ifndef ARDUINO_LORA_CONNECTION_HANDLER_H_
#define ARDUINO_LORA_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Arduino_ConnectionHandler.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class LoRaConnectionHandler : public ConnectionHandler
{
  public:

    LoRaConnectionHandler(char const * appeui, char const * appkey, _lora_band const band = _lora_band::EU868, _lora_class const device_class = _lora_class::CLASS_A);


    virtual NetworkConnectionState check();
    virtual int write(const uint8_t *buf, size_t size);
    virtual int read();
    virtual bool available();
    virtual void disconnect();
    virtual void connect();


  private:

    char const * _appeui;
    char const * _appkey;
    _lora_band _band;
    _lora_class _device_class;
    unsigned long _lastConnectionTickTime;
    bool _keep_alive;
    LoRaModem _modem;

    NetworkConnectionState update_handleInit();
    NetworkConnectionState update_handleConnecting();
    NetworkConnectionState update_handleConnected();
    NetworkConnectionState update_handleDisconnecting();
    NetworkConnectionState update_handleDisconnected();
};

#endif /* ARDUINO_LORA_CONNECTION_HANDLER_H_ */
