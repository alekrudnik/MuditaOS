// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "BaseMessage.hpp"

#include <module-services/service-appmgr/service-appmgr/Actions.hpp>
#include <module-services/service-appmgr/service-appmgr/ApplicationManifest.hpp>

namespace app::manager
{
    class ActionRequest : public BaseMessage
    {
      public:
        ActionRequest(const ApplicationName &senderName, actions::ActionId _actionId, actions::ActionParamsPtr &&_data);

        [[nodiscard]] auto getAction() const noexcept -> actions::ActionId;
        [[nodiscard]] auto getData() noexcept -> actions::ActionParamsPtr &;

      private:
        actions::ActionId actionId;
        actions::ActionParamsPtr data;
    };
} // namespace app::manager
