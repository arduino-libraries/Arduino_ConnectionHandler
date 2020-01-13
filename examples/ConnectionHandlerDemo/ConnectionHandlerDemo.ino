#include "arduino_secrets.h"

#include <Arduino_WiFiConnectionHandler.h>

/*		SECRET_ fields are in arduino_secrets.h included above
      if using a WiFi board (Arduino MKR1000, MKR WiFi 1010, Nano 33 IoT, UNO
   WiFi Rev 2 or ESP8266/32), create a WiFiConnectionHandler object by adding
   Network Name (SECRET_SSID) and password (SECRET_PASS) in the
   arduino_secrets.h file (or Secrets tab in Create Web Editor). (This example
   defaults to WiFi)

      WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);

      If using a MKR GSM 1400 or other GSM boards supporting the same API you'll
   need a GSMConnectionHandler object as follows

      GSMConnectionHandler conMan(SECRET_PIN, SECRET_APN, SECRET_GSM_USER,
   SECRET_GSM_PASS);

      If using a MKR NB1500 you'll need a NBConnectionHandler object as follows

      NBConnectionHandler conMan(SECRET_PIN);
*/

WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);

void setup() {
  Serial.begin(9600);
  // give a few seconds for the Serial connection to be available
  delay(4000);

  setDebugMessageLevel(2);

  // the following methods allow the sketch to be notified when connected or
  // disconnected to the network

  conMan.addConnectCallback(
    onNetworkConnect); // look at function onNetworkConnect towards the end of
  // this sketch
  conMan.addDisconnectCallback(
    onNetworkDisconnect); // look at function onNetworkDisconnect towards the
  // end of this sketch
}

void loop() {
  // the following code keeps on running connection workflows on our
  // ConnectionHandler object, hence allowing reconnection in case of failure
  // and notification of connect/disconnect event if enabled (see
  // addConnectCallback/addDisconnectCallback) NOTE: any use of delay() within
  // the loop or methods called from it will delay the execution of .update(),
  // which might not guarantee the correct functioning of the ConnectionHandler
  // object.

  conMan.update();
}

void onNetworkConnect(void *_arg) {
  Serial.println(">>>> CONNECTED to network");
}

void onNetworkDisconnect(void *_arg) {
  Serial.println(">>>> DISCONNECTED from network");
}
