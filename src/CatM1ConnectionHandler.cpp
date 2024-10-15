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

#include "ConnectionHandlerDefinitions.h"

#ifdef BOARD_HAS_CATM1_NBIOT /* Only compile if the board has CatM1 BN-IoT */
#include "CatM1ConnectionHandler.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

CatM1ConnectionHandler::CatM1ConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::CATM1) { }

CatM1ConnectionHandler::CatM1ConnectionHandler(
  const char * pin, const char * apn, const char * login, const char * pass,
  RadioAccessTechnologyType rat, uint32_t band, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::CATM1}
{
  _settings.type = NetworkAdapter::CATM1;
  strncpy(_settings.catm1.pin, pin, sizeof(_settings.catm1.pin)-1);
  strncpy(_settings.catm1.apn, apn, sizeof(_settings.catm1.apn)-1);
  strncpy(_settings.catm1.login, login, sizeof(_settings.catm1.login)-1);
  strncpy(_settings.catm1.pass, pass, sizeof(_settings.catm1.pass)-1);
  _settings.catm1.rat  = static_cast<uint8_t>(rat);
  _settings.catm1.band = band;
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
  /* Power on module */
  pinMode(ON_MKR2, OUTPUT);
  digitalWrite(ON_MKR2, HIGH);
#endif

  if(!GSM.begin(
    _settings.catm1.pin,
    _settings.catm1.apn,
    _settings.catm1.login,
    _settings.catm1.pass,
    static_cast<RadioAccessTechnologyType>(_settings.catm1.rat) ,
    _settings.catm1.band))
  {
    Debug.print(DBG_ERROR, F("The board was not able to register to the network..."));
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState CatM1ConnectionHandler::update_handleConnecting()
{
  if (!GSM.isConnected())
  {
    return NetworkConnectionState::INIT;
  }

  if(!_check_internet_availability){
    return NetworkConnectionState::CONNECTED;
  }

  int ping_result = GSM.ping("time.arduino.cc");
  Debug.print(DBG_INFO, F("GSM.ping(): %d"), ping_result);
  if (ping_result < 0)
  {
    Debug.print(DBG_ERROR, F("Internet check failed"));
    Debug.print(DBG_INFO, F("Retrying in  \"%d\" milliseconds"), CHECK_INTERVAL_TABLE[static_cast<unsigned int>(NetworkConnectionState::CONNECTING)]);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_INFO, F("Connected to Internet"));
    return NetworkConnectionState::CONNECTED;
  }
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
  GSM.end();
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
