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
   CONSTRUCTOR/DESTRUCTOR
 ******************************************************************************/

ConnectionHandler::ConnectionHandler(bool const keep_alive)
: _keep_alive{keep_alive}
, _lastConnectionTickTime{millis()}
, _current_net_connection_state{NetworkConnectionState::INIT}
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState ConnectionHandler::check()
{
  unsigned long const now = millis();
  unsigned int const connectionTickTimeInterval = CHECK_INTERVAL_TABLE[static_cast<unsigned int>(_current_net_connection_state)];

  if((now - _lastConnectionTickTime) > connectionTickTimeInterval)
  {
    _lastConnectionTickTime = now;
    NetworkConnectionState next_net_connection_state = _current_net_connection_state;

    /* While the state machine is implemented here, the concrete implementation of the
     * states is done in the derived connection handlers.
     */
    switch (_current_net_connection_state)
    {
      case NetworkConnectionState::INIT:          next_net_connection_state = update_handleInit         (); break;
      case NetworkConnectionState::CONNECTING:    next_net_connection_state = update_handleConnecting   (); break;
      case NetworkConnectionState::CONNECTED:     next_net_connection_state = update_handleConnected    (); break;
      case NetworkConnectionState::DISCONNECTING: next_net_connection_state = update_handleDisconnecting(); break;
      case NetworkConnectionState::DISCONNECTED:  next_net_connection_state = update_handleDisconnected (); break;
      case NetworkConnectionState::ERROR:                                                                   break;
      case NetworkConnectionState::CLOSED:                                                                  break;
    }

    /* Here we are determining whether a state transition from one state to the next has
     * occurred - and if it has, we call eventually registered callbacks.
     */
    if(next_net_connection_state != _current_net_connection_state)
    {
      /* Check the next state to determine the kind of state conversion which has occurred (and call the appropriate callback) */
      if(next_net_connection_state == NetworkConnectionState::CONNECTED)
      {
        if(_on_connect_event_callback) _on_connect_event_callback();
      }
      if(next_net_connection_state == NetworkConnectionState::DISCONNECTED)
      {
        if(_on_disconnect_event_callback) _on_disconnect_event_callback();
      }
      if(next_net_connection_state == NetworkConnectionState::ERROR)
      {
        if(_on_error_event_callback) _on_error_event_callback();
      }

      /* Assign new state to the member variable holding the state */
      _current_net_connection_state = next_net_connection_state;
    }
  }

  return _current_net_connection_state;
}

void ConnectionHandler::connect()
{
  if (_current_net_connection_state != NetworkConnectionState::INIT && _current_net_connection_state != NetworkConnectionState::CONNECTING)
  {
    _keep_alive = true;
    _current_net_connection_state = NetworkConnectionState::INIT;
  }
}

void ConnectionHandler::disconnect()
{
  _keep_alive = false;
  _current_net_connection_state = NetworkConnectionState::DISCONNECTING;
}

void ConnectionHandler::addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback)
{
  switch (event)
  {
    case NetworkConnectionEvent::CONNECTED:         _on_connect_event_callback           = callback; break;
    case NetworkConnectionEvent::DISCONNECTED:      _on_disconnect_event_callback        = callback; break;
    case NetworkConnectionEvent::ERROR:             _on_error_event_callback             = callback; break;
    case NetworkConnectionEvent::CONNECTION_FAILED: _on_connection_failed_event_callback = callback; break;
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
