﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "Constants.hpp"
#include "service-bluetooth/ServiceBluetooth.hpp"
#include "service-bluetooth/BluetoothMessage.hpp"

#include <module-sys/Service/Bus.hpp>

#include <Bluetooth/BluetoothWorker.hpp>
#include <interface/profiles/Profile.hpp>
#include <MessageType.hpp>
#include <Service/Service.hpp>
#include <Service/Message.hpp>
#include <agents/settings/SystemSettings.hpp>
#include <service-db/Settings.hpp>

#include <log/log.hpp>

#include <bits/exception.h>
#include <utility>

ServiceBluetooth::ServiceBluetooth()
    : sys::Service(service::name::bluetooth), settingsProvider(std::make_unique<settings::Settings>(this))
{
    LOG_INFO("[ServiceBluetooth] Initializing");
}

ServiceBluetooth::~ServiceBluetooth()
{
    LOG_INFO("[ServiceBluetooth] Cleaning resources");
}

// This code is experimental:
// this means it is an init point of bluetooth feature handling
sys::ReturnCodes ServiceBluetooth::InitHandler()
{
    LOG_ERROR("Bluetooth experimental!");
    worker = std::make_unique<BluetoothWorker>(this);
    settingsProvider->registerValueChange(settings::Bluetooth::state,
                                          [this](std::string value) { stateSettingChanged(value); });
    settingsProvider->registerValueChange(settings::Bluetooth::deviceVisibility,
                                          [this](std::string value) { deviceVisibilitySettingChanged(value); });
    settingsProvider->registerValueChange(settings::Bluetooth::deviceName,
                                          [this](std::string value) { deviceNameSettingChanged(value); });
    settingsProvider->registerValueChange(settings::Bluetooth::bondedDevices,
                                          [this](std::string value) { bondedDevicesSettingChanged(value); });

    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceBluetooth::DeinitHandler()
{

    return sys::ReturnCodes::Success;
}

sys::MessagePointer ServiceBluetooth::DataReceivedHandler(sys::DataMessage *msg, sys::ResponseMessage *resp)
{
    try {
        switch (static_cast<MessageType>(msg->messageType)) {
        case MessageType::BluetoothRequest: {
            BluetoothMessage *lmsg = dynamic_cast<BluetoothMessage *>(msg);
            LOG_INFO("Bluetooth request!");
            switch (lmsg->req) {
            case BluetoothMessage::Start:
                worker->run();

                break;
            case BluetoothMessage::Scan:
                if (worker->scan()) {
                    return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Success);
                }
                else {
                    return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Failure);
                }
            case BluetoothMessage::StopScan:
                worker->stopScan();
                break;
            case BluetoothMessage::PAN: {
                /// TODO request lwip first...
                /// because TODO blocking message - wrecks system
                LOG_INFO("Request LwIP running!");
                //                    auto ret = message_lwip(this, LwIP_message::Request::Start);
                //                    if (ret != sys::ReturnCodes::Success) {
                //                        LOG_ERROR("Request for LwIP start failed");
                //                    }
                //                    else {
                /// TODO request PPP
                LOG_INFO("Start PAN");
                worker->start_pan();
                //                    }
            } break;
            case BluetoothMessage::Visible:
                worker->toggleVisibility();
                break;

            case BluetoothMessage::Play:
                worker->establishAudioConnection();
                break;
            case BluetoothMessage::Stop:
                worker->disconnectAudioConnection();
                break;

            default:
                break;
            }
            break;
        }
        case MessageType::BluetoothAddrResult: {
            auto addrMsg = static_cast<BluetoothAddrMessage *>(msg);
            worker->setDeviceAddress(addrMsg->addr);
        } break;
        default:
            LOG_INFO("BT not handled!");
            break;
        }
    }
    catch (std::exception &ex) {
        LOG_ERROR("Exception on BtService!: %s", ex.what());
    }

    return std::make_shared<sys::ResponseMessage>();
}

sys::ReturnCodes ServiceBluetooth::SwitchPowerModeHandler(const sys::ServicePowerMode mode)
{
    LOG_ERROR("TODO");
    return sys::ReturnCodes::Success;
}

void ServiceBluetooth::stateSettingChanged(std::string value)
{
    LOG_DEBUG("Received new bt_state: %s", value.c_str());
    settingsProvider->unregisterValueChange(settings::Bluetooth::state);
}

void ServiceBluetooth::deviceVisibilitySettingChanged(std::string value)
{
    LOG_DEBUG("Received new bt_device_visibility: %s", value.c_str());
    settingsProvider->unregisterValueChange(settings::Bluetooth::deviceVisibility);
}

void ServiceBluetooth::deviceNameSettingChanged(std::string value)
{
    LOG_DEBUG("Received new bt_device_name: %s", value.c_str());
    settingsProvider->unregisterValueChange(settings::Bluetooth::deviceName);
}

void ServiceBluetooth::bondedDevicesSettingChanged(std::string value)
{
    LOG_DEBUG("Received new bt_bonded_devices: %s", value.c_str());
    settingsProvider->unregisterValueChange(settings::Bluetooth::bondedDevices);
}
