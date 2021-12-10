// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "PhoneLockedWindowBase.hpp"
#include <apps-common/popups/presenter/QuotesPresenter.hpp>

namespace gui
{
    namespace style
    {
        namespace textBox
        {
            constexpr auto y = 110;
            constexpr auto h = 340;
        } // namespace textBox

        namespace text
        {
            constexpr auto h             = 225;
            constexpr auto topMarigin    = 20;
            constexpr auto bottomMarigin = 30;

        } // namespace text

    } // namespace style

    class PhoneLockedWindowQuote : public PhoneLockedWindowBase
    {
      private:
        std::unique_ptr<QuotesPresenter> quotesPresenter;
        VBox *textBox    = nullptr;
        Text *quoteText  = nullptr;
        Text *authorText = nullptr;

      public:
        PhoneLockedWindowQuote(app::ApplicationCommon *app, const std::string &name);
        void buildInterface() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
    };

} /* namespace gui */
