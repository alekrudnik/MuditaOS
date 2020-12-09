// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
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

struct HCI;

/// debug option for HCI (uart) commands debugging
// #define DO_DEBUG_HCI_COMS

namespace Bt
{
    enum Message : uint8_t
    {
        /// asynchronous messages to use on event from irq
        EvtSent,        /// trigger Bt stack wrote, enable writting in HCI in BluetoothWorker task
        EvtRecUnwanted, /// not requested recieve - probably receive came to fast from sent...
        EvtRecError,    /// bsp error on receive
        EvtSentError,   /// bsp error on send
        EvtUartError,   /// generic uart error
        EvtReceived,    /// trigger Bt stack received, start processing HCI in BluetoothWorker task
        EvtErrorRec,    /// there was error o queue receive
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
        case EvtRecError:
            return "EvtRecError";
        case EvtSentError:
            return "EvtSentError";
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
        queue_profiles, /// queue for communication between profile workers,
                        /// main bt_worker_task should dispatch these in events
    };

    TaskHandle_t bt_worker_task = nullptr;
    sys::Service *service       = nullptr;
    int is_running              = false;

  public:
    enum Error
    {
        SuccessBt,
        ErrorBtGeneric,
        ErrorBtAPI,
    };

    BluetoothWorker(sys::Service *service);
    virtual ~BluetoothWorker();

    virtual bool handleMessage(uint32_t queueID);

    bool run();

    bool scan();

    bool toggleVisibility();

    bool start_pan();

    bool establishAudioConnection();

    bool disconnectAudioConnection();

    Error aud_init();
    /// bluetooth stack id in use
    unsigned long active_features;
    void stopScan();
    void setDeviceAddress(bd_addr_t addr);
    void initAudioBT();

    std::shared_ptr<Bt::Profile> currentProfile;
};
