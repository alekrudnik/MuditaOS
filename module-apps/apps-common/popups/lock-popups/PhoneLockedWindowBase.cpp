// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhoneLockedWindowBase.hpp"
#include "PhoneLockedInfoData.hpp"

#include <service-appmgr/Controller.hpp>
#include <locks/input/PhoneLockedKeysWhitelist.hpp>

namespace gui
{
    PhoneLockedWindowBase::PhoneLockedWindowBase(app::ApplicationCommon *app, const std::string &name)
        : AppWindow(app, name)
    {
        AppWindow::buildInterface();
    }

    status_bar::Configuration PhoneLockedWindowBase::configureStatusBar(status_bar::Configuration appConfiguration)
    {
        appConfiguration.disable(status_bar::Indicator::NetworkAccessTechnology);
        appConfiguration.disable(status_bar::Indicator::Time);
        appConfiguration.enable(status_bar::Indicator::PhoneMode);
        appConfiguration.enable(status_bar::Indicator::Lock);
        appConfiguration.enable(status_bar::Indicator::Battery);
        appConfiguration.enable(status_bar::Indicator::Signal);
        appConfiguration.enable(status_bar::Indicator::SimCard);
        appConfiguration.enable(status_bar::Indicator::Bluetooth);
        appConfiguration.enable(status_bar::Indicator::AlarmClock);

        return appConfiguration;
    }

    bool PhoneLockedWindowBase::processLongReleaseEvent(const InputEvent &inputEvent)
    {
        if (inputEvent.is(KeyCode::KEY_RF)) {

            application->switchWindow(gui::popup::window::power_off_window);
        }
        return true;
    }

    bool PhoneLockedWindowBase::onInput(const InputEvent &inputEvent)
    {
        // check if any of the lower inheritance onInput methods catch the event
        if (locks::PhoneLockedKeysWhitelist::isOnWhitelist(inputEvent)) {
            return AppWindow::onInput(inputEvent);
        }

        if (inputEvent.isLongRelease()) {
            return processLongReleaseEvent(inputEvent);
        }
        else if (inputEvent.isShortRelease() && navBar->isActive(nav_bar::Side::Center)) {
            const auto requiredStage = (inputEvent.is(KeyCode::KEY_ENTER)) ? PhoneLockedInfoData::Stage::Waiting
                                                                           : PhoneLockedInfoData::Stage::Idle;
            application->switchWindow(gui::popup::window::phone_lock_info_window,
                                      std::make_unique<PhoneLockedInfoData>(requiredStage));
            return true;
        }
        else if (inputEvent.isShortRelease(KeyCode::KEY_LF) && navBar->isActive(nav_bar::Side::Left)) {
            app::manager::Controller::sendAction(application,
                                                 app::manager::actions::EmergencyDial,
                                                 std::make_unique<SwitchData>(),
                                                 app::manager::OnSwitchBehaviour::RunInBackground);
        }
        return true;
    }

} /* namespace gui */
