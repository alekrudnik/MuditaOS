// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CommandHandler.hpp"
#include <service-bluetooth/ServiceBluetooth.hpp>

#include <utility>
#include <service-bluetooth/SettingsHolder.hpp>
#include <Service/Service.hpp>

#include "Device.hpp"
#include "BtCommand.hpp"

#include <service-desktop/service-desktop/Constants.hpp>
#include <service-bluetooth/messages/ResponseVisibleDevices.hpp>
#include "GAP/GAP.hpp"

extern "C"
{
#include <module-bluetooth/lib/btstack/src/btstack_util.h>
}

namespace bluetooth
{
    namespace
    {
        [[nodiscard]] auto toString(bluetooth::Error::Code code) -> std::string
        {
            return utils::enumToString(code);
        }
    } // namespace

    CommandHandler::CommandHandler(sys::Service *service,
                                   std::shared_ptr<bluetooth::SettingsHolder> settings,
                                   std::shared_ptr<bluetooth::ProfileManager> profileManager,
                                   std::shared_ptr<bluetooth::AbstractDriver> driver)
        : service{service}, settings{std::move(settings)}, profileManager{std::move(profileManager)}, driver{std::move(
                                                                                                          driver)}
    {}

    Error::Code CommandHandler::handle(Command command)
    {
        switch (command.getType()) {
        case bluetooth::Command::PowerOn:
            return Error::Success;
        case bluetooth::Command::StartScan:
            return scan();
        case bluetooth::Command::getDevicesAvailable:
            return availableDevices();
        case bluetooth::Command::StopScan:
            return stopScan();
        case bluetooth::Command::StartPan:
            return startPan();
        case bluetooth::Command::Pair:
            return pair(command.getAddress());
        case bluetooth::Command::Unpair:
            return unpair(command.getAddress());
        case bluetooth::Command::VisibilityOn:
            return setVisibility(true);
        case bluetooth::Command::VisibilityOff:
            return setVisibility(false);
        case bluetooth::Command::ConnectAudio:
            return establishAudioConnection(command.getAddress());
        case bluetooth::Command::DisconnectAudio:
            return disconnectAudioConnection();
        case bluetooth::Command::PowerOff:
            return Error::Success;
        case bluetooth::Command::SwitchProfile:
            return switchAudioProfile();
        case bluetooth::Command::None:
            return Error::Success;
        case Command::StartRinging:
            return profileManager->startRinging();
        case Command::StopRinging:
            return profileManager->stopRinging();
        case Command::StartRouting:
            return profileManager->initializeCall();
        case Command::StartStream:
            profileManager->start();
            return Error::Success;
        case Command::StopStream:
            profileManager->stop();
            return Error::Success;
        }
        return Error::LibraryError;
    }

    Error::Code CommandHandler::scan()
    {

        if (const auto ret = driver->scan(); ret.err != bluetooth::Error::Success) {
            LOG_ERROR("Cant start scan!: %s %" PRIu32 "", toString(ret.err).c_str(), ret.lib_code);
            return ret.err;
        }

        LOG_INFO("Scan started!");
        // open new scan window
        return Error::Success;
    }

    Error::Code CommandHandler::stopScan()
    {
        LOG_INFO("Stopping scan!");
        driver->stopScan();
        return Error::Success;
    }

    Error::Code CommandHandler::startPan()
    {
        bluetooth::PAN::bnep_setup();
        if (const auto err = bluetooth::PAN::bnep_start(); err.err != bluetooth::Error::Success) {
            LOG_ERROR("PAN setup error: %s %" PRIu32, toString(err.err).c_str(), err.lib_code);
            return err.err;
        }
        return bluetooth::Error::Success;
    }

    Error::Code CommandHandler::setVisibility(bool visibility)
    {
        driver->setVisibility(visibility);
        settings->setValue(bluetooth::Settings::Visibility, static_cast<int>(visibility));
        return Error::Success;
    }

    Error::Code CommandHandler::establishAudioConnection(uint8_t *addr)
    {
        profileManager->init();
        LOG_INFO("Connecting audio with %s", bd_addr_to_str(addr));
        profileManager->connect(addr);
        return Error::Success;
    }

    Error::Code CommandHandler::disconnectAudioConnection()
    {
        profileManager->disconnect();
        return Error::Success;
    }
    Error::Code CommandHandler::pair(bd_addr_t addr)
    {
        LOG_INFO("Pairing with %s", bd_addr_to_str(addr));

        return driver->pair(addr) ? Error::Success : Error::LibraryError;
    }
    Error::Code CommandHandler::switchAudioProfile()
    {
        static auto profile = AudioProfile::A2DP;
        if (profile == AudioProfile::A2DP) {
            profile = AudioProfile::HSP;
            LOG_INFO("New profile: HSP");
        }
        else {
            profile = AudioProfile::A2DP;
            LOG_INFO("New profile: A2DP");
        }
        profileManager->switchProfile(profile);
        return Error::Success;
    }
    Error::Code CommandHandler::unpair(uint8_t *addr)
    {
        LOG_INFO("Unpairing %s", bd_addr_to_str(addr));

        return driver->unpair(addr) ? Error::Success : Error::LibraryError;
    }

    Error::Code CommandHandler::availableDevices()
    {
        auto msg = std::make_shared<message::bluetooth::ResponseVisibleDevices>(bluetooth::GAP::getDevicesList());
        static_cast<ServiceBluetooth *>(service)->bus.sendUnicast(std::move(msg), service::name::service_desktop);

        return Error::Success;
    }
} // namespace bluetooth
