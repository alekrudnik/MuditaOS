// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "PhoneLockedWindowBase.hpp"

namespace gui
{
    namespace style
    {
        namespace logo
        {
            constexpr auto y = 163;
        } // namespace logo

    } // namespace style
    class PhoneLockedWindowLogo : public PhoneLockedWindowBase
    {
      private:
      public:
        PhoneLockedWindowLogo(app::ApplicationCommon *app, const std::string &name);
        void buildInterface() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
    };

} /* namespace gui */
