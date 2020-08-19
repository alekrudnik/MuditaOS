#pragma once

#include "windows/AppWindow.hpp"
#include "Application.hpp"
#include "application-calendar/ApplicationCalendar.hpp"
#include "application-calendar/models/MonthModel.hpp"
#include <gui/widgets/GridLayout.hpp>
#include <map>
#include <vector>
#include <string>

namespace gui
{
    class CalendarMainWindow;

    class DayLabel : public VBox
    {
        gui::Label *dayNumber = nullptr;
        gui::Label *dot       = nullptr;

      public:
        DayLabel(app::Application *app,
                 gui::Item *parent,
                 const uint32_t &cellIndex,
                 const uint32_t &firstWeekOffset,
                 const uint32_t &width,
                 const uint32_t &height,
                 bool isDayEmpty);
        ~DayLabel() override = default;
    };

    class MonthBox : public GridLayout
    {
      public:
        MonthBox(app::Application *app,
                 gui::Item *parent,
                 const int &offsetTop,
                 const uint32_t &width,
                 const uint32_t &height,
                 const uint32_t &dayWidth,
                 const uint32_t &dayHeight,
                 const std::unique_ptr<MonthModel> &model,
                 bool *isDayEmpty);

        ~MonthBox() override = default;
        std::string month;
        uint32_t monthFilterValue;
    };

    class CalendarMainWindow : public gui::AppWindow
    {
        bool isDayEmpty[31];
        uint32_t offsetFromTop = 0;
        uint32_t monthWidth    = 0;
        uint32_t monthHeight   = 0;
        uint32_t dayWidth      = 0;
        uint32_t dayHeight     = 0;

      protected:
        date::year_month_day actualDate;
        MonthBox *month = nullptr;
        Label *dateLabel = nullptr;
        std::unique_ptr<MonthModel> monthModel;

      public:
        CalendarMainWindow(app::Application *app, std::string name);

        ~CalendarMainWindow() override = default;
        void rebuild() override;
        void refresh();
        void filterRequest();
        void buildMonth(std::unique_ptr<MonthModel> &model);
        void buildDateLabel(std::string actualDateTime);
        void buildInterface() override;
        void destroyInterface() override;
        bool onInput(const gui::InputEvent &inputEvent) override;
        bool onDatabaseMessage(sys::Message *msgl) override;
        std::unique_ptr<MonthModel> getMonthModel()
        {
            return std::move(monthModel);
        }
    };

} // namespace gui
