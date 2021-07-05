// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "application-messages/models/SMSTemplateModel.hpp"
#include "application-messages/data/SMSdata.hpp"

#include <AppWindow.hpp>
#include <gui/widgets/Label.hpp>
#include <gui/widgets/ListView.hpp>

#include <string>
#include <functional>

namespace gui
{
    class SMSTemplatesWindow : public AppWindow
    {
        std::shared_ptr<SMSTemplateModel> smsTemplateModel;
        gui::ListView *list = nullptr;

        void smsSendTemplateRequestHandler(const SMSSendTemplateRequest *const switchData);
        void smsTemplateRequestHandler(const SMSTemplateRequest *const switchData);

      public:
        SMSTemplatesWindow() = delete;
        SMSTemplatesWindow(app::Application *app);
        virtual ~SMSTemplatesWindow();

        void onBeforeShow(ShowMode mode, SwitchData *data) override;

        void rebuild() override;
        void buildInterface() override;
        void destroyInterface() override;
    };

} /* namespace gui */
