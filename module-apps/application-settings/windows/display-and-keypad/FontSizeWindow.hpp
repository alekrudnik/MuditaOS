#pragma once

#include <functional>
#include <string>

#include "../BaseSettingsWindow.hpp"
#include "gui/widgets/Image.hpp"
#include "gui/widgets/Label.hpp"
#include "gui/widgets/Window.hpp"
#include <memory>
#include <BoxLayout.hpp>

namespace gui
{

    class FontSizeWindow : public SettingsWindow
    {
      public:
        FontSizeWindow(app::Application *app);
        void buildInterface() override;

        static constexpr const char *window_name = "FontSizeWindow";
    };
} // namespace gui
