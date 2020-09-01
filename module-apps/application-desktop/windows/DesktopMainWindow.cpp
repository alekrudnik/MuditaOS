/*
 * @file DesktopMainWindow.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 19 cze 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include <memory>
#include <functional>

#include "../ApplicationDesktop.hpp"
#include "../data/LockPhoneData.hpp"
#include "Alignment.hpp"
#include "Common.hpp"
#include "BottomBar.hpp"
#include "DesktopMainWindow.hpp"
#include "application-messages/ApplicationMessages.hpp"
#include "gui/widgets/Image.hpp"
#include "service-appmgr/ApplicationManager.hpp"

#include "application-call/ApplicationCall.hpp"
#include "application-call/data/CallSwitchData.hpp"
#include <UiCommonActions.hpp>

#include <gui/tools/Common.hpp>
#include "i18/i18.hpp"
#include "log/log.hpp"
#include <Span.hpp>
#include <Style.hpp>
#include <application-settings/ApplicationSettings.hpp>
#include <cassert>
#include <service-appmgr/ApplicationManager.hpp>
#include <time/time_conversion.hpp>

#include <module-utils/date/include/date/dateCommon.h>

namespace style
{
    const auto design_time_offset          = 106;
    const auto design_time_h               = 96;
    const auto design_day_offset           = 204;
    const auto design_day_h                = 51;
    const auto design_border_offset        = 20;
    const auto design_option_span          = 8;
    const auto design_notifications_offset = 284;
}; // namespace style

namespace gui
{

    void DesktopMainWindow::buildInterface()
    {

        auto ttime = utils::time::Time();
        AppWindow::buildInterface();

        bottomBar->setActive(BottomBar::Side::CENTER, true);
        topBar->setActive(
            {{TopBar::Elements::SIGNAL, true}, {TopBar::Elements::LOCK, true}, {TopBar::Elements::BATTERY, true}});

        time = new gui::Label(this, 0, style::design_time_offset, style::window_width, style::design_time_h);
        time->setFilled(false);
        time->setBorderColor(gui::ColorNoColor);
        time->setFont(style::window::font::supersizemelight);
        time->setText(ttime);
        time->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Top));

        dayText = new gui::Label(this, 0, style::design_day_offset, style::window_width, style::design_day_h);
        dayText->setFilled(false);
        dayText->setBorderColor(gui::ColorNoColor);
        dayText->setFont(style::window::font::biglight);
        dayText->setText(ttime.day() + ", " + ttime.str("%d %b"));
        dayText->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Top));

        setVisibleState();
    }

    void DesktopMainWindow::destroyInterface()
    {
        erase();
    }

    DesktopMainWindow::DesktopMainWindow(app::Application *app) : AppWindow(app, app::window::name::desktop_main_window)
    {
        buildInterface();
    }

    void DesktopMainWindow::setVisibleState()
    {
        auto app = dynamic_cast<app::ApplicationDesktop *>(application);
        assert(app != nullptr);

        if (app->getScreenLocked()) {
            bottomBar->restore();
            bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("app_desktop_unlock"));
            topBar->setActive(TopBar::Elements::LOCK, true);
            inputCallback = nullptr;
            setFocusItem(nullptr);
            erase(notifications);
        }
        else {
            auto app = dynamic_cast<app::ApplicationDesktop *>(application);
            assert(app);
            bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("app_desktop_menu"));
            topBar->setActive(TopBar::Elements::LOCK, false);
            if (!fillNotifications(app)) {
                LOG_ERROR("Couldn't fit in all notifications");
            }
            if (app->need_sim_select && Store::GSM::get()->sim == Store::GSM::SIM::SIM_UNKNOWN) {
                sapm::ApplicationManager::messageSwitchApplication(
                    this->application, app::name_settings, app::sim_select, nullptr);
            }
        }
    }

    void time_Tests()
    {

        TimePoint date_from = TIME_POINT_INVALID;
        date_from           = std::chrono::system_clock::now();

        // std::time_t tt = system_clock::to_time_t(date_from);
        // std::chrono::system_clock::time_point tp =
        // std::chrono::system_clock::from_time_t(appCalendar->getCurrentTimeStamp());

        // konwersja na string
        string s1 = date::format("%F %T", time_point_cast<seconds>(system_clock::now()));
        // string s2 =  date::format("%F %T\n", time_point_cast<seconds>(system_clock::now()));
        string s2 = date::format("%F %T", time_point_cast<seconds>(date_from));

        /*TimePoint date_to = date::parse("%F %T", s2);

        std::stringstream str( "28.08.2017 03:59:55.0007" );
        str.imbue( std::locale() );
        std::chrono::time_point< std::chrono::system_clock, std::chrono::microseconds > result;
        date::from_stream( str, "%d.%m.%Y %H:%M:%S", result );
        std::cout << result.time_since_epoch().count();*/

        /*TimePoint date_to;
        date::from_stream( s1, "%d-%m-%Y %H:%M:%S", date_to );
        date::to_stream(s1, "%d-%m-%Y %H:%M:%S", date_to)
        date::parse( s1, "%d-%m-%Y %H:%M:%S", date_to );*/
        // string f = "%d-%m-%Y %H:%M:%S";
        // TimePoint date_to = date::parse(f, date_to, s1);

        TimePoint date_to;
        std::chrono::system_clock::time_point tp;
        std::istringstream ss{"2010-12-30T01:20:30.123456Z"};
        // ss >> date::parse("%FT%TZ", date_to);
        std::istringstream(s1) >> date::parse("%F %T", date_to);

        string s3 = date::format("%F %T", time_point_cast<seconds>(date_to));
        // date::parse("%FT%TZ", date_to, ss);

        TimePoint date1 = std::chrono::system_clock::now();
        string xx1      = TimePointToString(date1);
        TimePoint date2;
        date2      = TimePointFromString(xx1.c_str());
        string xx2 = TimePointToString(date2);

        date2      = TimePointFromString("1990-12-11 23:13:22");
        string xx3 = TimePointToString(date2);

        // time_t tt = system_clock::to_time_t(date1);
        // auto ymd = date::year_month_day{date1};

        // parse("%a, %d %b %Y %T %z", tp);

        // konwersja ze string
        // string g1 = "1991-12-11 23:13:22";
        //_TimePoint n1 = g1.c_str();
        //_TimePoint n1 = std::chrono::system_clock::now();
        //_TimePoint n1 = std::chrono::system_clock::now();
        // TimePoint n1 = std::chrono::system_clock::now();
        [[maybe_unused]] date::year_month_day dt10 = date::year_month_day{
            date::floor<date::days>(std::chrono::system_clock::from_time_t(utils::time::Timestamp().getTime()))};

        [[maybe_unused]] date::year_month_day dt11 = date::year_month_day{
            date::floor<date::days>(std::chrono::system_clock::from_time_t(utils::time::Timestamp().getTime()))};

        [[maybe_unused]] auto dt12 = date::year_month_day{date::floor<date::days>(date2)};

        [[maybe_unused]] auto dt13 = date::year_month_day{date::floor<date::days>(date2)};

        //[[maybe_unused]]
        // date::year_month_day yearMonthDayFirst = 2021 / 11 / 1;

        //[[maybe_unused]]
        // auto dt13= date::hh_mm_ss{date::floor<seconds>(date2)};
        // date::hh_mm_ss<std::chrono::seconds> tod{std::chrono::seconds(secs)};
        // auto dt13 = date::hh_mm_ss;

        //[[maybe_unused]]
        // date::year_month_day dt11 = date::year_month_day{date2};

        if (s1 != s1)
            return;
        if (date_from != date_to)
            return;

        /*
        int main()
        {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t t_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));
            std::cout << "24 hours ago, the time was "
                      << std::put_time(std::localtime(&t_c), "%F %T") << '\n';

            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            std::cout << "Hello World\n";
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Printing took "
                      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
                      << "us.\n";
        }
         */
    }

    void DesktopMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        //***************************************************************************
        time_Tests();
        //***************************************************************************

        // update time
        time->setText(topBar->getTimeString());

        // check if there was a signal to lock the pone due to inactivity.
        if ((data != nullptr) && (data->getDescription() == "LockPhoneData")) {
            auto app = dynamic_cast<app::ApplicationDesktop *>(application);
            if (!app) {
                return;
            }
            app->setScreenLocked(true);

            LockPhoneData *lockData = reinterpret_cast<LockPhoneData *>(data);
            lockTimeoutApplilcation = lockData->getPreviousApplication();

            reinterpret_cast<app::ApplicationDesktop *>(application)->setSuspendFlag(true);
        }

        setVisibleState();
    }

    bool DesktopMainWindow::onInput(const InputEvent &inputEvent)
    {
        app::ApplicationDesktop *app = dynamic_cast<app::ApplicationDesktop *>(application);
        if (app == nullptr) {
            LOG_ERROR("not ApplicationDesktop");
            return AppWindow::onInput(inputEvent);
        }

        auto code = translator.handle(inputEvent.key, InputMode({InputMode::phone}).get());

        // process shortpress
        if (inputEvent.state == InputEvent::State::keyReleasedShort) {

            if (app->getScreenLocked()) {
                // if enter was pressed
                if (inputEvent.keyCode == KeyCode::KEY_ENTER) {
                    unlockStartTime = xTaskGetTickCount();
                    enterPressed    = true;
                }
                else if ((inputEvent.keyCode == KeyCode::KEY_PND) && enterPressed) {
                    // if interval between enter and pnd keys is less than time defined for unlocking
                    if (xTaskGetTickCount() - unlockStartTime < unclockTime) {
                        // display pin lock screen or simply refresh current window to update labels
                        if (app->getPinLocked())
                            // if there was no application on to before closing proceed normally to pin protection
                            // window.
                            if (lockTimeoutApplilcation.empty()) {
                                application->switchWindow("PinLockWindow");
                            }
                            else {
                                std::unique_ptr<LockPhoneData> data = std::make_unique<LockPhoneData>();
                                data->setPrevApplication(lockTimeoutApplilcation);
                                lockTimeoutApplilcation = "";
                                application->switchWindow("PinLockWindow", std::move(data));
                            }

                        else {

                            // if phone was locked by user show unlocked main window
                            if (lockTimeoutApplilcation.empty()) {
                                app->setScreenLocked(false);
                                setVisibleState();
                                application->refreshWindow(RefreshModes::GUI_REFRESH_FAST);
                            }
                            // if there was application on top when timeout occurred
                            else {
                                lockTimeoutApplilcation = "";
                                sapm::ApplicationManager::messageSwitchPreviousApplication(application);
                            }
                        }
                    }
                    enterPressed = false;
                }
                else {
                    enterPressed = false;
                }
            }
            // screen is unlocked
            else {
                // pressing enter moves user to menu screen
                if (inputEvent.keyCode == KeyCode::KEY_ENTER) {
                    application->switchWindow("MenuWindow");
                }
                // if numeric key was pressed record that key and send it to call application
                else if (code != 0) {
                    return app::prepareCall(application, std::string(1, static_cast<char>(code)));
                }
            }
        }
        else if (inputEvent.state == InputEvent::State::keyReleasedLong) {
            // long press of # locks screen if it was unlocked
            if ((inputEvent.keyCode == KeyCode::KEY_PND) && (app->getScreenLocked() == false)) {
                app->setScreenLocked(true);
                setVisibleState();
                app->refreshWindow(RefreshModes::GUI_REFRESH_FAST);
                app->setSuspendFlag(true);
                return true;
            }
            // long press of right function button move user to power off window
            else if (inputEvent.keyCode == KeyCode::KEY_RF) {
                application->switchWindow("PowerOffWindow");
                return true;
            }
            // long press of '0' key is translated to '+'
            else if (inputEvent.keyCode == KeyCode::KEY_0) {
                return app::prepareCall(application, "+");
            }
        }

        // check if any of the lower inheritance onInput methods catch the event
        return AppWindow::onInput(inputEvent);
    }

    void DesktopMainWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    bool DesktopMainWindow::updateTime(const UTF8 &timeStr)
    {
        auto ret = AppWindow::updateTime(timeStr);
        time->setText(topBar->getTimeString());
        return ret;
    }
    bool DesktopMainWindow::updateTime(const uint32_t &timestamp, bool mode24H)
    {
        auto ret = AppWindow::updateTime(timestamp, mode24H);
        time->setText(topBar->getTimeString());
        return ret;
    }

    std::list<DrawCommand *> DesktopMainWindow::buildDrawList()
    {
        time->setText(topBar->getTimeString());
        return gui::AppWindow::buildDrawList();
    }

    auto add_box_icon(gui::BoxLayout *layout, UTF8 icon)
    {
        auto thumbnail        = new gui::Image(icon);
        thumbnail->activeItem = false;
        layout->addWidget(thumbnail);
    }

    /// for now notifications are like that: `^<span>[icon]<span>[dumb text]       [dot image] [number of
    /// notifications]<span>$`
    auto add_notification(BoxLayout *layout,
                          UTF8 icon,
                          UTF8 name,
                          UTF8 indicator,
                          std::function<bool()> showCallback,
                          std::function<bool()> clearCallback) -> bool
    {
        const auto text_normal_size        = 200;
        const auto size_needed_for_2digits = 30;
        // 1. create hbox for all elements
        auto el = new gui::HBox(nullptr, 0, 0, style::window::default_body_width, style::window::label::default_h);
        el->setAlignment(Alignment(gui::Alignment::Vertical::Center));
        auto text = new gui::Label(nullptr, 0, 0, text_normal_size, style::window::label::default_h, "");
        text->setMaximumSize(el->area().w, Axis::X);
        text->setText(name);
        text->setFont(style::window::font::medium);
        text->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        text->setPenWidth(style::window::default_border_no_focus_w);
        text->activeItem = false;

        auto number = new gui::Label();
        number->setText(indicator);
        number->setFont(style::window::font::mediumbold);
        number->setPenWidth(style::window::default_border_no_focus_w);
        number->setSize(size_needed_for_2digits, el->area().h);
        number->setMinimumWidth(size_needed_for_2digits);
        number->setAlignment(Alignment(gui::Alignment::Horizontal::Right, gui::Alignment::Vertical::Center));
        number->activeItem = false;
        // 2. Add all elements to hbox layout
        new gui::Span(el, Axis::X, style::design_border_offset);
        add_box_icon(el, icon);
        new gui::Span(el, Axis::X, style::design_border_offset);
        el->addWidget(text);
        add_box_icon(el, "dot_12px_hard_alpha_W_G");
        el->addWidget(number);
        // box right inner margin
        new gui::Span(el, Axis::X, style::design_border_offset);
        // 3. Set hbox layout properties
        el->setPenWidth(style::window::default_border_no_focus_w);
        el->setPenFocusWidth(style::window::default_border_focus_w);
        el->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM | RectangleEdgeFlags::GUI_RECT_EDGE_TOP);
        el->inputCallback = [showCallback, clearCallback](Item &, const InputEvent &event) -> bool {
            if (event.state != InputEvent::State::keyReleasedShort) {
                return false;
            }
            if (event.keyCode == KeyCode::KEY_LF && showCallback) {
                return showCallback();
            }
            if (event.keyCode == KeyCode::KEY_RF && clearCallback) {
                return clearCallback();
            }
            return false;
        };

        layout->addWidget(el);
        if (el->visible) {
            // space between next notifications to show
            layout->addWidget(new gui::Span(Axis::Y, style::design_option_span));
        }

        return el->visible;
    }

    auto DesktopMainWindow::fillNotifications(app::ApplicationDesktop *app) -> bool
    {
        bottomBar->restore();
        erase(notifications);
        // 1. create notifications box
        notifications = new gui::VBox(nullptr,
                                      0,
                                      style::design_notifications_offset,
                                      style::window_width,
                                      bottomBar->widgetArea.pos(Axis::Y) - style::design_notifications_offset);
        notifications->setAlignment(Alignment(gui::Alignment::Horizontal::Center));
        notifications->setPenWidth(style::window::default_border_no_focus_w);
        notifications->setPenFocusWidth(style::window::default_border_no_focus_w);
        this->addWidget(notifications);
        if (!notifications->visible) {
            LOG_ERROR("Can't fit notifications box!");
            return false;
        }

        // 2. actually fill it in
        if (app->notifications.notSeen.Calls > 0) {
            add_notification(
                notifications,
                "phone",
                utils::localize.get("app_desktop_missed_calls"),
                std::to_string(app->notifications.notSeen.Calls),
                [app]() -> bool { return app->showCalls(); },
                [app]() -> bool { return app->clearCallsNotification(); });
        }
        if (app->notifications.notSeen.SMS > 0) {
            add_notification(
                notifications,
                "mail",
                utils::localize.get("app_desktop_unread_messages"),
                std::to_string(app->notifications.notSeen.SMS),
                [this]() -> bool {
                    return sapm::ApplicationManager::messageSwitchApplication(
                        application, app::name_messages, gui::name::window::main_window, nullptr);
                },
                [app]() -> bool { return app->clearMessagesNotification(); });
        }
        bottomBar->store();
        if (app->notifications.notSeen.areEmpty() != true) {
            setFocusItem(notifications);
            bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get("app_desktop_show"));
            bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get("app_desktop_clear"));
        }
        else {
            bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get("app_desktop_calls"));
            inputCallback = [app](Item &, const InputEvent &inputEvent) -> bool {
                if (inputEvent.state == InputEvent::State::keyReleasedShort &&
                    inputEvent.keyCode == gui::KeyCode::KEY_LF) {
                    return app->showCalls();
                }
                return false;
            };
        }
        return true;
    }

} /* namespace gui */
