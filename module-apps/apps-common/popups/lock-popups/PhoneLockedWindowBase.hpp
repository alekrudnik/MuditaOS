// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <AppWindow.hpp>

namespace app
{
    class ApplicationDesktop;
}

namespace gui
{
    class PhoneLockedWindowBase : public AppWindow
    {
      private:
        bool processLongReleaseEvent(const InputEvent &inputEvent);

      public:
        PhoneLockedWindowBase(app::ApplicationCommon *app, const std::string &name);
        bool onInput(const InputEvent &inputEvent) override;
        status_bar::Configuration configureStatusBar(status_bar::Configuration appConfiguration) override;
    };

} /* namespace gui */
