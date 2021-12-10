// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhoneLockedWindowLogo.hpp"
#include <service-appmgr/Controller.hpp>
#include <ImageBox.hpp>

namespace gui
{
    PhoneLockedWindowLogo::PhoneLockedWindowLogo(app::ApplicationCommon *app, const std::string &name)
        : PhoneLockedWindowBase(app, name)
    {
        buildInterface();
    }

    void PhoneLockedWindowLogo::buildInterface()
    {
        auto quoteImage = new ImageBox(this, new Image("logo", ImageTypeSpecifier::W_G));
        quoteImage->setMinimumSizeToFitImage();
        quoteImage->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Top));
        quoteImage->setMargins(Margins(0, 0, 0, 0));
    }

    void PhoneLockedWindowLogo::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        navBar->setActive(nav_bar::Side::Left, false);
        navBar->setText(nav_bar::Side::Center, utils::translate("app_desktop_unlock"));
        navBar->setActive(nav_bar::Side::Right, false);
    }

} /* namespace gui */
