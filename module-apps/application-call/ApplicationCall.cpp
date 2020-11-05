﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ApplicationCall.hpp"

#include "DialogMetadata.hpp"
#include "DialogMetadataMessage.hpp"
#include "data/CallSwitchData.hpp"
#include "windows/CallMainWindow.hpp"
#include "windows/CallWindow.hpp"
#include "windows/EmergencyCallWindow.hpp"
#include "windows/EnterNumberWindow.hpp"

#include <Application.hpp>
#include <MessageType.hpp>
#include <PhoneNumber.hpp>
#include <Dialog.hpp>
#include <log/log.hpp>
#include <memory>
#include <service-cellular/ServiceCellular.hpp>
#include <service-cellular/CellularServiceAPI.hpp>
#include <service-audio/AudioServiceAPI.hpp>
#include <service-appmgr/Controller.hpp>
#include <time/time_conversion.hpp>
#include <ticks.hpp>

#include <cassert>
#include <module-apps/application-phonebook/data/PhonebookItemData.hpp>
#include <module-services/service-db/service-db/DBServiceAPI.hpp>

namespace app
{
    ApplicationCall::ApplicationCall(std::string name, std::string parent, StartInBackground startInBackground)
        : Application(name, parent, startInBackground, app::call_stack_size)
    {
        using namespace gui::top_bar;
        topBarManager->enableIndicators({Indicator::Signal,
                                         Indicator::Time,
                                         Indicator::Battery,
                                         Indicator::SimCard,
                                         Indicator::NetworkAccessTechnology});
        addActionReceiver(manager::actions::Call, [this](auto &&data) {
            switchWindow(window::name_call, std::forward<decltype(data)>(data));
            return msgHandled();
        });
        addActionReceiver(manager::actions::Dial, [this](auto &&data) {
            switchWindow(window::name_enterNumber, std::forward<decltype(data)>(data));
            return msgHandled();
        });
    }

    //  number of seconds after end call to switch back to previous application
    constexpr inline auto delayToSwitchToPreviousApp = utils::time::Duration(std::chrono::seconds(3));

    void ApplicationCall::CallAbortHandler()
    {
        manager::Controller::sendAction(this, manager::actions::Call, std::make_unique<app::CallAbortData>());
    }

    void ApplicationCall::CallActiveHandler()
    {
        manager::Controller::sendAction(this, manager::actions::Call, std::make_unique<app::CallActiveData>());
    }

    void ApplicationCall::IncomingCallHandler(const CellularCallMessage *const msg)
    {
        if (state == call::State::IDLE) {
            manager::Controller::sendAction(
                this, manager::actions::Call, std::make_unique<app::IncomingCallData>(msg->number));
        }
    }

    void ApplicationCall::RingingHandler(const CellularCallMessage *const msg)
    {
        manager::Controller::sendAction(
            this, manager::actions::Call, std::make_unique<app::ExecuteCallData>(msg->number));
    }

    // Invoked upon receiving data message
    sys::MessagePointer ApplicationCall::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
    {

        auto retMsg = Application::DataReceivedHandler(msgl);
        // if message was handled by application's template there is no need to process further.
        auto response = dynamic_cast<sys::ResponseMessage *>(retMsg.get());
        assert(response);
        if (response->retCode == sys::ReturnCodes::Success) {
            return retMsg;
        }

        if (msgl->messageType == MessageType::CellularNotification) {
            auto *msg = dynamic_cast<CellularNotificationMessage *>(msgl);
            assert(msg != nullptr);

            switch (msg->type) {
            case CellularNotificationMessage::Type::CallAborted: {
                CallAbortHandler();
            } break;
            case CellularNotificationMessage::Type::CallActive: {
                CallActiveHandler();
            } break;
            default:
                break;
            }

            return std::make_shared<sys::ResponseMessage>();
        }
        else if (msgl->messageType == MessageType::CellularCall) {
            auto *msg = dynamic_cast<CellularCallMessage *>(msgl);
            assert(msg != nullptr);

            switch (msg->type) {
            case CellularCallMessage::Type::Ringing: {
                RingingHandler(msg);
            } break;
            case CellularCallMessage::Type::IncomingCall: {
                IncomingCallHandler(msg);
            } break;
            }
        }

        if (resp != nullptr) {
            switch (resp->responseTo) {
            case MessageType::CellularHangupCall: {
                if (resp->retCode == sys::ReturnCodes::Success) {
                    CallAbortHandler();
                }
                break;
            }
            default:
                break;
            }

            return std::make_shared<sys::ResponseMessage>();
        }

        return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
    }

