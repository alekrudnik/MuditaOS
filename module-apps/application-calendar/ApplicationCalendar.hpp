#pragma once

#include "Application.hpp"
#include "Service/Message.hpp"
#include "SystemManager/SystemManager.hpp"
#include "gui/widgets/Label.hpp"

namespace app
{

    inline const std::string name_calendar = "ApplicationCalendar";

    class ApplicationCalendar : public Application
    {
        time_t applicationStartTime = 0;
        uint32_t eventShift         = 0;

      public:
        ApplicationCalendar(std::string name,
                            std::string parent,
                            uint32_t stackDepth           = 4096,
                            sys::ServicePriority priority = sys::ServicePriority::Idle);

        sys::Message_t DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp) override;
        sys::ReturnCodes InitHandler() override;
        sys::ReturnCodes DeinitHandler() override;

        sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final
        {
            return sys::ReturnCodes::Success;
        }
        time_t getCurrentTimeStamp()
        {
            return applicationStartTime;
        }

        void createUserInterface() override;
        void destroyUserInterface() override;
        void switchToNoEventsWindow(const std::string &title,
                                    const uint32_t &dateFilter,
                                    const std::string &goBackWindow);
        void applyRepeatAndReminderMap();

        std::map<uint32_t, std::string> reminderOptions;
        std::map<uint32_t, std::string> repeatOptions;
    };

} /* namespace app */
