﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ApplicationSettings.hpp"

#include "windows/AddDeviceWindow.hpp"
#include "windows/AllDevicesWindow.hpp"
#include "windows/ApnSettingsWindow.hpp"
#include "windows/ApnOptionsWindow.hpp"
#include "windows/BluetoothWindow.hpp"
#include "windows/SettingsMainWindow.hpp"
#include "windows/DisplayAndKeypadWindow.hpp"
#include "windows/InputLanguageWindow.hpp"
#include "windows/LockedScreenWindow.hpp"
#include "windows/FontSizeWindow.hpp"
#include "windows/DisplayLightWindow.hpp"
#include "windows/KeypadLightWindow.hpp"
#include "windows/AppsAndToolsWindow.hpp"
#include "windows/NightshiftWindow.hpp"
#include "windows/NetworkWindow.hpp"
#include "windows/MessagesWindow.hpp"
#include "windows/PhoneNameWindow.hpp"
#include "windows/AutolockWindow.hpp"
#include "windows/TorchWindow.hpp"
#include "windows/WallpaperWindow.hpp"
#include "windows/QuotesMainWindow.hpp"
#include "windows/QuotesAddWindow.hpp"
#include "windows/SecurityMainWindow.hpp"
#include "windows/QuotesOptionsWindow.hpp"
#include "windows/ChangePasscodeWindow.hpp"
#include "windows/SystemMainWindow.hpp"
#include "windows/NewApnWindow.hpp"
#include "windows/LanguagesWindow.hpp"

#include "Dialog.hpp"

#include <service-evtmgr/EventManagerServiceAPI.hpp>
#include <service-cellular/CellularServiceAPI.hpp>
#include <service-bluetooth/BluetoothMessage.hpp>
#include <service-bluetooth/service-bluetooth/messages/Status.hpp>
#include <service-bluetooth/messages/BondedDevices.hpp>
#include <service-bluetooth/messages/DeviceName.hpp>
#include <service-db/agents/settings/SystemSettings.hpp>
#include <application-settings-new/data/ApnListData.hpp>
#include <application-settings-new/data/BondedDevicesData.hpp>
#include <application-settings-new/data/LanguagesData.hpp>
#include <application-settings-new/data/PhoneNameData.hpp>
#include <module-services/service-db/agents/settings/SystemSettings.hpp>
#include <service-db/Settings.hpp>

#include <i18n/i18n.hpp>
#include <module-services/service-evtmgr/service-evtmgr/ScreenLightControlMessage.hpp>
#include <module-services/service-evtmgr/service-evtmgr/Constants.hpp>
#include <module-services/service-appmgr/service-appmgr/messages/Message.hpp>
#include <module-services/service-appmgr/service-appmgr/model/ApplicationManager.hpp>

namespace app
{
    namespace settings
    {
        constexpr inline auto operators_on = "operators_on";
        const std::string quotesPath =
            purefs::createPath(purefs::dir::getUserDiskPath(), "data/applications/settings/quotes.json");
    } // namespace settings

    ApplicationSettingsNew::ApplicationSettingsNew(std::string name,
                                                   std::string parent,
                                                   StartInBackground startInBackground)
        : Application(name, parent, startInBackground)
    {
        if ((Store::GSM::SIM::SIM1 == selectedSim || Store::GSM::SIM::SIM2 == selectedSim) &&
            Store::GSM::get()->sim == selectedSim) {
            selectedSimNumber = CellularServiceAPI::GetOwnNumber(this);
        }
    }

    ApplicationSettingsNew::~ApplicationSettingsNew()
    {
    }

