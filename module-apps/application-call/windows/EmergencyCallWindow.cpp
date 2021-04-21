﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "EmergencyCallWindow.hpp"

#include "application-call/data/CallAppStyle.hpp"

#include <service-appmgr/Controller.hpp>

namespace gui
{

    EmergencyCallWindow::EmergencyCallWindow(app::Application *app, app::EnterNumberWindowInterface *interface)
        : NumberWindow(app, interface, app::window::name_emergencyCall)
    {
        buildInterface();
    }

    void EmergencyCallWindow::buildInterface()
    {
        using namespace callAppStyle;
        NumberWindow::buildInterface();

        bottomBar->setText(BottomBar::Side::CENTER, utils::translate("app_phonebook_ice_contacts_title"));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::translate(style::strings::common::back));

        numberDescriptionLabel->setText(utils::translate("app_call_emergency_text"));
    }

    top_bar::Configuration EmergencyCallWindow::configureTopBar(top_bar::Configuration appConfiguration)
    {
        appConfiguration.disable(top_bar::Indicator::NetworkAccessTechnology);
        appConfiguration.enable(top_bar::Indicator::PhoneMode);
        return appConfiguration;
    }

    bool EmergencyCallWindow::onInput(const InputEvent &inputEvent)
    {
        if (inputEvent.isShortPress()) {
            // Call function
            if (inputEvent.is(KeyCode::KEY_LF)) {
                interface->handleEmergencyCallEvent(enteredNumber);
                return true;
            }
            else if (inputEvent.is(gui::KeyCode::KEY_ENTER)) {
                auto data                        = std::make_unique<gui::SwitchData>();
                data->ignoreCurrentWindowOnStack = true;
                app::manager::Controller::sendAction(
                    application, app::manager::actions::ShowEmergencyContacts, std::move(data));
                return true;
            }
        }

        return NumberWindow::onInput(inputEvent);
    }

} /* namespace gui */
