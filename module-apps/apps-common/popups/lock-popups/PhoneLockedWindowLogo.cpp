// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhoneLockedWindowLogo.hpp"
#include <service-appmgr/Controller.hpp>
#include <ImageBox.hpp>
#include <Style.hpp>

namespace gui
{
    PhoneLockedWindowLogo::PhoneLockedWindowLogo(app::ApplicationCommon *app, const std::string &name)
        : PhoneLockedWindowBase(app, name)
    {
        buildInterface();
    }

    void PhoneLockedWindowLogo::buildInterface()
    {
        auto logoImage = new ImageBox(this,
                                      0,
                                      style::logo::y,
                                      ::style::window_width,
                                      ::style::window::default_body_height - style::logo::y,
                                      new Image("logo", ImageTypeSpecifier::W_G));
        logoImage->setMinimumSizeToFitImage();
        logoImage->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Top));
    }

    void PhoneLockedWindowLogo::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        navBar->setActive(nav_bar::Side::Left, false);
        navBar->setText(nav_bar::Side::Center, utils::translate("app_desktop_unlock"));
        navBar->setActive(nav_bar::Side::Right, false);
    }

} /* namespace gui */
