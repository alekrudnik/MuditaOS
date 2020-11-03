// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <Style.hpp>

namespace callAppStyle
{
    namespace strings
    {
        inline constexpr auto call      = "app_call_call";
        inline constexpr auto clear     = "app_call_clear";
        inline constexpr auto reject    = "app_call_reject";
        inline constexpr auto answer    = "app_call_answer";
        inline constexpr auto message   = "app_call_message";
        inline constexpr auto endcall   = "app_call_end_call";
        inline constexpr auto emergency = "app_call_emergency";
        inline constexpr auto iscalling = "app_call_is_calling";
        inline constexpr auto calling   = "app_call_calling";
        inline constexpr auto callended = "app_call_call_ended";
        inline constexpr auto contact   = "app_call_contact";
        inline constexpr auto mute      = "app_call_mute";
        inline constexpr auto MUTED     = "app_call_muted";
        inline constexpr auto speaker   = "app_call_speaker";
        inline constexpr auto speakeron = "app_call_speaker_on";
        inline constexpr auto bluetooth = "app_call_bluetooth";
    } // namespace strings

    namespace numberLabel
    {
        inline constexpr auto x       = 60;
        inline constexpr auto y       = 157;
        inline constexpr auto w       = style::window_width - 2 * x;
        inline constexpr auto h       = 51 + 16;
        inline constexpr auto borderW = 1;
    } // namespace numberLabel

    namespace icon
    {
        inline constexpr auto x_margin = 20;
    }

    // ENTER NUMBER WINDOW
    namespace enterNumberWindow
    {
        namespace newContactIcon
        {
            inline constexpr auto x = 190 - icon::x_margin;
            inline constexpr auto y = 411;
        } // namespace newContactIcon
    }     // namespace enterNumberWindow

    // CALL WINDOW
    namespace callWindow
    {
        namespace imageCircleTop
        {
            inline constexpr auto x    = 116; // TODO: should be 104 with final image
            inline constexpr auto y    = 59;
            inline constexpr auto name = "circle_top";
        } // namespace imageCircleTop
        namespace imageCircleBottom
        {
            inline constexpr auto x    = 106; // TODO: should be 104 with final image
            inline constexpr auto y    = 240;
            inline constexpr auto name = "circle_bottom";
        } // namespace imageCircleBottom
        namespace durationLabel
        {
            inline constexpr auto x = 120;
            inline constexpr auto y = 223;
            inline constexpr auto w = 240;
            inline constexpr auto h = 20;
        } // namespace durationLabel
        namespace speakerIcon
        {
            inline constexpr auto x = 260 - icon::x_margin;
            inline constexpr auto y = 411;
        } // namespace speakerIcon
        namespace microphoneIcon
        {
            inline constexpr auto x = 120 - icon::x_margin;
            inline constexpr auto y = 411;
        } // namespace microphoneIcon
        namespace sendMessageIcon
        {
            inline constexpr auto x = 190 - icon::x_margin;
            inline constexpr auto y = 411;
        } // namespace sendMessageIcon
    }     // namespace callWindow
} // namespace callAppStyle
