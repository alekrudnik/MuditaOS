// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#define DEBUG_APPLICATION_MANAGEMENT 0 /// show verbose logs in ApplicationManager
#define DEBUG_SCOPED_TIMINGS         0 /// show timings in measured functions
#define _RT1051_UART_DEBUG           0 /// show full modem uart communication
#define DEBUG_BLUETOOTH_HCI_COMS     0 /// show communication with BT module - transactions
#define DEBUG_BLUETOOTH_HCI_BYTES    0 /// show communication with BT module - all the HCI bytes
#define DEBUG_MODEM_OUTPUT_RESPONSE  1 /// show full modem output
#define DEBUG_SERVICE_MESSAGES       1 /// show messages prior to handling in service
#define DEBUG_DB_MODEL_DATA          0 /// show messages prior to handling in service
#define DEBUG_FONT                   0 /// show Font debug messages
#define DEBUG_GUI_TEXT               0 /// show basic debug messages for gui::Text - warning this can be hard on cpu
#define DEBUG_GUI_TEXT_LINES         0 /// show extended debug messages for gui::Text - lines building
#define DEBUG_GUI_TEXT_CURSOR        0 /// show extended debug messages for gui::Text - cursor handling
#define DEBUG_TIMER                  0 /// debug timers system utility
