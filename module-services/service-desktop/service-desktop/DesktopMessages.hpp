﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <endpoints/developerMode/DeveloperModeEndpoint.hpp>
#include <endpoints/update/UpdateOSTypes.hpp>
#include <service-appmgr/Actions.hpp>
#include <service-appmgr/messages/ActionRequest.hpp>
#include <Service/Message.hpp>
#include <MessageType.hpp>
#include <service-desktop/DeveloperModeMessage.hpp>
#include <service-desktop/DesktopEvent.hpp>

namespace sdesktop
{
    class UpdateOsMessage : public sys::DataMessage
    {
      public:
        UpdateOsMessage(const std::string updateFilePath, const uint32_t requestUUID)
            : sys::DataMessage(MessageType::UpdateOS)
        {
            updateStats.updateFile = updateFilePath;
            updateStats.uuid       = requestUUID;
        };

        UpdateOsMessage() : sys::DataMessage(MessageType::UpdateOS)
        {}

        UpdateOsMessage(const updateos::UpdateMessageType updateMessageType)
            : sys::DataMessage(MessageType::UpdateOS), messageType(updateMessageType)
        {}

        UpdateOsMessage(const updateos::UpdateMessageType updateMessageType, fs::path updateFileFoundOnBoot)
            : sys::DataMessage(MessageType::UpdateOS), messageType(updateMessageType)
        {
            updateStats.updateFile = updateFileFoundOnBoot;
        }

        ~UpdateOsMessage() override = default;

        updateos::UpdateStats updateStats       = {};
        updateos::UpdateMessageType messageType = updateos::UpdateMessageType::UpdateNow;
    };

    class BackupMessage : public sys::DataMessage
    {
      public:
        BackupMessage() : sys::DataMessage(MessageType::Backup)
        {}
        ~BackupMessage() override = default;
    };

    class RestoreMessage : public sys::DataMessage
    {
      public:
        RestoreMessage() : sys::DataMessage(MessageType::Restore)
        {}
        ~RestoreMessage() override = default;
    };

    class FactoryMessage : public sys::DataMessage
    {
      public:
        FactoryMessage() : sys::DataMessage(MessageType::Factory)
        {}
        ~FactoryMessage() override = default;
    };

    namespace usb
    {
        enum class USBConfigurationType
        {
            firstConfiguration,
            reconfiguration
        };

        class USBConnected : public sys::DataMessage
        {
          public:
            USBConnected() : sys::DataMessage(MessageType::USBConnected)
            {}
            ~USBConnected() override = default;
        };

        class USBConfigured : public sys::DataMessage
        {
          public:
            explicit USBConfigured(
                USBConfigurationType configurationType = sdesktop::usb::USBConfigurationType::firstConfiguration);
            ~USBConfigured() override = default;
            USBConfigurationType getConfigurationType() const noexcept;

          private:
            USBConfigurationType configurationType;
        };

        class USBDisconnected : public sys::DataMessage
        {
          public:
            USBDisconnected() : sys::DataMessage(MessageType::USBDisconnected)
            {}
            ~USBDisconnected() override = default;
        }; // namespace usb

    } // namespace usb

    namespace developerMode
    {
        class AppFocusChangeEvent : public Event
        {
          public:
            AppFocusChangeEvent() = default;
            explicit AppFocusChangeEvent(std::string appName);
        };

        class CellularHotStartEvent : public Event
        {
          public:
            CellularHotStartEvent() = default;
        };
        class CellularStateInfoRequestEvent : public Event
        {
          public:
            CellularStateInfoRequestEvent() = default;
            explicit CellularStateInfoRequestEvent(std::string stateStr);
        };
        class CellularSleepModeInfoRequestEvent : public Event
        {
          public:
            CellularSleepModeInfoRequestEvent() = default;
            explicit CellularSleepModeInfoRequestEvent(bool isInSleepMode);
        };
        class ScreenlockCheckEvent : public Event
        {
          public:
            ScreenlockCheckEvent() = default;
            explicit ScreenlockCheckEvent(bool isLocked);
        };
    } // namespace developerMode

    class DeviceInfoRequestEvent : public Event
    {
      public:
        DeviceInfoRequestEvent() = default;
        explicit DeviceInfoRequestEvent(const std::string &networkOperatorName);
    };

    namespace transfer
    {
        class TransferTimerState : public sys::DataMessage
        {
          public:
            enum Request
            {
                None,
                Start,
                Reload,
                Stop
            };
            enum Request req = Request::None;
            TransferTimerState(enum Request req = None) : sys::DataMessage(MessageType::TransferTimer), req(req){};
            ~TransferTimerState() override = default;
        };
    } // namespace transfer

} // namespace sdesktop