    // Invoked upon receiving data message
    auto ApplicationSettingsNew::DataReceivedHandler(sys::DataMessage *msgl,
                                                     [[maybe_unused]] sys::ResponseMessage *resp) -> sys::MessagePointer
    {
        auto retMsg = Application::DataReceivedHandler(msgl);
        // if message was handled by application's template there is no need to process further.
        auto rm = dynamic_cast<sys::ResponseMessage *>(retMsg.get());
        if (nullptr != rm && sys::ReturnCodes::Success == rm->retCode) {
            return retMsg;
        }

        if (auto btMsg = dynamic_cast<BluetoothScanResultMessage *>(msgl); btMsg != nullptr) {
            auto scannedBtDevices = btMsg->devices;
            LOG_INFO("Received BT Scan message!");

            auto data = std::make_unique<gui::DeviceData>(scannedBtDevices);
            windowsFactory.build(this, gui::window::name::add_device);
            switchWindow(gui::window::name::add_device, gui::ShowMode::GUI_SHOW_INIT, std::move(data));
            render(gui::RefreshModes::GUI_REFRESH_FAST);
        }
        else if (auto phoneMsg = dynamic_cast<CellularNotificationMessage *>(msgl); nullptr != phoneMsg) {
            selectedSim = Store::GSM::get()->selected;
            if (CellularNotificationMessage::Type::SIM_READY == phoneMsg->type) {
                selectedSimNumber = CellularServiceAPI::GetOwnNumber(this);
            }
            else if (CellularNotificationMessage::Type::SIM_NOT_READY == phoneMsg->type) {
                selectedSimNumber = {};
            }
            auto currentWindow = getCurrentWindow();
            if (gui::window::name::network == currentWindow->getName()) {
                currentWindow->rebuild();
            }
        }
        else if (auto responseStatusMsg = dynamic_cast<::message::bluetooth::ResponseStatus *>(msgl);
                 nullptr != responseStatusMsg) {
            if (gui::window::name::bluetooth == getCurrentWindow()->getName()) {
                auto btStatusData = std::make_unique<gui::BluetoothStatusData>(responseStatusMsg->getStatus());
                switchWindow(gui::window::name::bluetooth, std::move(btStatusData));
            }
        }

        return sys::MessageNone{};
    }

    // Invoked during initialization
    auto ApplicationSettingsNew::InitHandler() -> sys::ReturnCodes
    {
        board = EventManagerServiceAPI::GetBoard(this);

        auto ret = Application::InitHandler();
        if (ret != sys::ReturnCodes::Success) {
            return ret;
        }

        connect(typeid(::message::bluetooth::ResponseDeviceName), [&](sys::Message *msg) {
            auto responseDeviceNameMsg = static_cast<::message::bluetooth::ResponseDeviceName *>(msg);
            if (gui::window::name::phone_name == getCurrentWindow()->getName()) {
                auto phoneNameData = std::make_unique<gui::PhoneNameData>(responseDeviceNameMsg->getName());
                switchWindow(gui::window::name::phone_name, std::move(phoneNameData));
            }
            return sys::MessageNone{};
        });

        connect(typeid(::message::bluetooth::ResponseBondedDevices), [&](sys::Message *msg) {
            auto responseBondedDevicesMsg = static_cast<::message::bluetooth::ResponseBondedDevices *>(msg);
            if (gui::window::name::all_devices == getCurrentWindow()->getName()) {
                auto bondedDevicesData =
                    std::make_unique<gui::BondedDevicesData>(responseBondedDevicesMsg->getDevices());
                switchWindow(gui::window::name::all_devices, std::move(bondedDevicesData));
            }
            return sys::MessageNone{};
        });

        connect(typeid(CellularGetAPNResponse), [&](sys::Message *msg) {
            if (gui::window::name::apn_settings == getCurrentWindow()->getName()) {
                auto apns = dynamic_cast<CellularGetAPNResponse *>(msg);
                if (apns != nullptr) {
                    auto apnsData = std::make_unique<gui::ApnListData>(apns->getAPNs());
                    switchWindow(gui::window::name::apn_settings, std::move(apnsData));
                }
            }
            return sys::MessageNone{};
        });

        connect(typeid(manager::GetCurrentDisplayLanguageResponse), [&](sys::Message *msg) {
            if (gui::window::name::languages == getCurrentWindow()->getName()) {
                auto response = dynamic_cast<manager::GetCurrentDisplayLanguageResponse *>(msg);
                if (response != nullptr) {
                    auto languagesData = std::make_unique<LanguagesData>(response->getLanguage());
                    switchWindow(gui::window::name::languages, std::move(languagesData));
                }
            }
            return sys::MessageNone{};
        });

        createUserInterface();

        setActiveWindow(gui::name::window::main_window);

        settings->registerValueChange(settings::operators_on,
                                      [this](const std::string &value) { operatorOnChanged(value); });
        settings->registerValueChange(
            ::settings::Cellular::volte_on,
            [this](const std::string &value) { volteChanged(value); },
            ::settings::SettingsScope::Global);
        settings->registerValueChange(
            ::settings::SystemProperties::lockPassHash,
            [this](std::string value) { lockPassHash = utils::getNumericValue<unsigned int>(value); },
            ::settings::SettingsScope::Global);

        return ret;
    }

