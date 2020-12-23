﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "ServiceGUI.hpp"

#include <core/Context.hpp>
#include <core/DrawCommand.hpp>
#include <gui/core/Renderer.hpp>
#include <Service/Worker.hpp>

#include <cstdint>

namespace service::gui
{
    class ServiceGUI;

    class WorkerGUI : public sys::Worker
    {
      public:
        enum class Signal
        {
            Render
        };
        static constexpr auto SignallingQueueName     = "SignallingQueue";
        static constexpr auto SignallingQueueCapacity = 1;
        static constexpr auto SignalSize              = sizeof(Signal);

        explicit WorkerGUI(ServiceGUI *service);

        void notify(Signal command);
        auto handleMessage(std::uint32_t queueID) -> bool override;

      private:
        void handleCommand(Signal command);
        void render(std::list<std::unique_ptr<::gui::DrawCommand>> &commands, ::gui::RefreshModes refreshMode);
        void onRenderingFinished(int contextId, ::gui::RefreshModes refreshMode);

        ServiceGUI *guiService;
        ::gui::Renderer renderer;
    };
} // namespace service::gui