    // Invoked during initialization
    sys::ReturnCodes ApplicationCall::InitHandler()
    {

        auto ret = Application::InitHandler();
        if (ret != sys::ReturnCodes::Success) {
            return ret;
        }

        createUserInterface();

        setActiveWindow(gui::name::window::main_window);

        return ret;
    }

    sys::ReturnCodes ApplicationCall::DeinitHandler()
    {
        return sys::ReturnCodes::Success;
    }

    void ApplicationCall::createUserInterface()
    {
        windowsFactory.attach(gui::name::window::main_window, [](Application *app, const std::string name) {
            return std::make_unique<gui::CallMainWindow>(app);
        });
        windowsFactory.attach(app::window::name_enterNumber, [](Application *app, const std::string newname) {
            return std::make_unique<gui::EnterNumberWindow>(app, static_cast<ApplicationCall *>(app));
        });
        windowsFactory.attach(app::window::name_call, [](Application *app, const std::string &name) {
            return std::make_unique<gui::CallWindow>(app, static_cast<ApplicationCall *>(app));
        });
        windowsFactory.attach(app::window::name_emergencyCall, [](Application *app, const std::string &name) {
            return std::make_unique<gui::EmergencyCallWindow>(app, static_cast<ApplicationCall *>(app));
        });
        windowsFactory.attach(app::window::name_dialogConfirm, [](Application *app, const std::string &name) {
            return std::make_unique<gui::DialogConfirm>(app, name);
        });
    }

    bool ApplicationCall::showNotification(std::function<bool()> action)
    {
        gui::DialogMetadata meta;
        meta.icon   = "info_big_circle_W_G";
        meta.text   = utils::localize.get("app_call_no_sim");
        meta.action = action;
        switchWindow(app::window::name_dialogConfirm, std::make_unique<gui::DialogMetadataMessage>(meta));
        return true;
    }

    void ApplicationCall::destroyUserInterface()
    {}

    void ApplicationCall::handleCallEvent(const std::string &number)
    {
        if (!Store::GSM::get()->simCardInserted()) {
            LOG_INFO("No SIM card");
            auto action = [=]() -> bool {
                returnToPreviousWindow();
                return true;
            };
            showNotification(action);
            return;
        }

        LOG_INFO("number: [%s]", number.c_str());
        auto ret = CellularServiceAPI::DialNumber(this, utils::PhoneNumber(number));
        LOG_INFO("CALL RESULT: %s", (ret ? "OK" : "FAIL"));
    }

    void ApplicationCall::handleAddContactEvent(const std::string &number)
    {
        LOG_INFO("add contact information: %s", number.c_str());

        auto searchResults = DBServiceAPI::ContactSearch(this, UTF8{}, UTF8{}, number);
        if (const auto resultsSize = searchResults->size(); resultsSize > 1) {
            LOG_FATAL("Found more than one contact for number %s", number.c_str());
            for (auto i : *searchResults) {
                LOG_FATAL("ContactID = %" PRIu32, i.ID);
            }
        }
        else if (resultsSize == 1) {
            const auto &contactRecord = searchResults->front();
            LOG_INFO("Found contact matching search num %s : contact ID %" PRIu32 " - %s %s",
                     number.c_str(),
                     contactRecord.ID,
                     contactRecord.primaryName.c_str(),
                     contactRecord.alternativeName.c_str());
            app::manager::Controller::sendAction(
                this,
                app::manager::actions::AddContact,
                std::make_unique<PhonebookItemData>(std::make_shared<ContactRecord>(contactRecord)));
        }
        else {
            ContactRecord contactRecord;
            contactRecord.numbers.emplace_back(ContactRecord::Number(utils::PhoneNumber(number).getView()));

            auto data = std::make_unique<PhonebookItemData>(std::make_shared<ContactRecord>(contactRecord));
            data->ignoreCurrentWindowOnStack = true;
            app::manager::Controller::sendAction(
                this, manager::actions::AddContact, std::move(data), manager::OnSwitchBehaviour::RunInBackground);
        }
    }

    void ApplicationCall::transmitDtmfTone(uint32_t digit)
    {
        if (!CellularServiceAPI::TransmitDtmfTones(this, digit)) {
            LOG_ERROR("transmitDtmfTone failed");
        }
    }

    void ApplicationCall::stopAudio()
    {
        AudioServiceAPI::StopAll(this);
    }

    void ApplicationCall::startRinging()
    {
        AudioServiceAPI::PlaybackStart(this, audio::PlaybackType::CallRingtone, ringtone_path);
    }

    void ApplicationCall::startRouting()
    {
        AudioServiceAPI::RoutingStart(this);
    }
} // namespace app
