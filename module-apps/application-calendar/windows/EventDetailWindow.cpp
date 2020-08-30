#include "EventDetailWindow.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include <gui/widgets/Window.hpp>
#include <time/time_conversion.hpp>
#include <module-apps/application-calendar/data/CalendarData.hpp>

namespace gui
{

    EventDetailWindow::EventDetailWindow(app::Application *app, std::string name)
        : AppWindow(app, style::window::calendar::name::details_window), eventDetailModel{
                                                                             std::make_shared<EventDetailModel>(app)}
    {
        buildInterface();
    }

    void EventDetailWindow::rebuild()
    {
        erase();
        buildInterface();
    }

    void EventDetailWindow::buildInterface()
    {
        AppWindow::buildInterface();

        topBar->setActive(gui::TopBar::Elements::TIME, true);
        bottomBar->setActive(gui::BottomBar::Side::RIGHT, true);
        bottomBar->setActive(gui::BottomBar::Side::LEFT, true);
        bottomBar->setText(gui::BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
        bottomBar->setText(gui::BottomBar::Side::LEFT, utils::localize.get(style::strings::common::options));

        bodyList = new gui::ListView(this,
                                     style::window::calendar::listView_x,
                                     style::window::calendar::listView_y,
                                     style::window::calendar::listView_w,
                                     style::window::calendar::listView_h,
                                     eventDetailModel);

        setFocusItem(bodyList);
    }

    void EventDetailWindow::onBeforeShow(gui::ShowMode mode, gui::SwitchData *data)
    {
        if (mode == gui::ShowMode::GUI_SHOW_INIT) {
            bodyList->rebuildList();
        }

        eventDetailModel->loadData(eventRecord);
    }

    auto EventDetailWindow::handleSwitchData(SwitchData *data) -> bool
    {
        if (data == nullptr) {
            return false;
        }

        auto *item = dynamic_cast<EventRecordData *>(data);
        if (item == nullptr) {
            return false;
        }

        eventRecord = item->getData();
        prevWindowName         = item->getWindowName();
        std::chrono::system_clock::time_point start_tp =
            std::chrono::system_clock::from_time_t(utils::time::Timestamp().getTime() + 7200);
        auto startDate       = date::year_month_day{date::floor<date::days>(start_tp)};
        std::string monthStr = utils::time::Locale::get_month(utils::time::Locale::Month(unsigned(startDate.month())));
        setTitle(std::to_string(unsigned(startDate.day())) + " " + monthStr);

        return true;
    }

    bool EventDetailWindow::onInput(const gui::InputEvent &inputEvent)
    {
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_LF) {
            LOG_DEBUG("Switch to option window");
            auto rec  = std::make_unique<EventsRecord>(*eventRecord);
            auto data = std::make_unique<EventRecordData>(std::move(rec));
            data->setWindowName(prevWindowName);
            application->switchWindow(style::window::calendar::name::events_options, std::move(data));
            return true;
        }

        return false;
    }
} /* namespace gui */
