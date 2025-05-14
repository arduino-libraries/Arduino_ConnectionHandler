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

#ifdef BOARD_HAS_GSM /* Only compile if this is a board with GSM */
#include "GSMConnectionHandler.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const GSM_TIMEOUT = 30000;
static int const GPRS_TIMEOUT = 30000;

/******************************************************************************
   FUNCTION DEFINITION
 ******************************************************************************/

__attribute__((weak)) void mkr_gsm_feed_watchdog()
{
    /* This function can be overwritten by a "strong" implementation
     * in a higher level application, such as the ArduinoIoTCloud
     * firmware stack.
     */
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
GSMConnectionHandler::GSMConnectionHandler()
: ConnectionHandler(true, NetworkAdapter::GSM) {}

GSMConnectionHandler::GSMConnectionHandler(const char * pin, const char * apn, const char * login, const char * pass, bool const keep_alive)
: ConnectionHandler{keep_alive, NetworkAdapter::GSM}
{
  _settings.type = NetworkAdapter::GSM;
  // To keep the backward compatibility, the user can call enableCheckInternetAvailability(false) for disabling the check
  _check_internet_availability = true;
  strncpy(_settings.gsm.pin, pin, sizeof(_settings.gsm.pin)-1);
  strncpy(_settings.gsm.apn, apn, sizeof(_settings.gsm.apn)-1);
  strncpy(_settings.gsm.login, login, sizeof(_settings.gsm.login)-1);
  strncpy(_settings.gsm.pass, pass, sizeof(_settings.gsm.pass)-1);
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long GSMConnectionHandler::getTime()
{
  return _gsm.getTime();
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState GSMConnectionHandler::update_handleInit()
{
  mkr_gsm_feed_watchdog();

  if (_gsm.begin(_settings.gsm.pin) != GSM_READY)
  {
    Debug.print(DBG_ERROR, F("SIM not present or wrong PIN"));
    return NetworkConnectionState::ERROR;
  }

  mkr_gsm_feed_watchdog();

  Debug.print(DBG_INFO, F("SIM card ok"));
  _gsm.setTimeout(GSM_TIMEOUT);
  _gprs.setTimeout(GPRS_TIMEOUT);

  mkr_gsm_feed_watchdog();

  GSM3_NetworkStatus_t const network_status = _gprs.attachGPRS(
    _settings.gsm.apn, _settings.gsm.login, _settings.gsm.pass, true);
  Debug.print(DBG_DEBUG, F("GPRS.attachGPRS(): %d"), network_status);
  if (network_status == GSM3_NetworkStatus_t::ERROR)
  {
    Debug.print(DBG_ERROR, F("GPRS attach failed"));
    Debug.print(DBG_ERROR, F("Make sure the antenna is connected and reset your board."));
    return NetworkConnectionState::ERROR;
  }

  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState GSMConnectionHandler::update_handleConnecting()
{
  if(!_check_internet_availability){
    return NetworkConnectionState::CONNECTED;
  }

  Debug.print(DBG_INFO, F("Sending PING to outer space..."));
  int const ping_result = _gprs.ping("time.arduino.cc");
  Debug.print(DBG_INFO, F("GPRS.ping(): %d"), ping_result);
  if (ping_result < 0)
  {
    Debug.print(DBG_ERROR, F("PING failed"));
    Debug.print(DBG_INFO, F("Retrying in  \"%d\" milliseconds"), _timeoutTable.timeout.connecting);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_INFO, F("Connected to GPRS Network"));
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState GSMConnectionHandler::update_handleConnected()
{
  int const is_gsm_access_alive = _gsm.isAccessAlive();
  if (is_gsm_access_alive != 1)
  {
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnecting()
{
  _gsm.shutdown();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState GSMConnectionHandler::update_handleDisconnected()
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

#endif /* #ifdef BOARD_HAS_GSM  */
