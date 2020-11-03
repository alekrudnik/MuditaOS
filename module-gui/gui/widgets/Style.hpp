// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <gui/core/Color.hpp>
#include <gui/Common.hpp>
#include <Alignment.hpp>
#include <Margins.hpp>

namespace gui
{
    class Rect;
    class Label;
}; // namespace gui

/// one place to gather all common magical numbers from design
namespace style
{
    inline constexpr auto window_height = 600;
    inline constexpr auto window_width  = 480;
    namespace header
    {
        inline constexpr auto height = 105;
        namespace font
        {
            inline constexpr auto time  = "gt_pressura_regular_24";
            inline constexpr auto modes = "gt_pressura_regular_20";
            inline constexpr auto title = "gt_pressura_bold_32";
        }; // namespace font
    };     // namespace header

    namespace footer
    {
        inline constexpr auto height = 54;
        namespace font
        {
            inline constexpr auto bold   = "gt_pressura_bold_24";
            inline constexpr auto medium = "gt_pressura_regular_24";
        }; // namespace font
    };     // namespace footer

    namespace window
    {
        inline constexpr auto default_left_margin  = 20;
        inline constexpr auto default_right_margin = 20;
        inline constexpr auto default_body_width =
            style::window_width - style::window::default_right_margin - style::window::default_left_margin;
        inline constexpr auto default_body_height =
            style::window_height - style::header::height - style::footer::height;
        inline constexpr auto default_border_focus_w       = 2;
        inline constexpr auto default_border_rect_no_focus = 1;
        inline constexpr auto default_border_no_focus_w    = 0;
        inline constexpr auto default_rect_yaps            = 10;
        namespace font
        {
            inline constexpr auto supersizemelight = "gt_pressura_light_90";
            inline constexpr auto largelight       = "gt_pressura_light_46";
            inline constexpr auto verybigbold      = "gt_pressura_bold_32";
            inline constexpr auto bigbold          = "gt_pressura_bold_30";
            inline constexpr auto big              = "gt_pressura_regular_30";
            inline constexpr auto biglight         = "gt_pressura_light_30";
            inline constexpr auto mediumbold       = "gt_pressura_bold_27";
            inline constexpr auto medium           = "gt_pressura_regular_27";
            inline constexpr auto mediumlight      = "gt_pressura_light_27";
            inline constexpr auto smallbold        = "gt_pressura_bold_24";
            inline constexpr auto small            = "gt_pressura_regular_24";
            inline constexpr auto verysmallbold    = "gt_pressura_bold_20";
            inline constexpr auto verysmall        = "gt_pressura_regular_20";
        }; // namespace font

        inline constexpr auto list_offset_default = 12;
        namespace label
        {
            inline constexpr auto small_h   = 33;
            inline constexpr auto default_h = 50;
            inline constexpr auto big_h     = 55;
        }; // namespace label

        /// minimal label decoration - edges, focus & alignment
        void decorate(gui::Rect *el);
        void decorate(gui::Label *el);
        /// minimal label decoration for Option
        void decorateOption(gui::Label *el);

    }; // namespace window

    namespace settings
    {
        namespace date
        {
            inline constexpr auto date_time_item_height       = 107;
            inline constexpr auto date_time_item_width        = 120;
            inline constexpr auto date_time_item_title_height = 30;
            inline constexpr auto date_time_spacer_width      = 20;
            inline constexpr auto date_time_x_offset          = 30;

            inline constexpr auto date_time_box_h = 107;
            inline constexpr auto date_box_y_pos  = 123;
            inline constexpr auto time_box_y_pos  = 285;

        } // namespace date
        namespace ussd
        {
            inline constexpr auto commonXPos = 40;
            inline constexpr auto commonYPos = 110;

