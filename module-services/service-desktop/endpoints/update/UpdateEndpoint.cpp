﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "UpdateEndpoint.hpp"

#include <service-desktop/DesktopMessages.hpp>
#include <service-desktop/ServiceDesktop.hpp>
#include <endpoints/Context.hpp>
#include <endpoints/messages/MessageHelper.hpp>

#include <json11.hpp>
#include <purefs/filesystem_paths.hpp>

#include <filesystem>
#include <memory>

using namespace parserFSM;

auto UpdateEndpoint::handle(Context &context) -> void
{
    auto &p               = helperSwitcher(context);
    auto [sent, response] = p.process(context.getMethod(), context);
    if (sent == sent::delayed) {
        LOG_DEBUG("There is no proper delayed serving mechanism - depend on invisible context caching");
    }
    if (sent == sent::no) {
        if (not response) {
            LOG_ERROR("Response not sent & response not created : respond with error");
            context.setResponseStatus(http::Code::NotAcceptable);
        }
        else {
            context.setResponse(response.value());
        }

        MessageHandler::putToSendQueue(context.createSimpleResponse());
    }
    if (sent == sent::yes and response) {
        LOG_ERROR("Response set when we already handled response in handler");
    }
}

auto UpdateEndpoint::helperSwitcher(parserFSM::Context &ctx) -> parserFSM::BaseHelper &
{
    return *updateHelper;
}
