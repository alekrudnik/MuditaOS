﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <Application.hpp>

namespace gui::window::name
{
    inline constexpr auto bellAlarm = "BellAlarm";
} // namespace gui::window::name

namespace app
{
    namespace internal
    {
        class BellAlarmPriv;
    } // namespace internal

    inline constexpr auto applicationBellAlarmName = "ApplicationBellAlarm";

    class ApplicationBellAlarm : public Application
    {
      private:
        std::unique_ptr<internal::BellAlarmPriv> priv;

      public:
        explicit ApplicationBellAlarm(
            std::string name                            = applicationBellAlarmName,
            std::string parent                          = "",
            sys::phone_modes::PhoneMode mode            = sys::phone_modes::PhoneMode::Offline,
            sys::bluetooth::BluetoothMode bluetoothMode = sys::bluetooth::BluetoothMode::Disabled,
            StartInBackground startInBackground         = {false},
            std::uint32_t stackDepth                    = 8192);

        ~ApplicationBellAlarm() override;
        sys::ReturnCodes InitHandler() override;

        void createUserInterface() override;
        void destroyUserInterface() override
        {}

        sys::MessagePointer DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp) override;

        sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final
        {
            return sys::ReturnCodes::Success;
        }
    };

    template <> struct ManifestTraits<ApplicationBellAlarm>
    {
        static auto GetManifest() -> manager::ApplicationManifest
        {
            return {{manager::actions::Launch}};
        }
    };
} // namespace app
