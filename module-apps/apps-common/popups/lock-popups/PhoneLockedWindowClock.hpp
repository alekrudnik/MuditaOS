// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "PhoneLockedWindowBase.hpp"
#include <widgets/ClockDateWidget.hpp>
#include <notifications/NotificationsModel.hpp>
#include <ListView.hpp>

namespace gui
{
    class PhoneLockedWindowClock : public PhoneLockedWindowBase
    {
      private:
        gui::ClockDateWidget *clockDate                             = nullptr;
        gui::ListView *notificationsList                            = nullptr;
        std::shared_ptr<gui::NotificationsModel> notificationsModel = nullptr;
        bool refreshedOnPhoneLockTimeLock                           = false;

      public:
        PhoneLockedWindowClock(app::ApplicationCommon *app, const std::string &name);
        void buildInterface() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        bool updateTime() override;
    };

} /* namespace gui */
