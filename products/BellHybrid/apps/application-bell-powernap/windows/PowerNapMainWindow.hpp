// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

// Copyright (c) 2018-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "presenter/PowerNapMainWindowPresenter.hpp"
#include <AppWindow.hpp>

namespace gui
{
    class SideListView;
    class PowerNapMainWindow : public AppWindow, public app::powernap::PowerNapMainWindowContract::View
    {
        std::unique_ptr<app::powernap::PowerNapMainWindowContract::Presenter> windowPresenter;
        SideListView *sideListView;

        bool onInput(const gui::InputEvent &inputEvent) override;
        void buildInterface() override;
        status_bar::Configuration configureStatusBar(status_bar::Configuration appConfiguration) override;

      public:
        PowerNapMainWindow(app::Application *app,
                           std::unique_ptr<app::powernap::PowerNapMainWindowContract::Presenter> &&windowPresenter);
    };
} // namespace gui
