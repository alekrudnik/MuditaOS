﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <MessageType.hpp>
#include <Service/Message.hpp>
#include <bsp/common.hpp>
#include <Service/Timer.hpp>

namespace sys
{
    class Service;
} // namespace sys

namespace EventManagerServiceAPI
{
    /*
     * @brief Gets board version.
     * @return board type
     */
    bsp::Board GetBoard(sys::Service *serv);

    void checkBatteryLevelCriticalState(sys::Service *serv);

    /*
     * @brief Call single vibra pulse
     */
    void vibraPulseOnce(sys::Service *serv);

    /*
     * @brief Call vibra to stop
     */
    void vibraStop(sys::Service *serv);

    /*
     * @brief Call repetitive vibra pulses for given time [ms]
     */
    void vibraPulseRepeat(sys::Service *serv, sys::ms time);

    /*
     * @brief Call repetitive vibra pulses until stop message is sent
     */
    void vibraPulseRepeatUntilStop(sys::Service *serv);

} // namespace EventManagerServiceAPI
