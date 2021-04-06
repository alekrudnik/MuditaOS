// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "TetheringPhoneModePopup.hpp"
#include "DialogMetadataMessage.hpp"
#include "Application.hpp"
#include "SystemManager/messages/TetheringPhoneModeChangeProhibitedMessage.hpp"
#include <service-appmgr/Controller.hpp>

namespace gui
{
    TetheringPhoneModePopup::TetheringPhoneModePopup(app::Application *app, const std::string &name)
        : TetheringNotificationPopup{app, name}
    {}

    void TetheringPhoneModePopup::onBeforeShow(ShowMode mode, [[maybe_unused]] SwitchData *data)
    {
        DialogMetadata metadata;
        metadata.title  = "";
        metadata.text   = utils::translateI18("tethering_phone_mode_change_prohibited");
        metadata.icon   = "info_big_circle_W_G";
        metadata.action = [this]() {
            auto params = std::make_unique<app::PopupRequestParams>(gui::popup::ID::TetheringPhoneModeChangeProhibited);
            app::manager::Controller::sendAction(application, app::manager::actions::AbortPopup, std::move(params));
            return true;
        };
        auto msg = std::make_unique<DialogMetadataMessage>(std::move(metadata));
        DialogConfirm::onBeforeShow(mode, msg.get());
    }
} // namespace gui
