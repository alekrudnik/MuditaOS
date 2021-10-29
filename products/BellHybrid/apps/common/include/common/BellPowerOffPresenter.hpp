// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/popups/presenter/PowerOffPresenter.hpp>

namespace gui
{
    class BellPowerOffPresenter : public gui::AbstractPowerOffPresenter
    {
      public:
        explicit BellPowerOffPresenter(app::ApplicationCommon *app);

        void powerOff() override;
        void factoryReset() override;
        void reboot() override;

      private:
        app::ApplicationCommon *application;
    };
} // namespace gui
