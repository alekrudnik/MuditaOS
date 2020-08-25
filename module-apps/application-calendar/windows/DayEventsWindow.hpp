#pragma once

#include "application-calendar/models/DayEventsInternalModel.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include "windows/AppWindow.hpp"
#include "Application.hpp"
#include "NoEvents.hpp"

#include <ListView.hpp>
#include <gui/widgets/Item.hpp>
#include <gui/widgets/Label.hpp>

namespace gui
{
    class DayEventsWindow : public gui::AppWindow
    {
        std::string dayMonthTitle;
        uint32_t filterFrom;
        uint32_t filterTill;
        gui::Image *leftArrowImage                               = nullptr;
        gui::Image *newDayEventImage                             = nullptr;
        gui::ListView *dayEventsList                             = nullptr;
        std::shared_ptr<DayEventsInternalModel> dayEventsModel   = nullptr;

      public:
        DayEventsWindow(app::Application *app, std::string name);
        bool handleSwitchData(SwitchData *data) override;
        bool onInput(const gui::InputEvent &inputEvent) override;
        bool onDatabaseMessage(sys::Message *msgl) override;
        void rebuild() override;
        void buildInterface() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;

        bool checkEmpty = false;
    };

} /* namespace app */
