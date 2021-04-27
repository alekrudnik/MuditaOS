﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PhonebookNewContact.hpp"

#include "DialogMetadata.hpp"
#include "application-phonebook/ApplicationPhonebook.hpp"
#include "application-phonebook/data/PhonebookUtils.hpp"

#include <Dialog.hpp>
#include <service-db/DBServiceAPI.hpp>
#include <messages/DialogMetadataMessage.hpp>

namespace gui
{

    PhonebookNewContact::PhonebookNewContact(app::Application *app)
        : AppWindow(app, gui::window::name::new_contact), newContactModel{std::make_shared<NewContactModel>(app)}
    {
        buildInterface();
    }

    void PhonebookNewContact::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void PhonebookNewContact::buildInterface()
    {
        AppWindow::buildInterface();

        bottomBar->setText(BottomBar::Side::CENTER, utils::translate(style::strings::common::save));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::translate(style::strings::common::back));

        setTitle(utils::translate("app_phonebook_contact_title"));

        list = new gui::ListView(this,
                                 phonebookStyle::newContactWindow::newContactsList::x,
                                 phonebookStyle::newContactWindow::newContactsList::y,
                                 phonebookStyle::newContactWindow::newContactsList::w,
                                 phonebookStyle::newContactWindow::newContactsList::h,
                                 newContactModel,
                                 listview::ScrollBarType::PreRendered);
        setFocusItem(list);
    }

    void PhonebookNewContact::destroyInterface()
    {
        erase();
    }

    void PhonebookNewContact::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        if (mode != ShowMode::GUI_SHOW_RETURN) {
            newContactModel->clearData();
        }

        if (mode == ShowMode::GUI_SHOW_INIT) {
            list->rebuildList();
        }

        switch (contactAction) {
        case ContactAction::None:
            break;
        case ContactAction::Add:
        case ContactAction::EditTemporary:
            setTitle(utils::translate("app_phonebook_contact_title"));
            break;
        case ContactAction::Edit:
            setTitle(utils::translate("app_phonebook_options_edit"));
            break;
        }

        newContactModel->loadData(contact);
    }

    auto PhonebookNewContact::handleSwitchData(SwitchData *data) -> bool
    {
        if (data == nullptr) {
            return false;
        }

        auto *item = dynamic_cast<PhonebookItemData *>(data);
        if (item == nullptr) {
            return false;
        }

        contact = item->getContact();
        if (contact == nullptr) {
            contactAction = ContactAction::Add;
            contact       = std::make_shared<ContactRecord>();
            setSaveButtonVisible(false);
            return true;
        }

        if (contact->ID == DB_ID_NONE) {
            contactAction = ContactAction::Add;
            setSaveButtonVisible(false);
        }
        else if (contact->isTemporary()) {
            contactAction = ContactAction::EditTemporary;
        }
        else {
            contactAction = ContactAction::Edit;
            setSaveButtonVisible(true);
        }

        return true;
    }

    void PhonebookNewContact::setSaveButtonVisible(bool visible)
    {
        bottomBar->setActive(BottomBar::Side::CENTER, visible);
    }

    auto PhonebookNewContact::onInput(const InputEvent &inputEvent) -> bool
    {
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        if (!inputEvent.isShortRelease()) {
            return false;
        }

        if (inputEvent.is(gui::KeyCode::KEY_ENTER)) {
            auto tmpId  = contact->ID;
            contact     = std::make_shared<ContactRecord>();
            contact->ID = tmpId;

            newContactModel->saveData(contact);
            verifyAndSave();

            return true;
        }

        application->refreshWindow(RefreshModes::GUI_REFRESH_FAST);

        return false;
    }

    auto PhonebookNewContact::verifyAndSave() -> bool
    {
        LOG_DEBUG("%s", __FUNCTION__);
        if (!contact->isTemporary()) {
            auto result = DBServiceAPI::verifyContact(application, *contact);
            switch (result) {
            case DBServiceAPI::ContactVerificationResult::success:
                break;
            case DBServiceAPI::ContactVerificationResult::emptyContact:
                return false;
            case DBServiceAPI::ContactVerificationResult::primaryNumberDuplicate:
                showDialogDuplicatedNumber(contact->numbers[0].number);
                return false;
            case DBServiceAPI::ContactVerificationResult::secondaryNumberDuplicate:
                showDialogDuplicatedNumber(contact->numbers[1].number);
                return false;
            case DBServiceAPI::ContactVerificationResult::speedDialDuplicate:
                showDialogDuplicatedSpeedDialNumber();
                return false;
            case DBServiceAPI::ContactVerificationResult::temporaryContactExists:
                std::unique_ptr<ContactRecord> tempContact;
                assert(!contact->numbers.empty());
                for (auto number : contact->numbers) {
                    if (number.number.getEntered().size() > 0) {
                        tempContact = DBServiceAPI::MatchContactByPhoneNumber(application, number.number);
                        if (tempContact != nullptr) {
                            contact->ID   = tempContact->ID;
                            contactAction = ContactAction::EditTemporary;
                            break;
                        }
                    }
                }
                return false;
            }
        }
        else {
            contact->removeFromGroup(ContactsDB::temporaryGroupId());
        }

        // perform actual add/update operation
        if (contactAction == ContactAction::Add) {
            if (DBServiceAPI::ContactAdd(application, *contact) == false) {
                LOG_ERROR("verifyAndSave failed to ADD contact");
                return false;
            }
        }
        else if (contactAction == ContactAction::Edit || contactAction == ContactAction::EditTemporary) {
            std::unique_ptr<gui::SwitchData> data = std::make_unique<PhonebookItemData>(contact);
            data->ignoreCurrentWindowOnStack      = true;

            contact->groups.erase(ContactsDB::temporaryGroupId());

            if (DBServiceAPI::ContactUpdate(application, *contact) == false) {
                LOG_ERROR("verifyAndSave failed to UPDATE contact");
                return false;
            }
        }

        application->switchWindow(gui::name::window::main_window);
        return true;
    } // namespace gui

    void PhonebookNewContact::showDialogDuplicatedNumber(const utils::PhoneNumber::View &duplicatedNumber)
    {
        auto matchedContact   = DBServiceAPI::MatchContactByPhoneNumber(application, duplicatedNumber);
        auto oldContactRecord = (matchedContact != nullptr) ? *matchedContact : ContactRecord{};

        if (contactAction == ContactAction::Add) {
            contact->ID = oldContactRecord.ID;
        }

        std::string duplicatedNumberPhrase = utils::translate("app_phonebook_duplicate_numbers");
        phonebookUtils::fillContactData(duplicatedNumberPhrase, oldContactRecord);

        auto metaData = std::make_unique<gui::DialogMetadataMessage>(gui::DialogMetadata{
            duplicatedNumber.getFormatted(), "info_big_circle_W_G", duplicatedNumberPhrase, "", [=]() -> bool {
                if (!DBServiceAPI::ContactUpdate(application, *contact)) {
                    LOG_ERROR("Contact id=%" PRIu32 " update failed", contact->ID);
                    return false;
                }
                application->switchWindow(gui::name::window::main_window);
                return true;
            }});
        application->switchWindow(gui::window::name::dialog_yes_no, std::move(metaData));
    }

    void PhonebookNewContact::showDialogDuplicatedSpeedDialNumber()
    {
        auto contactRecordsPtr = DBServiceAPI::ContactGetBySpeeddial(application, contact->speeddial);
        auto oldContactRecord  = !contactRecordsPtr->empty() ? contactRecordsPtr->front() : ContactRecord{};

        if (contactAction == ContactAction::Add) {
            contact->ID = oldContactRecord.ID;
        }

        std::string duplicatedSpeedDialPhrase = utils::translate("app_phonebook_duplicate_numbers");
        phonebookUtils::fillContactData(duplicatedSpeedDialPhrase, oldContactRecord);
        std::string duplicatedSpeedDialTitle = utils::translate("app_phonebook_duplicate_speed_dial_title");
        phonebookUtils::fillContactData(duplicatedSpeedDialTitle, oldContactRecord);

        auto metaData = std::make_unique<gui::DialogMetadataMessage>(
            gui::DialogMetadata{duplicatedSpeedDialTitle,
                                "phonebook_empty_grey_circle_speed_dial",
                                duplicatedSpeedDialPhrase,
                                contact->speeddial,
                                [=]() -> bool {
                                    if (!DBServiceAPI::ContactUpdate(application, *contact)) {
                                        LOG_ERROR("Contact id=%" PRIu32 " update failed", contact->ID);
                                        return false;
                                    }
                                    application->switchWindow(gui::name::window::main_window);
                                    return true;
                                }});
        application->switchWindow(gui::window::name::dialog_yes_no_icon_txt, std::move(metaData));
    }

} // namespace gui
