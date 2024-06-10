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

#include "Arduino_ConnectionHandlerInterface.h"

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  #include <MKRWAN.h>
#endif

#ifdef BOARD_HAS_LORA /* Only compile if the board has LoRa */

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class LoRaConnectionHandler : public ConnectionHandler
{
  public:

    LoRaConnectionHandler(char const * appeui, char const * appkey, _lora_band const band = _lora_band::EU868, char const * channelMask = NULL, _lora_class const device_class = _lora_class::CLASS_A);

    virtual int write(const uint8_t *buf, size_t size) override;
    virtual int read() override;
    virtual bool available() override;

    inline String getVersion() { return _modem.version(); }
    inline String getDeviceEUI() { return _modem.deviceEUI(); }
    inline int getChannelMaskSize(_lora_band band) { return _modem.getChannelMaskSize(band); }
    inline String getChannelMask() { return _modem.getChannelMask(); }
    inline int isChannelEnabled(int pos) { return _modem.isChannelEnabled(pos); }
    inline int getDataRate() { return _modem.getDataRate(); }
    inline int getADR() { return _modem.getADR(); }
    inline String getDevAddr() { return _modem.getDevAddr(); }
    inline String getNwkSKey() { return _modem.getNwkSKey(); }
    inline String getAppSKey() { return _modem.getAppSKey(); }
    inline int getRX2DR() { return _modem.getRX2DR(); }
    inline uint32_t getRX2Freq() { return _modem.getRX2Freq(); }
    inline int32_t getFCU() { return _modem.getFCU(); }
    inline int32_t getFCD() { return _modem.getFCD(); }

  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;


  private:

    char const * _appeui;
    char const * _appkey;
    _lora_band _band;
    char const * _channelMask;
    _lora_class _device_class;
    LoRaModem _modem;
};

#endif /* #ifdef BOARD_HAS_LORA */

#endif /* ARDUINO_LORA_CONNECTION_HANDLER_H_ */
