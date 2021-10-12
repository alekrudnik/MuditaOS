﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhonebookMainWindow.hpp"
#include "application-phonebook/ApplicationPhonebook.hpp"
#include "application-phonebook/data/PhonebookItemData.hpp"
#include "application-phonebook/data/PhonebookStyle.hpp"

#include <memory>
#include <queries/phonebook/QueryContactGet.hpp>

#include <header/AddElementAction.hpp>
#include <header/SearchAction.hpp>
#include <service-appmgr/Controller.hpp>
#include <service-db/QueryMessage.hpp>
#include <service-db/DBNotificationMessage.hpp>
#include <utility>

namespace gui
{
    PhonebookMainWindow::PhonebookMainWindow(app::ApplicationCommon *app, std::shared_ptr<SearchModel> model)
        : AppWindow(app, gui::name::window::main_window),
          model(std::move(model)), phonebookModel{std::make_shared<PhonebookModel>(this->application)}
    {
        buildInterface();
    }

    void PhonebookMainWindow::rebuild()
    {
        contactsList->rebuildList(gui::listview::RebuildType::InPlace);
    }

    void PhonebookMainWindow::buildInterface()
    {
        AppWindow::buildInterface();

        setTitle(utils::translate("app_phonebook_title_main"));
        header->navigationIndicatorAdd(new gui::header::AddElementAction(), gui::header::BoxSelection::Left);
        header->navigationIndicatorAdd(new gui::header::SearchAction(), gui::header::BoxSelection::Right);

        bottomBar->setText(BottomBar::Side::LEFT, utils::translate(style::strings::common::call));
        bottomBar->setText(BottomBar::Side::CENTER, utils::translate(style::strings::common::open));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::translate(style::strings::common::back));

        contactsList = new gui::PhonebookListView(this,
                                                  phonebookStyle::mainWindow::contactsList::x,
                                                  phonebookStyle::mainWindow::contactsList::y,
                                                  phonebookStyle::mainWindow::contactsList::w,
                                                  phonebookStyle::mainWindow::contactsList::h,
                                                  phonebookModel);
        setFocusItem(contactsList);

        emptyListIcon =
            new gui::Icon(this,
                          0,
                          ::style::window::default_vertical_pos,
                          ::style::window_width,
                          ::style::window_height - ::style::window::default_vertical_pos - ::style::footer::height,
                          "empty_list_add_W_G",
                          utils::translate("app_phonebook_no_contacts"));

        contactsList->emptyListCallback    = [this]() { onEmptyList(); };
        contactsList->notEmptyListCallback = [this]() { onListFilled(); };

        phonebookModel->setDisplayMode(static_cast<uint32_t>(ContactDisplayMode::SortedByLetter));
        contactsList->rebuildList(gui::listview::RebuildType::Full);

        auto app  = application;
        inputMode = std::make_unique<InputMode>(
            std::list<InputMode::Mode>{InputMode::ABC, InputMode::abc},
            [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text); },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); },
            [app]() { app->getCurrentWindow()->selectSpecialCharacter(); });
    }

    void PhonebookMainWindow::destroyInterface()
    {
        erase();
    }

    PhonebookMainWindow::~PhonebookMainWindow()
    {
        destroyInterface();
    }

    void PhonebookMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        LOG_INFO("onBeforeShow");

        auto contactRequest = dynamic_cast<PhonebookSearchRequest *>(data);
        model->setRequested(contactRequest != nullptr);
        if (model->requestedSearch()) {
            enableNewContact                       = false;
            phonebookModel->messagesSelectCallback = [=](gui::PhonebookItem *item) {
                std::unique_ptr<PhonebookSearchRequest> data = std::make_unique<PhonebookSearchRequest>();
                data->result                                 = item->contact;
                data->setDescription("PhonebookSearchRequest");
                return app::manager::Controller::switchBack(
                    application,
                    std::make_unique<app::manager::SwitchBackRequest>(application->GetName(), std::move(data)));
            };

            header->navigationIndicatorRemove(gui::header::BoxSelection::Left);
            header->navigationIndicatorRemove(gui::header::BoxSelection::Right);
        }
    }

    void PhonebookMainWindow::HandleFilteringByLetter(const InputEvent &inputEvent)
    {
        auto code = translator.handle(inputEvent.getRawKey(), inputMode ? inputMode->get() : "");
        if (code != Profile::none_key) {
            LOG_INFO("char=' %c'", static_cast<char>(code));
            char letter = static_cast<char>(code);
            std::string filterLetter;
            filterLetter.push_back(letter);

            LOG_DEBUG("Number of favourites contacts : %" PRIu32, phonebookModel->letterMap.favouritesCount);
            uint32_t dataOffset = phonebookModel->letterMap.firstLetterDictionary[filterLetter];
            if (dataOffset != phonebookContactsMap::NO_MATCH_FOUND) {
                LOG_DEBUG("PhoneBook Data Offset : %" PRIu32, dataOffset);
                phonebookModel->setDisplayMode(static_cast<uint32_t>(ContactDisplayMode::SortedByLetter));
                contactsList->rebuildList(gui::listview::RebuildType::OnOffset, dataOffset);
            }
        }
    }

    bool PhonebookMainWindow::onInput(const InputEvent &inputEvent)
    {
        if (inputEvent.isShortRelease()) {
            if (inputEvent.is(gui::KeyCode::KEY_LEFT)) {
                if (enableNewContact) {
                    std::unique_ptr<gui::SwitchData> data = std::make_unique<PhonebookItemData>();
                    application->switchWindow(
                        gui::window::name::new_contact, gui::ShowMode::GUI_SHOW_INIT, std::move(data));
                    return true;
                }
            }
            else if (inputEvent.is(gui::KeyCode::KEY_RIGHT) &&
                     header->navigationIndicatorVisible(gui::header::BoxSelection::Right)) {
                application->switchWindow(gui::window::name::search);
                return true;
            }
            else {
                HandleFilteringByLetter(inputEvent);
            }
        }
        return AppWindow::onInput(inputEvent);
    }

    bool PhonebookMainWindow::onDatabaseMessage(sys::Message *msgl)
    {
        auto *msgNotification = dynamic_cast<db::NotificationMessage *>(msgl);
        if (msgNotification != nullptr) {
            if (msgNotification->interface == db::Interface::Name::Contact) {

                if (msgNotification->dataModified()) {

                    phonebookModel->letterMap = phonebookModel->requestLetterMap();
                    rebuild();

                    return true;
                }
            }
        }

        return false;
    }

    void PhonebookMainWindow::onEmptyList()
    {
        bottomBar->setActive(gui::BottomBar::Side::LEFT, false);
        bottomBar->setActive(gui::BottomBar::Side::CENTER, false);
        emptyListIcon->setVisible(true);
        header->navigationIndicatorRemove(gui::header::BoxSelection::Right);
        application->refreshWindow(RefreshModes::GUI_REFRESH_DEEP);
    }

    void PhonebookMainWindow::onListFilled()
    {
        bottomBar->setActive(gui::BottomBar::Side::LEFT, true);
        bottomBar->setActive(gui::BottomBar::Side::CENTER, true);
        emptyListIcon->setVisible(false);
        header->navigationIndicatorAdd(new gui::header::SearchAction(), gui::header::BoxSelection::Right);
        application->refreshWindow(RefreshModes::GUI_REFRESH_DEEP);
    }
} /* namespace gui */
