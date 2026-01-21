/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_LORA_CONNECTION_HANDLER_H_
#define ARDUINO_LORA_CONNECTION_HANDLER_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  #include <MKRWAN.h>
#endif

#ifndef BOARD_HAS_LORA
  #error "Board doesn't support LORA"
#endif


/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class LoRaConnectionHandler : public ConnectionHandler
{
  public:

    LoRaConnectionHandler(char const * appeui, char const * appkey, _lora_band const band = _lora_band::EU868, char const * channelMask = NULL, _lora_class const device_class = _lora_class::CLASS_A);

    int write(const uint8_t *buf, size_t size) override;
    int read() override;
    bool available() override;

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

    NetworkConnectionState update_handleInit         () override;
    NetworkConnectionState update_handleConnecting   () override;
    NetworkConnectionState update_handleConnected    () override;
    NetworkConnectionState update_handleDisconnecting() override;
    NetworkConnectionState update_handleDisconnected () override;


  private:

    LoRaModem _modem;
};

#endif /* ARDUINO_LORA_CONNECTION_HANDLER_H_ */
