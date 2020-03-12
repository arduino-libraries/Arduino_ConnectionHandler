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

#include "Arduino_WiFiConnectionHandler.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

WiFiConnectionHandler::WiFiConnectionHandler(char const * ssid, char const * pass, bool const keep_alive)
: ConnectionHandler{keep_alive}
, _ssid{ssid}
, _pass{pass}
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

unsigned long WiFiConnectionHandler::getTime()
{
#if !defined(BOARD_ESP8266)
  return WiFi.getTime();
#else
  return 0;
#endif
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState WiFiConnectionHandler::update_handleInit()
{
#ifndef BOARD_ESP8266
  Debug.print(DBG_INFO, "WiFi.status(): %d", WiFi.status());
  if (WiFi.status() == NETWORK_HARDWARE_ERROR)
  {
    execCallback(NetworkConnectionEvent::ERROR);
    Debug.print(DBG_ERROR, "WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield.");
    Debug.print(DBG_ERROR, "Then reset and retry.");
    return NetworkConnectionState::ERROR;
  }

  Debug.print(DBG_ERROR, "Current WiFi Firmware: %s", WiFi.firmwareVersion());

  if (WiFi.firmwareVersion() < WIFI_FIRMWARE_VERSION_REQUIRED)
  {
    Debug.print(DBG_ERROR, "Latest WiFi Firmware: %s", WIFI_FIRMWARE_VERSION_REQUIRED);
    Debug.print(DBG_ERROR, "Please update to the latest version for best performance.");
    delay(5000);
  }
#else
  Debug.print(DBG_ERROR, "WiFi status ESP: %d", WiFi.status());
  WiFi.disconnect();
  delay(300);
  WiFi.begin(_ssid, _pass);
  delay(1000);
#endif /* ifndef BOARD_ESP8266 */

  return NetworkConnectionState::CONNECTING;
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnecting()
{
#ifndef BOARD_ESP8266
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(_ssid, _pass);
  }
#endif /* ifndef BOARD_ESP8266 */

  if (WiFi.status() != NETWORK_CONNECTED)
  {
    Debug.print(DBG_ERROR, "Connection to \"%s\" failed", _ssid);
    Debug.print(DBG_INFO, "Retrying in  \"%d\" milliseconds", CHECK_INTERVAL_TABLE[static_cast<unsigned int>(NetworkConnectionState::CONNECTING)]);
    return NetworkConnectionState::CONNECTING;
  }
  else
  {
    Debug.print(DBG_INFO, "Connected to \"%s\"", _ssid);
    execCallback(NetworkConnectionEvent::CONNECTED);
#ifdef BOARD_ESP8266
  configTime(0, 0, "time.arduino.cc", "pool.ntp.org", "time.nist.gov");
#endif
    return NetworkConnectionState::CONNECTED;
  }
}

NetworkConnectionState WiFiConnectionHandler::update_handleConnected()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    execCallback(NetworkConnectionEvent::DISCONNECTED);
   
    Debug.print(DBG_VERBOSE, "WiFi.status(): %d", WiFi.status());
    Debug.print(DBG_ERROR, "Connection to \"%s\" lost.", _ssid);
  
    if (_keep_alive)
    {
      Debug.print(DBG_ERROR, "Attempting reconnection");
    }
  
    return NetworkConnectionState::DISCONNECTED;
  }
  return NetworkConnectionState::CONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleDisconnecting()
{
  WiFi.disconnect();
  return NetworkConnectionState::DISCONNECTED;
}

NetworkConnectionState WiFiConnectionHandler::update_handleDisconnected()
{
#ifndef BOARD_ESP8266
  WiFi.end();
#endif /* ifndef BOARD_ESP8266 */
  if (_keep_alive)
  {
    return NetworkConnectionState::INIT;
  }
  else
  {
    return NetworkConnectionState::CLOSED;
  }
}

#endif /* #ifdef BOARD_HAS_WIFI */
