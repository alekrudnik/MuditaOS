﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-time/CalendarTimeEvents.hpp>
#include <service-time/TimeEvents.hpp>

#include <BaseInterface.hpp>
#include <Common/Query.hpp>
#include <application-calendar/widgets/CalendarStyle.hpp>
#include <module-apps/application-popup/ApplicationPopup.hpp>
#include <module-apps/application-calendar/data/CalendarData.hpp>
#include <module-apps/application-calendar/data/dateCommon.hpp>
#include <module-db/queries/calendar/QueryEventsSelectFirstUpcoming.hpp>

#include <module-gui/gui/SwitchData.hpp>
#include <queries/calendar/QueryEventsEdit.hpp>
#include <service-appmgr/Controller.hpp>
#include <service-db/DBServiceAPI.hpp>

#include <algorithm>
#include <chrono>
#include <type_traits>
#include <utility>
#include <vector>

namespace sys
{
    class Service;
} // namespace sys

namespace stm
{
    using namespace std::chrono_literals;
    constexpr static auto eventTimerMinSkipInterval = 100ms;

    CalendarTimeEvents::CalendarTimeEvents(sys::Service *service) : TimeEvents(service)
    {}

    bool CalendarTimeEvents::sendNextEventQuery()
    {
        TimePoint filterFrom = TimePointNow();
        TimePoint filterTill = filterFrom;
        if (startTP != TIME_POINT_INVALID) {
            filterFrom = std::min(startTP, filterFrom);
            filterTill = filterFrom;
        }

        const auto [succeed, _] =
            DBServiceAPI::GetQuery(service(),
                                   db::Interface::Name::Events,
                                   std::make_unique<db::query::events::SelectFirstUpcoming>(filterFrom, filterTill));
        return succeed;
    }

    uint32_t CalendarTimeEvents::calcToNextEventInterval(std::unique_ptr<db::QueryResult> nextEventQueryResult)
    {
        auto firstUpcomingQuery =
            dynamic_cast<db::query::events::SelectFirstUpcomingResult *>(nextEventQueryResult.get());
        if (firstUpcomingQuery == nullptr) {
            return 0;
        }

        eventRecords = firstUpcomingQuery->getResult();
        std::reverse(eventRecords.begin(), eventRecords.end());

        if (eventRecords.size() == 0) {
            return 0;
        }

        auto eventRecord = eventRecords.at(0);
        startTP          = eventRecord.date_from - std::chrono::minutes{eventRecord.reminder};
        auto duration    = eventRecord.date_from - std::chrono::minutes{eventRecord.reminder} - TimePointNow();
        if (duration.count() <= 0) {
            duration = std::chrono::milliseconds(eventTimerMinSkipInterval);
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    bool CalendarTimeEvents::sendEventFiredQuery()
    {
        bool result = true;
        for (auto eventRecord : eventRecords) {
            eventRecord.reminder_fired = TimePointNow();
            const auto [succeed, _]    = DBServiceAPI::GetQuery(
                service(), db::Interface::Name::Events, std::make_unique<db::query::events::Edit>(eventRecord));
            result = result & succeed;
        }
        return result;
    }

    void CalendarTimeEvents::invokeEvent()
    {
        std::unique_ptr<EventRecordsData> eventsData = std::make_unique<EventRecordsData>();
        eventsData->setDescription(style::window::name::event_reminder_window);
        auto events = std::make_shared<std::vector<EventsRecord>>(eventRecords);
        eventsData->setData(events);
        app::manager::Controller::sendAction(service(),
                                             app::manager::actions::ShowReminder,
                                             std::move(eventsData),
                                             app::manager::OnSwitchBehaviour::RunInBackground);
    }
} // namespace stm
