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

#ifdef BOARD_HAS_NB /* Only compile if this is a board with NB */
#include "NBConnectionHandler.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const NB_TIMEOUT = 30000;

/******************************************************************************
   FUNCTION DEFINITION
 ******************************************************************************/

__attribute__((weak)) void mkr_nb_feed_watchdog()
{
    /* This function can be overwritten by a "strong" implementation
     * in a higher level application, such as the ArduinoIoTCloud
     * firmware stack.
     */
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

NBConnectionHandler::NBConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::NB) {}

NBConnectionHandler::NBConnectionHandler(char const * pin, bool const keep_alive)
: NBConnectionHandler(pin, "", keep_alive)
{

}

NBConnectionHandler::NBConnectionHandler(char const * pin, char const * apn, bool const keep_alive)
: NBConnectionHandler(pin, apn, "", "", keep_alive)
{

}

NBConnectionHandler::NBConnectionHandler(char const * pin, char const * apn, char const * login, char const * pass, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::NB}
{
  _settings.type = NetworkAdapter::NB;
  strncpy(_settings.nb.pin, pin, sizeof(_settings.nb.pin)-1);
  strncpy(_settings.nb.apn, apn, sizeof(_settings.nb.apn)-1);
  strncpy(_settings.nb.login, login, sizeof(_settings.nb.login)-1);
  strncpy(_settings.nb.pass, pass, sizeof(_settings.nb.pass)-1);
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long NBConnectionHandler::getTime()
{
  return _nb.getTime();
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState NBConnectionHandler::update_handleInit()
{
  mkr_nb_feed_watchdog();

  if (_nb.begin(_settings.nb.pin,
                _settings.nb.apn,
                _settings.nb.login,
                _settings.nb.pass) == NB_READY)
  {
    Debug.print(DBG_INFO, F("SIM card ok"));
    _nb.setTimeout(NB_TIMEOUT);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_ERROR, F("SIM not present or wrong PIN"));
    return NetworkConnectionState::ERROR;
  }
  NB_NetworkStatus_t const network_status = _nb_gprs.attachGPRS(true);
  Debug.print(DBG_DEBUG, F("GPRS.attachGPRS(): %d"), network_status);
  if (network_status == NB_NetworkStatus_t::NB_ERROR)
  {
    Debug.print(DBG_ERROR, F("GPRS.attachGPRS() failed"));
    return NetworkConnectionState::ERROR;
  }
  else
  {
    Debug.print(DBG_INFO, F("Connected to GPRS Network"));
    return NetworkConnectionState::CONNECTING;
  }

}

NetworkConnectionState NBConnectionHandler::update_handleConnecting()
{
  if(_nb.isAccessAlive() != 1){
    return NetworkConnectionState::INIT;
  }

  if(!_check_internet_availability){
    return NetworkConnectionState::CONNECTED;
  }

  int ping_result = _nb_gprs.ping("time.arduino.cc");
  Debug.print(DBG_INFO, F("GPRS.ping(): %d"), ping_result);
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

NetworkConnectionState NBConnectionHandler::update_handleConnected()
{
  int const nb_is_access_alive = _nb.isAccessAlive();
  Debug.print(DBG_VERBOSE, F("GPRS.isAccessAlive(): %d"), nb_is_access_alive);
  if (nb_is_access_alive != 1)
  {
    Debug.print(DBG_INFO, F("Disconnected from cellular network"));
    return NetworkConnectionState::DISCONNECTED;
  }
  else
  {
    Debug.print(DBG_VERBOSE, F("Connected to Cellular Network"));
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState NBConnectionHandler::update_handleDisconnecting()
{
  Debug.print(DBG_VERBOSE, F("Disconnecting from Cellular Network"));
  _nb.shutdown();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState NBConnectionHandler::update_handleDisconnected()
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

#endif /* #ifdef BOARD_HAS_NB  */
