/* SECRET_ fields are in `arduino_secrets.h` (included below)
 *
 * If using a Host + Notecard connected over I2C you'll need a
 * NotecardConnectionHandler object as follows:
 *
 *   NotecardConnectionHandler conMan(NOTECARD_PRODUCT_UID);
 *
 * If using a Host + Notecard connected over Serial you'll need a
 * NotecardConnectionHandler object as follows:
 *
 *   NotecardConnectionHandler conMan(NOTECARD_PRODUCT_UID, UART_INTERFACE);
 */

#include <Notecard.h> // MUST include this first to enable Notecard support
#include <Arduino_ConnectionHandler.h>

#include "arduino_secrets.h"

/* Uncomment the following line to use this example in a manner that is more
 * compatible with LoRa.
 */
// #define USE_NOTE_LORA

#ifndef USE_NOTE_LORA
#define CONN_TOGGLE_MS 60000
#else
#define CONN_TOGGLE_MS 300000
#endif

/* The Notecard can provide connectivity to almost any board via ESLOV (I2C)
 * or UART. An empty string (or the default value provided below) will not
 * override the Notecard's existing configuration.
 * Learn more at: https://dev.blues.io */
#define NOTECARD_PRODUCT_UID "com.domain.you:product"

/* Uncomment the following line to use the Notecard over UART */
// #define UART_INTERFACE Serial1

#ifndef UART_INTERFACE
NotecardConnectionHandler conMan(NOTECARD_PRODUCT_UID);
#else
NotecardConnectionHandler conMan(NOTECARD_PRODUCT_UID, UART_INTERFACE);
#endif

bool attemptConnect = false;
uint32_t lastConnToggleMs = 0;

void setup() {
  /* Initialize serial debug port and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime <= 5000); ) { }

  /* Set the debug message level:
   * - DBG_ERROR: Only show error messages
   * - DBG_WARNING: Show warning and error messages
   * - DBG_INFO: Show info, warning, and error messages
   * - DBG_DEBUG: Show debug, info, warning, and error messages
   * - DBG_VERBOSE: Show all messages
   */
  setDebugMessageLevel(DBG_INFO);

  /* Add callbacks to the ConnectionHandler object to get notified of network
   * connection events. */
  conMan.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  conMan.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
  conMan.addCallback(NetworkConnectionEvent::ERROR, onNetworkError);

  /* First call to `check()` initializes the connection to the Notecard. While
   * not strictly necessary, it cleans up the logging from this application.
   */
  conMan.check();

#ifndef USE_NOTE_LORA
  /* Set the Wi-Fi credentials for the Notecard */
  String ssid = SECRET_WIFI_SSID;
  if (ssid.length() > 0 && ssid != "NETWORK NAME") {
    conMan.setWiFiCredentials(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
  }
#else
  conMan.setNotehubPollingInterval(720);  // poll twice per day
#endif

  /* Confirm Interface */
  Serial.print("Network Adapter Interface: ");
  if (NetworkAdapter::NOTECARD == conMan.getInterface()) {
    Serial.print("Notecard ");
    Serial.print(conMan.getNotecardUid());
#ifndef UART_INTERFACE
    Serial.println(" (via I2C)");
#else
    Serial.println(" (via UART)");
#endif
  } else {
    Serial.println("ERROR: Unexpected Interface");
    while(1);
  }

  /* Display the Arduino IoT Cloud Device ID */
  displayCachedDeviceId();
}

void loop() {
  /* Toggle the connection every `CONN_TOGGLE_MS` milliseconds */
  if ((millis() - lastConnToggleMs) > CONN_TOGGLE_MS) {
    Serial.println("Toggling connection...");
    if (attemptConnect) {
      displayCachedDeviceId();
      conMan.connect();
    } else {
      // Flush any queued Notecard requests before disconnecting
      conMan.initiateNotehubSync(NotecardConnectionHandler::SyncType::Outbound);
      conMan.disconnect();
    }
    attemptConnect = !attemptConnect;
    lastConnToggleMs = millis();
  }

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

void displayCachedDeviceId() {
  Serial.print("Cached Arduino IoT Cloud Device ID: ");
  Serial.println(conMan.getDeviceId());
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
