// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "BaseSettingsWindow.hpp"

namespace gui
{
    ///  @brief Notification sound selection window
    class NotificationSoundWindow : public BaseSettingsWindow
    {
      public:
        ///  Constructor
        ///  @param app pointer to application
        explicit NotificationSoundWindow(app::Application *app);

        ///  Build the list of options to be drawn
        ///  @return list of options
        std::list<Option> buildOptionsList() override
        {
            std::list<Option> lst;
            return lst;
        }
    };
} // namespace gui
