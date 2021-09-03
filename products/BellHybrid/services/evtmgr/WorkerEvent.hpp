// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <service-evtmgr/WorkerEventCommon.hpp>

namespace bell
{
    class WorkerEvent : public WorkerEventCommon
    {
      public:
        explicit WorkerEvent(sys::Service *service);

      private:
        void addProductQueues(std::list<sys::WorkerQueueInfo> &queuesList) final;
        void initProductHardware() final;
        void deinitProductHardware() final;
        bool handleMessage(std::uint32_t queueID) override;
    };
} // namespace bell