    void ApplicationSettingsNew::createUserInterface()
    {
        windowsFactory.attach(gui::name::window::main_window, [](Application *app, const std::string &name) {
            return std::make_unique<gui::OptionWindow>(
                app, utils::localize.get("app_settings_title_main"), mainWindowOptionsNew(app));
        });
        windowsFactory.attach(gui::window::name::bluetooth, [](Application *app, const std::string &name) {
            return std::make_unique<gui::BluetoothWindow>(app);
        });
        windowsFactory.attach(gui::window::name::add_device, [](Application *app, const std::string &name) {
            return std::make_unique<gui::AddDeviceWindow>(app);
        });
        windowsFactory.attach(gui::window::name::all_devices, [](Application *app, const std::string &name) {
            return std::make_unique<gui::AllDevicesWindow>(app);
        });
        windowsFactory.attach(gui::window::name::dialog_settings, [](Application *app, const std::string &name) {
            return std::make_unique<gui::Dialog>(app, name);
        });
        windowsFactory.attach(gui::window::name::display_and_keypad, [](Application *app, const std::string &name) {
            return std::make_unique<gui::DisplayAndKeypadWindow>(app);
        });
        windowsFactory.attach(gui::window::name::input_language, [](Application *app, const std::string &name) {
            return std::make_unique<gui::InputLanguageWindow>(app);
        });
        windowsFactory.attach(gui::window::name::locked_screen, [](Application *app, const std::string &name) {
            return std::make_unique<gui::LockedScreenWindow>(app);
        });
        windowsFactory.attach(gui::window::name::keypad_light, [](Application *app, const std::string &name) {
            return std::make_unique<gui::KeypadLightWindow>(app);
        });
        windowsFactory.attach(gui::window::name::font_size, [](Application *app, const std::string &name) {
            return std::make_unique<gui::FontSizeWindow>(app);
        });
        windowsFactory.attach(gui::window::name::display_light, [](Application *app, const std::string &name) {
            return std::make_unique<gui::DisplayLightWindow>(app, static_cast<ApplicationSettingsNew *>(app));
        });
        windowsFactory.attach(gui::window::name::apps_and_tools, [](Application *app, const std::string &name) {
            return std::make_unique<gui::AppsAndToolsWindow>(app);
        });
        windowsFactory.attach(gui::window::name::nightshift, [](Application *app, const std::string &name) {
            return std::make_unique<gui::NightshiftWindow>(app);
        });
        windowsFactory.attach(gui::window::name::network, [](Application *app, const std::string &name) {
            return std::make_unique<gui::NetworkWindow>(
                app, static_cast<ApplicationSettingsNew *>(app), static_cast<ApplicationSettingsNew *>(app));
        });
        windowsFactory.attach(gui::window::name::apn_settings, [](Application *app, const std::string &name) {
            return std::make_unique<gui::ApnSettingsWindow>(app);
        });
        windowsFactory.attach(gui::window::name::apn_options, [](Application *app, const std::string &name) {
            return std::make_unique<gui::ApnOptionsWindow>(app);
        });
        windowsFactory.attach(gui::window::name::messages, [](Application *app, const std::string &name) {
            return std::make_unique<gui::MessagesWindow>(app);
        });
        windowsFactory.attach(gui::window::name::phone_name, [](Application *app, const std::string &name) {
            return std::make_unique<gui::PhoneNameWindow>(app);
        });
        windowsFactory.attach(gui::window::name::autolock, [](Application *app, const std::string &name) {
            return std::make_unique<gui::AutolockWindow>(app);
        });
        windowsFactory.attach(gui::window::name::torch, [](Application *app, const std::string &name) {
            return std::make_unique<gui::TorchWindow>(app);
        });
        windowsFactory.attach(gui::window::name::wallpaper, [](Application *app, const std::string &name) {
            return std::make_unique<gui::WallpaperWindow>(app);
        });
        windowsFactory.attach(gui::window::name::quotes_dialog_yes_no, [](Application *app, const std::string &name) {
            return std::make_unique<gui::DialogYesNo>(app, name);
        });
        windowsFactory.attach(gui::window::name::security, [](Application *app, const std::string &name) {
            return std::make_unique<gui::SecurityMainWindow>(app);
        });
        windowsFactory.attach(gui::window::name::change_passcode, [](Application *app, const std::string &name) {
            return std::make_unique<gui::ChangePasscodeWindow>(app);
        });
        windowsFactory.attach(gui::window::name::dialog_confirm, [](Application *app, const std::string &name) {
            return std::make_unique<gui::DialogConfirm>(app, gui::window::name::dialog_confirm);
        });
        windowsFactory.attach(gui::window::name::system, [](Application *app, const std::string &name) {
            return std::make_unique<gui::SystemMainWindow>(app);
        });
        windowsFactory.attach(gui::window::name::new_apn, [](Application *app, const std::string &name) {
            return std::make_unique<gui::NewApnWindow>(app);
        });
        windowsFactory.attach(gui::window::name::languages, [](Application *app, const std::string &name) {
            return std::make_unique<gui::LanguagesWindow>(app);
        });
    }

