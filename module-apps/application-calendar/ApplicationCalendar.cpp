#include "ApplicationCalendar.hpp"
#include "windows/CalendarMainWindow.hpp"
#include "windows/DayEventsWindow.hpp"
#include "windows/CalendarEventsOptionsWindow.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include "NoEvents.hpp"
#include "Dialog.hpp"
#include <map>

namespace app
{

    ApplicationCalendar::ApplicationCalendar(std::string name,
                                             std::string parent,
                                             uint32_t stackDepth,
                                             sys::ServicePriority priority)
        : Application(name, parent, false, stackDepth, priority)
    {}

    sys::Message_t ApplicationCalendar::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
    {
        return Application::DataReceivedHandler(msgl);
    }

    sys::ReturnCodes ApplicationCalendar::InitHandler()
    {
        auto ret = Application::InitHandler();
        createUserInterface();
        return ret;
    }

    sys::ReturnCodes ApplicationCalendar::DeinitHandler()
    {
        return sys::ReturnCodes::Success;
    }

    void ApplicationCalendar::createUserInterface()
    {

        windows.insert(std::pair<std::string, gui::AppWindow *>(
            gui::name::window::main_window, new gui::CalendarMainWindow(this, gui::name::window::main_window)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::day_events_window,
            new DayEventsWindow(this, style::window::calendar::name::day_events_window)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::no_events_window,
            new gui::NoEvents(this, style::window::calendar::name::no_events_window, gui::NoEvents::Meta())));
        windows.insert(std::pair<std::string, gui::AppWindow *>(style::window::calendar::name::events_options,
                                                                new gui::CalendarEventsOptions(this)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::dialog_yes_no,
            new gui::DialogYesNo(this, style::window::calendar::name::dialog_yes_no)));
    }

    void ApplicationCalendar::destroyUserInterface()
    {}

} /* namespace app */
