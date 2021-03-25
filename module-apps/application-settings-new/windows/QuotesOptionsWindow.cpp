// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "QuotesOptionsWindow.hpp"
#include "OptionSetting.hpp"
#include "application-settings-new/ApplicationSettings.hpp"
#include "DialogMetadataMessage.hpp"
#include "QuotesMainWindow.hpp"
#include "application-settings-new/ApplicationSettings.hpp"
#include "application-settings-new/widgets/QuoteWidget.hpp"

#include <i18n/i18n.hpp>
#include <widgets/Text.hpp>

namespace gui
{

    QuotesOptionsWindow::QuotesOptionsWindow(app::Application *app)
        : BaseSettingsWindow(app, gui::window::name::quotes), quotesModel(std::make_shared<Quotes::QuotesModel>(app))
    {
        setTitle(utils::localize.get("app_settings_display_wallpaper_quotes_options"));
    }

    auto QuotesOptionsWindow::buildOptionsList() -> std::list<gui::Option>
    {
        std::list<gui::Option> optionsList;

        optionsList.emplace_back(std::make_unique<gui::option::OptionSettings>(
            utils::translateI18("app_settings_display_wallpaper_quotes_edit"),
            [=](gui::Item &item) {
                application->switchWindow(gui::window::name::new_quote,
                                          std::make_unique<QuoteSwitchData>(QuoteAction::Edit, quote));
                return true;
            },
            [=](gui::Item &item) {
                if (item.focus) {
                    this->setBottomBarText(utils::translateI18(style::strings::common::select),
                                           BottomBar::Side::CENTER);
                }
                return true;
            }));

        optionsList.emplace_back(std::make_unique<gui::option::OptionSettings>(
            utils::translateI18("app_settings_display_wallpaper_quotes_delete"),
            [=](gui::Item &item) {
                auto metaData = std::make_unique<gui::DialogMetadataMessage>(gui::DialogMetadata{
                    quote.quote,
                    "phonebook_contact_delete_trashcan",
                    utils::localize.get("app_settings_display_wallpaper_quotes_delete_confirmation"),
                    "",
                    [this]() {
                        auto backToQuotesMainWindow = 2;
                        quotesModel->remove(quote);
                        application->returnToPreviousWindow(backToQuotesMainWindow);
                        return true;
                    }});
                application->switchWindow(
                    gui::window::name::quotes_dialog_yes_no, gui::ShowMode::GUI_SHOW_INIT, std::move(metaData));
                return true;
            },
            [=](gui::Item &item) {
                if (item.focus) {
                    this->setBottomBarText(utils::translateI18(style::strings::common::select),
                                           BottomBar::Side::CENTER);
                }
                return true;
            }));

        return optionsList;
    } // namespace gui

    void QuotesOptionsWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        auto *quoteSwitchData = dynamic_cast<QuoteSwitchData *>(data);
        if (quoteSwitchData != nullptr) {
            quote = quoteSwitchData->getQuote();
        }

        BaseSettingsWindow::onBeforeShow(mode, data);
    }
} // namespace gui
