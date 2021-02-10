// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "module-apps/Application.hpp"
#include "AppWindow.hpp"
#include "module-apps/widgets/BarGraph.hpp"
#include <functional>

namespace style::window::volume
{
    constexpr inline auto title_height = 33;
    constexpr inline auto title_key          = "app_popup_volume_text";
    constexpr inline auto muted_title_key    = "app_popup_muted_text";

    namespace bar
    {
        constexpr inline auto volume_levels = 10;
        constexpr inline auto top_offset    = 179;
    } // namespace bar

} // namespace style::window::volume
namespace gui
{
    class VolumeWindow : public AppWindow
    {

    protected:

        std::unique_ptr<sys::Timer> volumeWindowTimer;
        auto isMuted() -> bool;

        Label *volumeText    = nullptr;
        VBarGraph *volumeBar = nullptr;

        void windowTimerCallback();
        void startTimer();
        void destroyTimer();

      public:

        VolumeWindow(app::Application *app, const std::string &name);

        ~VolumeWindow() override;

        void addVolumeText();
        void addVolumeBar();

        void onClose() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        void rebuild() override;
        void buildInterface() override;
        void destroyInterface() override;
        auto onInput(const gui::InputEvent &inputEvent) -> bool override;
        auto closeWindow() -> bool;
    };
}; // namespace gui
