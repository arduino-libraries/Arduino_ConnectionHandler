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

#include "GenericConnectionHandler.h"
#include "Arduino_ConnectionHandler.h"

static inline ConnectionHandler* instantiate_handler(NetworkAdapter adapter);

bool GenericConnectionHandler::updateSetting(const models::NetworkSetting& s) {
    if(_ch != nullptr && _ch->_current_net_connection_state != NetworkConnectionState::INIT) {
        // If the internal connection handler is already being used and not in INIT phase we cannot update the settings
        return false;
    } else if(_ch != nullptr && _ch->_current_net_connection_state == NetworkConnectionState::INIT && _interface != s.type) {
        // If the internal connection handler is already being used and in INIT phase and the interface type is being changed
        // -> we need to deallocate the previously allocated handler

        // if interface type is not being changed -> we just need to call updateSettings
        delete _ch;
        _ch = nullptr;
    }

    if(_ch == nullptr) {
        _ch = instantiate_handler(s.type);
    }

    if(_ch != nullptr) {
        _interface = s.type;
        _ch->setKeepAlive(_keep_alive);
        _ch->enableCheckInternetAvailability(_check_internet_availability);
        return _ch->updateSetting(s);
    } else {
        _interface = NetworkAdapter::NONE;

        return false;
    }
}

void GenericConnectionHandler::getSetting(models::NetworkSetting& s) {
    if(_ch != nullptr) {
        _ch->getSetting(s);
    } else {
        s.type = NetworkAdapter::NONE;
    }
}

NetworkConnectionState GenericConnectionHandler::updateConnectionState() {
    return _ch != nullptr ? _ch->updateConnectionState() : NetworkConnectionState::INIT;
}

NetworkConnectionState GenericConnectionHandler::update_handleInit() {
    return _ch != nullptr ? _ch->update_handleInit() : NetworkConnectionState::INIT;
}

NetworkConnectionState GenericConnectionHandler::update_handleConnecting() {
    return _ch != nullptr ? _ch->update_handleConnecting() : NetworkConnectionState::INIT;
}

NetworkConnectionState GenericConnectionHandler::update_handleConnected() {
    return _ch != nullptr ? _ch->update_handleConnected() : NetworkConnectionState::INIT;
}

NetworkConnectionState GenericConnectionHandler::update_handleDisconnecting() {
    return _ch != nullptr ? _ch->update_handleDisconnecting() : NetworkConnectionState::INIT;
}

NetworkConnectionState GenericConnectionHandler::update_handleDisconnected() {
    return _ch != nullptr ? _ch->update_handleDisconnected() : NetworkConnectionState::INIT;
}

#if not (defined(BOARD_HAS_LORA) or defined(BOARD_HAS_NOTECARD))
unsigned long GenericConnectionHandler::getTime() {
    return _ch != nullptr ? _ch->getTime() : 0;
}

Client & GenericConnectionHandler::getClient() {
    return _ch->getClient(); // NOTE _ch may be nullptr
}

UDP & GenericConnectionHandler::getUDP() {
    return _ch->getUDP(); // NOTE _ch may be nullptr
}

#endif // not (defined(BOARD_HAS_LORA) or defined(BOARD_HAS_NOTECARD))

void GenericConnectionHandler::connect() {
    if(_ch!=nullptr) {
        _ch->connect();
    }
    ConnectionHandler::connect();
}

void GenericConnectionHandler::disconnect() {
    if(_ch!=nullptr) {
        _ch->disconnect();
    }
    ConnectionHandler::disconnect();
}

void GenericConnectionHandler::setKeepAlive(bool keep_alive) {
    _keep_alive = keep_alive;

    if(_ch!=nullptr) {
        _ch->setKeepAlive(keep_alive);
    }
}

static inline ConnectionHandler* instantiate_handler(NetworkAdapter adapter) {
    switch(adapter) {
        #if defined(BOARD_HAS_WIFI)
        case NetworkAdapter::WIFI:
            return new WiFiConnectionHandler();
            break;
        #endif

        #if defined(BOARD_HAS_ETHERNET)
        case NetworkAdapter::ETHERNET:
            return new EthernetConnectionHandler();
            break;
        #endif

        #if defined(BOARD_HAS_NB)
        case NetworkAdapter::NB:
            return new NBConnectionHandler();
            break;
        #endif

        #if defined(BOARD_HAS_GSM)
        case NetworkAdapter::GSM:
            return new GSMConnectionHandler();
            break;
        #endif

        #if defined(BOARD_HAS_CATM1_NBIOT)
        case NetworkAdapter::CATM1:
            return new CatM1ConnectionHandler();
            break;
        #endif

        #if defined(BOARD_HAS_CELLULAR)
        case NetworkAdapter::CELL:
            return new CellularConnectionHandler();
            break;
        #endif

        default:
            Debug.print(DBG_ERROR, "Network adapter not supported by this platform: %d", adapter);
            return nullptr;
    }
}
