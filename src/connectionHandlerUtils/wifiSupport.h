/*
 * Copyright (c) Arduino SA
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ConnectionHandlerDefinitions.h"

#if defined(ARDUINO_OPTA) && !defined(ARDUINO_ARCH_ZEPHYR)
#include <opta_info.h>
extern uint8_t* boardInfo();
#endif

/* Depending on the board there may be hardware differences that cannot be established at compile
 * time, for instance opta have factory variants without wifi module present.
 * in order to check for this, we need to check variant configuration at runtime
 */
inline bool isWifiModulePresent() {
#if defined(ARDUINO_OPTA) && !defined(ARDUINO_ARCH_ZEPHYR)
    OptaBoardInfo* info = reinterpret_cast<OptaBoardInfo*>(boardInfo());
    return info->_board_functionalities.wifi == 1;
#elif defined(BOARD_HAS_WIFI)
    return true;
#else
    return false;
#endif
}