            inline constexpr auto commonW      = 420;
            inline constexpr auto commonLabelH = 33;
            inline constexpr auto commonTextH  = 99;
        } // namespace ussd
    }     // namespace settings
    namespace color
    {
        inline constexpr auto lightgrey = gui::Color(3, 0);
    }; //  namespace color
    namespace text
    {
        inline constexpr auto defaultTextAlignment =
            gui::Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Bottom);
        inline constexpr auto maxTextLines = 10;
    }; // namespace text

    namespace strings
    {
        namespace common
        {
            inline constexpr auto add            = "common_add";
            inline constexpr auto open           = "common_open";
            inline constexpr auto call           = "common_call";
            inline constexpr auto send           = "common_send";
            inline constexpr auto save           = "common_save";
            inline constexpr auto confirm        = "common_confirm";
            inline constexpr auto select         = "common_select";
            inline constexpr auto use            = "common_use";
            inline constexpr auto ok             = "common_ok";
            inline constexpr auto back           = "common_back";
            inline constexpr auto set            = "common_set";
            inline constexpr auto yes            = "common_yes";
            inline constexpr auto no             = "common_no";
            inline constexpr auto Switch         = "common_switch";
            inline constexpr auto options        = "common_options";
            inline constexpr auto information    = "common_information";
            inline constexpr auto search         = "common_search";
            inline constexpr auto search_results = "common_search_results";
            inline constexpr auto emoji          = "common_emoji";
            inline constexpr auto special_chars  = "common_special_characters";
            inline constexpr auto start          = "common_start";
            inline constexpr auto stop           = "common_stop";
            inline constexpr auto resume         = "common_resume";
            inline constexpr auto pause          = "common_pause";
            // days
            inline constexpr auto Monday    = "common_monday";
            inline constexpr auto Tuesday   = "common_tuesday";
            inline constexpr auto Wednesday = "common_wednesday";
            inline constexpr auto Thursday  = "common_thursday";
            inline constexpr auto Friday    = "common_friday";
            inline constexpr auto Saturday  = "common_saturday";
            inline constexpr auto Sunday    = "common_sunday";
            // months
            inline constexpr auto January   = "common_january";
            inline constexpr auto February  = "common_february";
            inline constexpr auto March     = "common_march";
            inline constexpr auto April     = "common_april";
            inline constexpr auto May       = "common_may";
            inline constexpr auto June      = "common_june";
            inline constexpr auto July      = "common_july";
            inline constexpr auto August    = "common_august";
            inline constexpr auto September = "common_september";
            inline constexpr auto October   = "common_october";
            inline constexpr auto November  = "common_november";
            inline constexpr auto December  = "common_december";
            inline constexpr auto Yesterday = "common_yesterday";
            inline constexpr auto Today     = "common_today";
        } // namespace common
    }     // namespace strings

    namespace listview
    {
        /// Possible List boundaries handling types
        enum class Boundaries
        {
            Fixed,     ///< Fixed - list will stop scrolling on first or last elements on appropriate top or bottom
                       ///< directions.
            Continuous ///< Continuous - list will continue to beginning or end on first or last elements on
                       ///< appropriate top or bottom directions.
        };

        /// Possible List scrolling directions
        enum class Direction
        {
            Top,
            Bottom
        };

        /// Possible List rebuild types
        enum class RebuildType
        {
            Full,    ///< Full rebuild - resets lists to all initial conditions and request data from beginning.
            InPlace, ///< InPlace rebuild - stores currently focused part of list and rebuild from that part.
            OnOffset ///< OnOffset rebuild - resets lists to all initial conditions and request data from provided
                     ///< offset.
        };

        enum class Orientation
        {
            TopBottom,
            BottomTop
        };

        namespace scroll
        {
            inline constexpr auto x           = 0;
            inline constexpr auto y           = 0;
            inline constexpr auto w           = 5;
            inline constexpr auto h           = 0;
            inline constexpr auto radius      = 2;
            inline constexpr auto color       = gui::Color{0, 0};
            inline constexpr auto margin      = 5;
            inline constexpr auto min_space   = 10;
            inline constexpr auto item_margin = 10;
        } // namespace scroll

        inline constexpr auto item_width_with_scroll =
            style::window::default_body_width - style::listview::scroll::item_margin;
        inline constexpr auto body_width_with_scroll =
            style::window::default_body_width + style::listview::scroll::margin;
        inline constexpr auto right_margin        = 15;
        inline constexpr auto top_margin_small    = 5;
        inline constexpr auto top_margin_big      = 8;
        inline constexpr auto top_margin_very_big = 12;
        inline constexpr auto item_span_small     = 8;
        inline constexpr auto item_span_big       = 12;

    } // namespace listview

    namespace margins
    {
        inline constexpr auto small    = 6;
        inline constexpr auto big      = 8;
        inline constexpr auto very_big = 12;
        inline constexpr auto huge     = 24;
    } // namespace margins

    namespace padding
    {
        inline constexpr auto default_left_text_padding = 10;
    } // namespace padding

}; // namespace style
