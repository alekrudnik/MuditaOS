#include "UiCommonActions.hpp"
#include "application-call/ApplicationCall.hpp"
#include "application-call/data/CallSwitchData.hpp"
#include "application-messages/ApplicationMessages.hpp"
#include "application-messages/data/SMSdata.hpp"
#include "application-messages/windows/ThreadViewWindow.hpp"
#include "application-phonebook/ApplicationPhonebook.hpp"
#include "application-phonebook/data/PhonebookItemData.hpp"
#include "service-appmgr/ApplicationManager.hpp"

#include <i18/i18.hpp>
#include <log/log.hpp>
#include <PhoneNumber.hpp>

#include <cassert>
#include <string>
#include <utility>
#include <vector>

namespace app
{
    bool call(Application *app, const ContactRecord &contact)
    {
        assert(app != nullptr);

        if (contact.numbers.size() != 0) {
            return call(app, std::string(contact.numbers[0].numberE164));
        }
        else {
            LOG_ERROR("No contact numbers!");
            return false;
        }
    }

    bool call(Application *app, const utils::PhoneNumber::View &phoneNumber)
    {
        assert(app != nullptr);
        auto data = std::make_unique<ExecuteCallData>(phoneNumber);

        return sapm::ApplicationManager::messageSwitchApplication(
            app, name_call, window::name_enterNumber, std::move(data));
    }

    bool call(Application *app, const std::string &e164number)
    {
        return call(app, utils::PhoneNumber::parse(e164number));
    }

    bool prepare_call(Application *app, const std::string &number)
    {
        assert(app != nullptr);
        auto data = std::make_unique<EnterNumberData>(number);

        return sapm::ApplicationManager::messageSwitchApplication(
            app, name_call, window::name_enterNumber, std::move(data));
    }

    bool sms(Application *app, SmsOperation smsOperation, const utils::PhoneNumber::View &number)
    {
        assert(app != nullptr);

        switch (smsOperation) {
        case SmsOperation::New: {
            return sapm::ApplicationManager::messageSwitchApplication(
                app, name_messages, gui::name::window::new_sms, std::make_unique<SMSSendRequest>(number));
        }
        case SmsOperation::Template: {
            return sapm::ApplicationManager::messageSwitchApplication(
                app, name_messages, gui::name::window::sms_templates, std::make_unique<SMSSendTemplateRequest>(number));
        }
        default: {
            LOG_ERROR("SmsOperation not supported %" PRIu32, static_cast<uint32_t>(smsOperation));
            return false;
        }
        }
    }

    bool contact(Application *app, ContactOperation contactOperation, const ContactRecord &contact)
    {
        assert(app != nullptr);
        switch (contactOperation) {
        case ContactOperation::Add: {
            return sapm::ApplicationManager::messageSwitchApplication(
                app,
                name_phonebook,
                gui::window::name::new_contact,
                std::make_unique<PhonebookItemData>(std::shared_ptr<ContactRecord>(new ContactRecord(contact))));
        } break;
        case ContactOperation::Details: {
            return sapm::ApplicationManager::messageSwitchApplication(
                app,
                name_phonebook,
                gui::window::name::contact,
                std::make_unique<PhonebookItemData>(std::shared_ptr<ContactRecord>(new ContactRecord(contact))));
        } break;
        case ContactOperation::Edit: {
            return sapm::ApplicationManager::messageSwitchApplication(
                app,
                name_phonebook,
                gui::window::name::new_contact, // TODO: need to be fixed when contact edition is working
                std::make_unique<PhonebookItemData>(std::shared_ptr<ContactRecord>(new ContactRecord(contact))));
        } break;
        default: {
            LOG_ERROR("ContactOperation not supported %" PRIu32, static_cast<uint32_t>(contactOperation));
            return false;
        }
        }
    }

    bool contact(Application *app, ContactOperation contactOperation, const std::string &number)
    {
        assert(app != nullptr);

        auto searchResults = DBServiceAPI::ContactSearch(app, "", "", number);
        ContactRecord contactRec;
        if (searchResults.get()->size() == 1) {
            contactRec = searchResults->front();
            LOG_INFO("Found contact matching search num %s : contact ID %" PRIu32 " - %s %s",
                     number.c_str(),
                     contactRec.ID,
                     contactRec.primaryName.c_str(),
                     contactRec.alternativeName.c_str());

            if (contactOperation == ContactOperation::Add) {
                return sapm::ApplicationManager::messageSwitchApplication(
                    app,
                    name_phonebook,
                    gui::window::name::new_contact,
                    std::make_unique<PhonebookItemData>(std::make_shared<ContactRecord>(contactRec)));
            }
        }
        else if (searchResults.get()->size() > 1) {
            LOG_FATAL("Found more than one contact for number %s", number.c_str());
            for (auto i : *searchResults) {
                LOG_FATAL("ContactID = %" PRIu32, i.ID);
            }
            return false;
        }
        else if (contactOperation != ContactOperation::Add) {
            LOG_ERROR("Invalid operation for not existing contact for number %s", number.c_str());
            return false;
        }

        return contact(app, contactOperation, contactRec);
    }

    bool contact(Application *app, ContactOperation contactOperation, uint32_t contactId)
    {
        assert(app != nullptr);
        assert(contactOperation != ContactOperation::Add);

        auto searchResults = DBServiceAPI::ContactGetByID(app, contactId);
        if (searchResults.get()->size() == 1) {
            ContactRecord contactRec = searchResults->front();
            return contact(app, contactOperation, contactRec);
        }

        return false;
    }

} // namespace app
