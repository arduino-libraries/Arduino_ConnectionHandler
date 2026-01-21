/*
  This file is part of the Arduino_ConnectionHandler library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "ConnectionHandlerInterface.h"

/******************************************************************************
  CONSTRUCTOR/DESTRUCTOR
 ******************************************************************************/

ConnectionHandler::ConnectionHandler(bool const keep_alive, NetworkAdapter interface,
  bool settings_required)
: _flags{keep_alive, false, settings_required, false}
, _interface{interface}
, _lastConnectionTickTime{millis()}
, _current_net_connection_state{NetworkConnectionState::CHECK}
, _timeoutTable(DefaultTimeoutTable)
{

}

/******************************************************************************
  PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

NetworkConnectionState ConnectionHandler::check()
{
  unsigned long const now = millis();
  unsigned int const connectionTickTimeInterval =
    _timeoutTable.intervals[static_cast<unsigned int>(_current_net_connection_state)];

  if((now - _lastConnectionTickTime) > connectionTickTimeInterval)
  {
    _lastConnectionTickTime = now;

    NetworkConnectionState old_net_connection_state = _current_net_connection_state;
    NetworkConnectionState next_net_connection_state = updateConnectionState();

    /* Here we are determining whether a state transition from one state to the next has
     * occurred - and if it has, we call eventually registered callbacks.
     */

    if(old_net_connection_state != next_net_connection_state) {
      updateCallback(next_net_connection_state);

      /* It may happen that the local _current_net_connection_state
       * is not updated by the updateConnectionState() call. This is the case for GenericConnection handler
       * where the call of updateConnectionState() is replaced by the inner ConnectionHandler call
       * that updates its state, but not the outer one. For this reason it is required to perform this call twice
       */
      _current_net_connection_state = next_net_connection_state;
    }
  }

  return _current_net_connection_state;
}

NetworkConnectionState ConnectionHandler::updateConnectionState() {
  NetworkConnectionState next_net_connection_state = _current_net_connection_state;

  /* While the state machine is implemented here, the concrete implementation of the
   * states is done in the derived connection handlers.
   */
  switch (_current_net_connection_state)
  {
    case NetworkConnectionState::CHECK:         next_net_connection_state = update_handleCheck        (); break;
    case NetworkConnectionState::INIT:          next_net_connection_state = update_handleInit         (); break;
    case NetworkConnectionState::CONNECTING:    next_net_connection_state = update_handleConnecting   (); break;
    case NetworkConnectionState::CONNECTED:     next_net_connection_state = update_handleConnected    (); break;
    case NetworkConnectionState::DISCONNECTING: next_net_connection_state = update_handleDisconnecting(); break;
    case NetworkConnectionState::DISCONNECTED:  next_net_connection_state = update_handleDisconnected (); break;
    case NetworkConnectionState::ERROR:                                                                   break;
    case NetworkConnectionState::CLOSED:                                                                  break;
  }

  /* Assign new state to the member variable holding the state */
  _current_net_connection_state = next_net_connection_state;

  return next_net_connection_state;
}

void ConnectionHandler::updateCallback(NetworkConnectionState next_net_connection_state) {

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
}

void ConnectionHandler::connect()
{
  if (_current_net_connection_state != NetworkConnectionState::INIT && _current_net_connection_state != NetworkConnectionState::CONNECTING)
  {
    _flags.keep_alive = true;
    _current_net_connection_state = NetworkConnectionState::INIT;
  }
}

void ConnectionHandler::disconnect()
{
  _flags.keep_alive = false;
  _current_net_connection_state = NetworkConnectionState::DISCONNECTING;
}

void ConnectionHandler::addCallback(NetworkConnectionEvent const event, OnNetworkEventCallback callback)
{
  switch (event)
  {
    case NetworkConnectionEvent::CONNECTED:    _on_connect_event_callback    = callback; break;
    case NetworkConnectionEvent::DISCONNECTED: _on_disconnect_event_callback = callback; break;
    case NetworkConnectionEvent::ERROR:        _on_error_event_callback      = callback; break;
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

NetworkConnectionState ConnectionHandler::update_handleCheck() {
  if(_flags.settings_required && !_flags.settings_provided) {
    return NetworkConnectionState::CHECK;
  } else {
    return NetworkConnectionState::INIT;
  }
}
