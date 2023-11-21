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

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Arduino_CatM1ConnectionHandler.h"

#ifdef BOARD_HAS_CATM1_NBIOT /* Only compile if the board has CatM1 BN-IoT */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

CatM1ConnectionHandler::CatM1ConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, RadioAccessTechnologyType rat, uint32_t band, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::GSM}
, _pin(pin)
, _apn(apn)
, _login(login)
, _pass(pass)
, _rat(rat)
, _band(band)
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long CatM1ConnectionHandler::getTime()
{
  return GSM.getTime();
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState CatM1ConnectionHandler::update_handleInit()
{
#if defined (ARDUINO_EDGE_CONTROL)
  pinMode(ON_MKR2, OUTPUT);
  digitalWrite(ON_MKR2, HIGH);
#endif
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleConnecting()
{
  if(!GSM.begin(_pin, _apn, _login, _pass, _rat, _band))
  {
    Debug.print(DBG_ERROR, F("The board was not able to register to the network..."));
    return NetworkConnectionState::ERROR;
  }
  Debug.print(DBG_INFO, F("Connected to Network"));
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleConnected()
{
  int const is_gsm_access_alive = GSM.isConnected();
  if (is_gsm_access_alive != 1)
  {
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleDisconnecting()
{
  GSM.disconnect();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleDisconnected()
{
  if (_keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
   return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_CATM1_NBIOT  */
