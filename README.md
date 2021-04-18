Arduino Library for network connections management
==================================================
![](https://github.com/arduino-libraries/Arduino_ConnectionHandler/workflows/Compile%20Examples/badge.svg)
![](https://github.com/arduino-libraries/Arduino_ConnectionHandler/workflows/Spell%20Check/badge.svg)

[![Check Arduino status](https://github.com/arduino-libraries/Arduino_ConnectionHandler/actions/workflows/check-arduino.yml/badge.svg)](https://github.com/arduino-libraries/Arduino_ConnectionHandler/actions/workflows/check-arduino.yml)

Library for handling and managing network connections by providing keep-alive functionality and automatic reconnection in case of connection-loss. It supports the following boards:
* **WiFi**: [`MKR 1000`](https://store.arduino.cc/arduino-mkr1000-wifi), [`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), `ESP8266`
* **GSM**: [`MKR GSM 1400`](https://store.arduino.cc/arduino-mkr-gsm-1400-1415)
* **5G**: [`MKR NB 1500`](https://store.arduino.cc/arduino-mkr-nb-1500-1413)
* **LoRa**: [`MKR WAN 1300/1310`](https://store.arduino.cc/mkr-wan-1310)

### How-to-use

```C++
#include <Arduino_ConnectionHandler.h>
/* ... */
#if defined(BOARD_HAS_WIFI)
WiFiConnectionHandler conMan("SECRET_SSID", "SECRET_PASS");
#elif defined(BOARD_HAS_GSM)
GSMConnectionHandler conMan("SECRET_PIN", "SECRET_APN", "SECRET_GSM_LOGIN", "SECRET_GSM_PASS");
#elif defined(BOARD_HAS_NB)
NBConnectionHandler conMan("SECRET_PIN", "SECRET_APN", "SECRET_GSM_LOGIN", "SECRET_GSM_PASS");
#elif defined(BOARD_HAS_LORA)
LoRaConnectionHandler conMan("SECRET_APP_EUI", "SECRET_APP_KEY");
#endif
/* ... */
void setup() {
  Serial.begin(9600);
  while(!Serial) { }

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
   * the loop or methods called from it will delay the execution of .check(),
   * which might not guarantee the correct functioning of the ConnectionHandler
   * object.
   */
  conMan.check();
}
/* ... */
void onNetworkConnect() {
  Serial.println(">>>> CONNECTED to network");
}

void onNetworkDisconnect() {
  Serial.println(">>>> DISCONNECTED from network");
}

void onNetworkError() {
  Serial.println(">>>> ERROR");
}
```
