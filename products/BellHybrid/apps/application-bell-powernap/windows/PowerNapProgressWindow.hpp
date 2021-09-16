// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "presenter/PowerNapProgressPresenter.hpp"
#include <AppWindow.hpp>

namespace gui
{
    class HBarGraph;
    class Text;
    class PowerNapProgressWindow : public AppWindow, public app::powernap::PowerNapProgressContract::View
    {
        std::shared_ptr<app::powernap::PowerNapProgressContract::Presenter> presenter;
        gui::HBarGraph *progressBar = nullptr;
        gui::Text *timerText        = nullptr;

        void buildInterface() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        auto onInput(const InputEvent &inputEvent) -> bool override;

        void buildLayout();
        void configureTimer();

        void napEnded() override;

      public:
        PowerNapProgressWindow(app::ApplicationCommon *app,
                               std::shared_ptr<app::powernap::PowerNapProgressContract::Presenter> presenter);
    };
} // namespace gui
