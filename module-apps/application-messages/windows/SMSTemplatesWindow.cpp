﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SMSTemplatesWindow.hpp"

#include "application-messages/ApplicationMessages.hpp"
#include "application-messages/widgets/SMSTemplateItem.hpp"
#include "application-messages/data/MessagesStyle.hpp"

#include <service-appmgr/Controller.hpp>
#include <i18n/i18n.hpp>
#include <Style.hpp>
#include <log/log.hpp>

#include <cassert>
#include <memory>

namespace gui
{
    SMSTemplatesWindow::SMSTemplatesWindow(app::Application *app)
        : AppWindow(app, name::window::sms_templates), smsTemplateModel{std::make_shared<SMSTemplateModel>(app)}
    {
        buildInterface();
    }

    SMSTemplatesWindow::~SMSTemplatesWindow()
    {
        destroyInterface();
    }

    void SMSTemplatesWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void SMSTemplatesWindow::buildInterface()
    {
        AppWindow::buildInterface();

        setTitle(utils::localize.get("app_messages_templates"));

        bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get(style::strings::common::use));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));

        namespace style = style::messages::templates::list;

        list = new gui::ListView(
            this, style::x, style::y, style::w, style::h, smsTemplateModel, listview::ScrollBarType::Fixed);

        setFocusItem(list);
    }

    void SMSTemplatesWindow::destroyInterface()
    {
        erase();
    }

    void SMSTemplatesWindow::smsTemplateRequestHandler(const SMSTemplateRequest *const switchData)
    {
        auto app = dynamic_cast<app::ApplicationMessages *>(application);
        assert(app != nullptr);

        auto requestingWindow  = switchData->requestingWindow;
        app->templatesCallback = [=](std::shared_ptr<SMSTemplateRecord> templ) {
            LOG_DEBUG("SMS template id = %" PRIu32 "chosen", templ->ID);
            std::unique_ptr<gui::SwitchData> data =
                std::make_unique<SMSTextData>(templ->text, SMSTextData::Concatenate::True);
            application->switchWindow(requestingWindow, std::move(data));
            return true;
        };
    }

    void SMSTemplatesWindow::smsSendTemplateRequestHandler(const SMSSendTemplateRequest *const switchData)
    {
        auto app = dynamic_cast<app::ApplicationMessages *>(application);
        assert(app != nullptr);

        auto phoneNumber       = switchData->getPhoneNumber();
        app->templatesCallback = [=](std::shared_ptr<SMSTemplateRecord> templ) {
            LOG_DEBUG("SMS template id = %" PRIu32 "sent to %s", templ->ID, phoneNumber.getFormatted().c_str());
            app->sendSms(phoneNumber, templ->text);
            app::manager::Controller::switchBack(app,
                                                 std::make_unique<app::manager::SwitchBackRequest>(
                                                     application->GetName(), std::make_unique<SMSTemplateSent>()));
            return true;
        };
    }

    void SMSTemplatesWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        if (mode == ShowMode::GUI_SHOW_INIT) {
            list->rebuildList();
        }

        if (auto switchData = dynamic_cast<SMSTemplateRequest *>(data)) {
            smsTemplateRequestHandler(switchData);
        }

        if (auto switchData = dynamic_cast<SMSSendTemplateRequest *>(data); switchData != nullptr) {
            smsSendTemplateRequestHandler(switchData);
        }
    }

} /* namespace gui */
