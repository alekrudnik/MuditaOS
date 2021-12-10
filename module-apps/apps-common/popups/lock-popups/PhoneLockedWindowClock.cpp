// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhoneLockedWindowClock.hpp"
#include <service-appmgr/Controller.hpp>

namespace gui
{
    PhoneLockedWindowClock::PhoneLockedWindowClock(app::ApplicationCommon *app, const std::string &name)
        : PhoneLockedWindowBase(app, name),
          notificationsModel(std::make_shared<NotificationsModel>(NotificationsListPlacement::LockedScreen))
    {
        buildInterface();

        preBuildDrawListHook = [this](std::list<Command> &cmd) { updateTime(); };
    }

    void PhoneLockedWindowClock::buildInterface()
    {
        clockDate = new ClockDateWidget(
            this, 0, style::window::default_vertical_pos, style::window_width, clock_date_widget::h);

        notificationsList = new ListView(this,
                                         style::notifications::model::x,
                                         style::notifications::model::y,
                                         style::notifications::model::w,
                                         style::notifications::model::h,
                                         notificationsModel,
                                         listview::ScrollBarType::None);
    }

    void PhoneLockedWindowClock::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        auto notificationData = dynamic_cast<app::manager::actions::NotificationsChangedParams *>(data);
        if (notificationData) {
            notificationsModel->updateData(notificationData);
        }
        else if (!notificationsModel->isPhoneTimeLock()) {
            app::manager::Controller::requestNotifications(application);
            navBar->setActive(nav_bar::Side::Left, false);
            navBar->setActive(nav_bar::Side::Center, false);
            navBar->setActive(nav_bar::Side::Right, false);
            return;
        }

        if (notificationsModel->isPhoneTimeLock()) {
            if (!refreshedOnPhoneLockTimeLock) {
                application->refreshWindow(RefreshModes::GUI_REFRESH_DEEP);
                refreshedOnPhoneLockTimeLock = true;
            }

            navBar->setText(nav_bar::Side::Left, utils::translate("app_desktop_emergency"));
            navBar->setActive(nav_bar::Side::Center, false);
            navBar->setActive(nav_bar::Side::Right, false);
        }
        else {
            navBar->setActive(nav_bar::Side::Left, false);
            navBar->setText(nav_bar::Side::Center, utils::translate("app_desktop_unlock"));
            navBar->setActive(nav_bar::Side::Right, false);
        }
    }

    bool PhoneLockedWindowClock::updateTime()
    {
        auto ret = AppWindow::updateTime();
        clockDate->setTime(std::time(nullptr));
        return ret;
    }
} /* namespace gui */
