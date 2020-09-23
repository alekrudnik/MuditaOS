#include "LockedScreenWindow.hpp"

#include <functional>
#include <memory>

#include "service-appmgr/ApplicationManager.hpp"
#include "i18/i18.hpp"
#include "Label.hpp"

namespace gui
{

    LockedScreenWindow::LockedScreenWindow(app::Application *app) : SettingsWindow(app, window_name)
    {
        buildInterface();
    }

    void LockedScreenWindow::buildInterface()
    {
        SettingsWindow::buildInterface();
        setTitle(utils::localize.get("app_settings_display_locked_screen"));
    }
} // namespace gui
