// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "BaseSettingsWindow.hpp"

namespace gui
{
    class PINSettingsWindow : public BaseSettingsWindow
    {
      public:
        explicit PINSettingsWindow(app::Application *app);

      private:
        auto buildOptionsList() -> std::list<Option> override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        void changePinState(bool &currentState);

        bool pinIsOn = false;
    };
} // namespace gui