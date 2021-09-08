// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <AlarmOperations.hpp>

namespace alarms
{
    class AlarmOperationsFactory : public IAlarmOperationsFactory
    {
      public:
        std::unique_ptr<IAlarmOperations> create(
            std::unique_ptr<AbstractAlarmEventsRepository> &&alarmEventsRepo,
            IAlarmOperations::GetCurrentTime getCurrentTimeCallback) const override;
    };

    class AlarmOperations : public AlarmOperationsCommon
    {
      public:
        using AlarmOperationsCommon::AlarmOperationsCommon;
    };
} // namespace alarms
