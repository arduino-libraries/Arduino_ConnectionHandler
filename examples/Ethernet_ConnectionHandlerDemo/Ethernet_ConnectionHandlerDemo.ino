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
 * If using an Ethernet based shield you will need a EthernetConnectionHandler object as follows
 *
 *      EthernetConnectionHandler conMan(mac);
 *
 * this example was tested on a Teensy4.0 board with a Wiznet850 Ethernet Adapter
 */

#include <Arduino_ConnectionHandler.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetConnectionHandler conMan(mac);

void setup() {
  SPI.begin();
  Ethernet.init(10); // The CS pin on most Arduino Ethernet shields

  Serial.begin(9600);
  /* Give a few seconds for the Serial connection to be available */
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