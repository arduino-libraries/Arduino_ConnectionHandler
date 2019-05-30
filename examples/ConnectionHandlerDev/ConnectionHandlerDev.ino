#include "arduino_secrets.h"

#include <FTDebouncer.h>
#include <Arduino_WiFiConnectionHandler.h>

/*		SECRET_ fields are in arduino_secrets.h included above		*/
WiFiConnectionHandler conMan(SECRET_SSID, SECRET_PASS);


#define PIN_CONNECT 2
#define PIN_DISCONNECT 4

FTDebouncer deb;

void setup(){
	Serial.begin(9600);
	unsigned long serialBeginTime = millis();
	//while(millis() - serialBeginTime < 4000 || !Serial){}
	delay(4000);
	setDebugMessageLevel(4);
	deb.addPin(PIN_CONNECT, HIGH, INPUT_PULLUP);
	deb.addPin(PIN_DISCONNECT, HIGH, INPUT_PULLUP);
	deb.init();
	//conMan.addCallback(NetworkConnectionEvent::CONNECTED, &onNetworkConnect);
	conMan.addConnectCallback(onNetworkConnect);
	conMan.addDisconnectCallback(onNetworkDisconnect);
}


void loop(){
	deb.update();
	conMan.update();
}

void connectionStateChanged(NetworkConnectionState _newState){
	Serial.println((int)_newState);
}

void onPinActivated(uint8_t _pinNr){
	Serial.print("activated ");
	Serial.println(_pinNr);
	switch(_pinNr){
		case PIN_CONNECT:
		conMan.connect();
		break;
		case PIN_DISCONNECT:
		conMan.disconnect();
		break;
	}
}
void onPinDeactivated(uint8_t _pinNr){
	Serial.print("deactivated ");
	Serial.println(_pinNr);
	switch(_pinNr){
		default:
		{
		}
		break;
	}
}

void onNetworkConnect(void * _arg){
	Serial.println(">>>> CALLBACK!!!");
	Serial.println(">>>> CONNECTED to network");
}

void onNetworkDisconnect(void * _arg){
	Serial.println(">>>> CALLBACK!!!");
	Serial.println(">>>> DISCONNECTED from network");
}