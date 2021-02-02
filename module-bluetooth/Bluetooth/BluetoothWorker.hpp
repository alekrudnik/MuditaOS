// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Device.hpp"
#include "Service/Worker.hpp"
#include "interface/profiles/Profile.hpp"
#include <FreeRTOS.h>
#include <bsp/bluetooth/Bluetooth.hpp>
#include <memory>
#include <task.h>
#include <vector>
#include "service-bluetooth/SettingsHolder.hpp"
#include "glucode/BluetoothRunLoop.hpp"
#include "interface/BluetoothDriver.hpp"
struct HCI;

/// debug option for HCI (uart) commands debugging
// #define DO_DEBUG_HCI_COMS

namespace bluetooth
{
    enum Message : std::uint8_t
    {
        /// asynchronous messages to use on event from irq
        EvtSending,        /// Bt stack ordered a write transaction and it is pending
        EvtSent,           /// trigger Bt stack wrote, enable writting in HCI in BluetoothWorker task
        EvtSendingError,   /// bsp error on send
        EvtReceiving,      /// Bt stack requested a receive transaction and it is pending
        EvtReceived,       /// trigger Bt stack received, start processing HCI in BluetoothWorker task
        EvtRecUnwanted,    /// not requested recieve - probably receive came to fast from sent...
        EvtReceivingError, /// bsp error on receive
        EvtUartError,      /// generic uart error
        EvtErrorRec,       /// there was error o queue receive
    };

    enum Command : std::uint8_t
    {
        StartScan,
        StopScan,
        VisibilityOn,
        VisibilityOff,
        ConnectAudio,
        DisconnectAudio,
        PowerOn,
        PowerOff,
    };

    inline const char *MessageCstr(Message what)
    {
        switch (what) {
        case EvtReceived:
            return "EvtReceived";
        case EvtSent:
            return "EvtSent";
        case EvtRecUnwanted:
            return "EvtRecUnwanted";
        case EvtReceivingError:
            return "EvtReceivingError";
        case EvtSendingError:
            return "EvtSendingError";
        case EvtUartError:
            return "EvtUartError";
        case EvtErrorRec:
            return "EvtErrorRec";
        default:
            return "";
        }
    }

    struct EvtWorker
    {
        enum Evt : uint8_t
        {
        };
    };
}; // namespace Bt

class BluetoothWorker : private sys::Worker
{
    enum WorkerEventQueues
    {
        queueService = 0,
        queueControl = 1,
        queueIO_handle, /// bsp support queue
        queueCommands,
        queueRunloopTrigger // btstack run_loop queue
    };

    TaskHandle_t bt_worker_task = nullptr;
    int is_running              = false;
    sys::Service *service       = nullptr;

  public:
    enum Error
    {
        SuccessBt,
        ErrorBtGeneric,
        ErrorBtAPI,
    };

    BluetoothWorker(sys::Service *service);
    ~BluetoothWorker() override;

    auto handleMessage(uint32_t queueID) -> bool override;

    auto handleCommand(QueueHandle_t queue) -> bool;
    auto handleBtStackTrigger(QueueHandle_t queue) -> bool;
    bool run();
    auto scan() -> bool;
    void setVisibility(bool visibility);
    auto start_pan() -> bool;
    auto establishAudioConnection() -> bool;
    auto disconnectAudioConnection() -> bool;
    /// bluetooth stack id in use
    unsigned long active_features;
    void stopScan();
    void setDeviceAddress(bd_addr_t addr);
    auto deinit() -> bool override;
    std::shared_ptr<bluetooth::Profile> currentProfile;
    std::shared_ptr<bluetooth::SettingsHolder> settings;
    std::vector<Devicei> pairedDevices;
    std::unique_ptr<bluetooth::RunLoop> runLoop;
    std::unique_ptr<bluetooth::Driver> driver;
};
