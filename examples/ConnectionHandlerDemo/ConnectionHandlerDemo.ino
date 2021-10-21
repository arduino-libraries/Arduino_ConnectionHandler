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
 */


//#define ARDUINO_ETHERNET_SHIELD /* Uncomment this line if you want to use a ethernet shield */

#include "arduino_secrets.h"

#include <Arduino_ConnectionHandler.h>

#if defined(BOARD_HAS_WIFI)
WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
GSMConnectionHandler conMan(SECRET_APN, SECRET_PIN, SECRET_GSM_USER, SECRET_GSM_PASS);
#elif defined(BOARD_HAS_NB)
NBConnectionHandler conMan(SECRET_PIN);
#elif defined(BOARD_HAS_LORA)
LoRaConnectionHandler conMan(SECRET_APP_EUI, SECRET_APP_KEY);
#elif defined(BOARD_HAS_ETHERNET)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetConnectionHandler conMan(mac);
#endif

void setup() {
  Serial.begin(9600);
  /* Give a few seconds for the Serial connection to be available */
  #if defined(BOARD_HAS_ETHERNET)
    SPI.begin();
    Ethernet.init(10); // CS pin on most Arduino shields
  #endif
  delay(4000);

  setDebugMessageLevel(DBG_INFO);

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
