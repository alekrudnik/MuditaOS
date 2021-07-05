// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "BaseMessage.hpp"

namespace app::manager
{
    class UpdateInProgress : public BaseMessage
    {
      public:
        explicit UpdateInProgress(const ApplicationName &sender);
    };
} // namespace app::manager
