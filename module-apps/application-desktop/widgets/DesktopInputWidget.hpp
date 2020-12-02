// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Application.hpp"
#include <AppWindow.hpp>

#include "Text.hpp"
#include "Image.hpp"
#include "ListItem.hpp"
#include <BoxLayout.hpp>

namespace gui
{

    // class DesktopInputWidget : public ListItem
    class DesktopInputWidget : public HBox
    {
        app::Application *application = nullptr;
        HBox *body                    = nullptr;
        gui::Image *replyImage        = nullptr;

      public:
        gui::Text *inputText = nullptr;

        DesktopInputWidget(app::Application *application, Item *parent, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        ~DesktopInputWidget() override = default;

        auto onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim) -> bool override;
        auto handleRequestResize(const Item *, unsigned short request_w, unsigned short request_h) -> Size override;
    };

} /* namespace gui */