    void ApplicationSettingsNew::destroyUserInterface()
    {}

    std::string ApplicationSettingsNew::getNumber()
    {
        return selectedSimNumber;
    }

    void ApplicationSettingsNew::setSim(Store::GSM::SIM sim)
    {
        CellularServiceAPI::SetSimCard(this, sim);
    }

    Store::GSM::SIM ApplicationSettingsNew::getSim()
    {
        return selectedSim;
    }

    void ApplicationSettingsNew::operatorOnChanged(const std::string &value)
    {
        LOG_DEBUG("[ApplicationSettingsNew::operatorOnChanged] value=%s", value.c_str());
        if (!value.empty()) {
            operatorsOn = utils::getNumericValue<bool>(value);
        }
    }
    bool ApplicationSettingsNew::getOperatorsOn() const noexcept
    {
        LOG_DEBUG("[ApplicationSettingsNew::getOperatorsOn] %d", operatorsOn);
        return operatorsOn;
    }
    void ApplicationSettingsNew::setOperatorsOn(bool value)
    {
        operatorsOn = value;
        LOG_DEBUG("[ApplicationSettingsNew::setOperatorsOn] to %d", operatorsOn);
        settings->setValue(settings::operators_on, std::to_string(value));
    }

    void ApplicationSettingsNew::setVoLTEOn(bool value)
    {
        voLteStateOn = value;
        CellularServiceAPI::SetVoLTE(this, voLteStateOn);
    };

    bool ApplicationSettingsNew::getVoLTEOn() const noexcept
    {
        return voLteStateOn;
    }

    void ApplicationSettingsNew::volteChanged(const std::string &value)
    {
        if (!value.empty()) {
            voLteStateOn = utils::getNumericValue<bool>(value);
        }
    }

    void ApplicationSettingsNew::setLockPassHash(unsigned int value)
    {
        lockPassHash = value;
        settings->setValue(
            ::settings::SystemProperties::lockPassHash, std::to_string(value), ::settings::SettingsScope::Global);
    }

    auto ApplicationSettingsNew::getCurrentValues() -> settingsInterface::ScreenLightSettings::Values
    {
        constexpr int timeout = pdMS_TO_TICKS(1500);

        auto response = sys::Bus::SendUnicast(
            std::make_shared<sevm::ScreenLightControlRequestParameters>(), service::name::evt_manager, this, timeout);

        if (response.first == sys::ReturnCodes::Success) {
            auto msgState = dynamic_cast<sevm::ScreenLightControlParametersResponse *>(response.second.get());
            if (msgState == nullptr) {
                return {};
            }

            return {msgState->lightOn, msgState->mode, msgState->parameters};
        }

        return {};
    }

    void ApplicationSettingsNew::setBrightness(bsp::eink_frontlight::BrightnessPercentage value)
    {
        screen_light_control::Parameters parameters{value};
        sys::Bus::SendUnicast(std::make_shared<sevm::ScreenLightControlMessage>(
                                  screen_light_control::Action::setManualModeBrightness, parameters),
                              service::name::evt_manager,
                              this);
    }

    void ApplicationSettingsNew::setMode(bool isAutoLightSwitchOn)
    {
        sys::Bus::SendUnicast(std::make_shared<sevm::ScreenLightControlMessage>(
                                  isAutoLightSwitchOn ? screen_light_control::Action::enableAutomaticMode
                                                      : screen_light_control::Action::disableAutomaticMode),
                              service::name::evt_manager,
                              this);
    }

    void ApplicationSettingsNew::setStatus(bool isDisplayLightSwitchOn)
    {
        sys::Bus::SendUnicast(
            std::make_shared<sevm::ScreenLightControlMessage>(
                isDisplayLightSwitchOn ? screen_light_control::Action::turnOn : screen_light_control::Action::turnOff),
            service::name::evt_manager,
            this);
    }

} /* namespace app */
