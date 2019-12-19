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

#include "Arduino_ConnectionHandler.h"

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ConnectionHandler::addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback) {
  switch (event) {
    case NetworkConnectionEvent::CONNECTED:       _on_connect_event_callback       = callback; break;
    case NetworkConnectionEvent::DISCONNECTED:    _on_disconnect_event_callback    = callback; break;
    case NetworkConnectionEvent::ERROR:           _on_error_event_callback         = callback; break;
    case NetworkConnectionEvent::INIT:                                                       ; break;
    case NetworkConnectionEvent::CONNECTING:                                                 ; break;
    case NetworkConnectionEvent::DISCONNECTING:                                              ; break;
    case NetworkConnectionEvent::CLOSED:                                                     ; break;
  }
}

void ConnectionHandler::addConnectCallback(OnNetworkEventCallback callback) {
  _on_connect_event_callback = callback;
}
void ConnectionHandler::addDisconnectCallback(OnNetworkEventCallback callback) {
  _on_disconnect_event_callback = callback;
}
void ConnectionHandler::addErrorCallback(OnNetworkEventCallback callback) {
  _on_error_event_callback = callback;
}

void ConnectionHandler::execNetworkEventCallback(OnNetworkEventCallback & callback, void * callback_arg) {
  if (callback) {
    (*callback)(callback_arg);
  }
}

