// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhoneLockedWindowQuote.hpp"
#include <service-appmgr/Controller.hpp>
#include <ImageBox.hpp>
#include <Text.hpp>
#include <Style.hpp>

namespace gui
{
    PhoneLockedWindowQuote::PhoneLockedWindowQuote(app::ApplicationCommon *app, const std::string &name)
        : PhoneLockedWindowBase(app, name), quotesPresenter{std::make_unique<QuotesPresenter>(app)}
    {
        buildInterface();

        quotesPresenter->setQuoteDisplayCallback([&](std::string quote, std::string author) {
            quoteText->setText(quote);
            authorText->setText(std::string("- ") + author);
            textBox->resizeItems();
        });
    }

    void PhoneLockedWindowQuote::buildInterface()
    {
        textBox = new VBox(this,
                           ::style::window::default_left_margin,
                           style::textBox::y,
                           ::style::window::default_body_width,
                           style::textBox::h);
        textBox->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Top));
        textBox->setEdges(RectangleEdge::None);

        auto quoteImage = new ImageBox(textBox, new Image("quote", ImageTypeSpecifier::W_G));
        quoteImage->setMinimumSizeToFitImage();
        quoteImage->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Top));
        quoteImage->setMargins(Margins(0, 0, 0, 0));

        quoteText = new Text(textBox, 0, 0, 0, 0);
        quoteText->setMaximumSize(::style::window::default_body_width, style::text::h);
        quoteText->setMargins(Margins(0, style::text::topMarigin, 0, style::text::bottomMarigin));
        quoteText->setFont(::style::window::font::biglight);
        quoteText->setEdges(RectangleEdge::None);
        quoteText->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));

        authorText = new Text(textBox, 0, 0, 0, 0);
        authorText->setMaximumSize(::style::window::default_body_width, 30);
        authorText->setMargins(Margins(0, 0, 0, 0));
        authorText->setFont(::style::window::font::small);
        authorText->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));

        textBox->resizeItems();
    }

    void PhoneLockedWindowQuote::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        navBar->setActive(nav_bar::Side::Left, false);
        navBar->setText(nav_bar::Side::Center, utils::translate("app_desktop_unlock"));
        navBar->setActive(nav_bar::Side::Right, false);

        quotesPresenter->requestQuote();
    }

} /* namespace gui */
