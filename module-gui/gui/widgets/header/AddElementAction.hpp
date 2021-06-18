// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <BoxLayout.hpp>

namespace gui::header
{
    class AddElementAction : public HBox
    {
      public:
        explicit AddElementAction(Item *parent = nullptr);
    };
} // namespace gui::header
