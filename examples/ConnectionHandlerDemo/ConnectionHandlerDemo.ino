/* SECRET_ fields are in arduino_secrets.h included above
 * if using a WiFi board (Arduino MKR1000, MKR WiFi 1010, Nano 33 IoT, UNO
 * WiFi Rev 2 or ESP8266/32), create a WiFiConnectionHandler object by adding
 * Network Name (SECRET_SSID) and password (SECRET_PASS) in the arduino_secrets.h
 * file (or Secrets tab in Create Web Editor).
 *
 *    WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);
 *
 * If using a MKR GSM 1400 or other GSM boards supporting the same API you'll
 * need a GSMConnectionHandler object as follows
 *
 *    GSMConnectionHandler conMan(SECRET_PIN, SECRET_APN, SECRET_GSM_USER, SECRET_GSM_PASS);
 *
 * If using a MKR NB1500 you'll need a NBConnectionHandler object as follows
 *
 *    NBConnectionHandler conMan(SECRET_PIN);
 *
 * If using a Portenta + Ethernet shield you'll need a EthernetConnectionHandler object as follows:
 *
 * DHCP mode
 *    EthernetConnectionHandler conMan;
 *
 * Manual configuration
 *    EthernetConnectionHandler conMan(SECRET_IP, SECRET_DNS, SECRET_GATEWAY, SECRET_NETMASK);
 *
 * Manual configuration will fallback on DHCP mode if SECRET_IP is invalid or equal to INADDR_NONE.
 *
 */

#include "arduino_secrets.h"

#include <Arduino_ConnectionHandler.h>

#if defined(BOARD_HAS_ETHERNET)
EthernetConnectionHandler conMan(SECRET_IP, SECRET_DNS, SECRET_GATEWAY, SECRET_NETMASK);
#elif defined(BOARD_HAS_WIFI)
WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
GSMConnectionHandler conMan(SECRET_PIN, SECRET_APN, SECRET_GSM_USER, SECRET_GSM_PASS);
#elif defined(BOARD_HAS_NB)
NBConnectionHandler conMan(SECRET_PIN);
#elif defined(BOARD_HAS_LORA)
LoRaConnectionHandler conMan(SECRET_APP_EUI, SECRET_APP_KEY);
#elif defined(BOARD_HAS_CATM1_NBIOT)
CatM1ConnectionHandler conMan(SECRET_PIN, SECRET_APN, SECRET_GSM_USER, SECRET_GSM_PASS);
#elif defined(BOARD_HAS_CELLULAR)
CellularConnectionHandler conMan(SECRET_PIN, SECRET_APN, SECRET_GSM_USER, SECRET_GSM_PASS);
#endif

void setup() {
  Serial.begin(9600);
  /* Give a few seconds for the Serial connection to be available */
  delay(4000);
#ifndef __AVR__
  setDebugMessageLevel(DBG_INFO);
#endif
  conMan.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  conMan.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
  conMan.addCallback(NetworkConnectionEvent::ERROR, onNetworkError);
}

void loop() {
  /* The following code keeps on running connection workflows on our
   * ConnectionHandler object, hence allowing reconnection in case of failure
   * and notification of connect/disconnect event if enabled (see
   * addConnectCallback/addDisconnectCallback) NOTE: any use of delay() within
   * the loop or methods called from it will delay the execution of .update(),
   * which might not guarantee the correct functioning of the ConnectionHandler
   * object.
   */

  conMan.check();
}

void onNetworkConnect() {
  Serial.println(">>>> CONNECTED to network");
}

void onNetworkDisconnect() {
  Serial.println(">>>> DISCONNECTED from network");
}

void onNetworkError() {
  Serial.println(">>>> ERROR");
}
