﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "MessageHandler.hpp"
#include "ParserUtils.hpp"

#include <json/json11.hpp>

#include <string>

namespace parserFSM
{
    enum class State
    {
        NoMsg,
        ReceivedPartialHeader,
        ReceivedPartialPayload,
        ReceivedPayload,
    };

    class StateMachine
    {
      public:
        explicit StateMachine(sys::Service *OwnerService);
        void processMessage(std::string &&msg);
        [[nodiscard]] auto getCurrentState() const noexcept -> State
        {
            return state;
        };

        void setState(const parserFSM::State newState) noexcept
        {
            state = newState;
        }

      private:
        std::string receivedMsg;
        parserFSM::State state = State::NoMsg;
        std::string payload;
        std::string header;
        unsigned long payloadLength   = 0;
        sys::Service *OwnerServicePtr = nullptr;

        void parseHeader();
        void parsePartialHeader();
        void parseNewMessage();
        void parsePartialMessage();
        void parsePayload();
    };
} // namespace parserFSM
