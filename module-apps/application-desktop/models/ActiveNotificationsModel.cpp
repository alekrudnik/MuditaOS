// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ActiveNotificationsModel.hpp"
#include <application-desktop/ApplicationDesktop.hpp>
#include <module-db/queries/notifications/QueryNotificationsClear.hpp>
#include <service-appmgr/Controller.hpp>
#include <application-call/data/CallSwitchData.hpp>
#include <queries/messages/threads/QueryThreadGetByNumber.hpp>
#include <application-messages/data/SMSdata.hpp>
#include <application-messages/Constants.hpp>
#include <service-appmgr/messages/SwitchRequest.hpp>

#include <gsl/assert>

using namespace gui;

namespace
{
    using Notification = const notifications::NotificationWithContact *;
    void setSMSFocusChangedCallback(NotificationListItem *item, ActiveNotificationsModel *model)
    {
        item->focusChangedCallback = [model](gui::Item &_item) {
            if (_item.focus) {
                model->setParentBottomBar(
                    {}, utils::translate("app_desktop_show"), utils::translate("app_desktop_clear"));
                return true;
            }
            return false;
        };
    }

    auto createSMSActivatedCallback(app::Application *app)
    {
        return [app]([[maybe_unused]] gui::Item &_item) {
            return app::manager::Controller::sendAction(
                app, app::manager::actions::Launch, std::make_unique<app::ApplicationLaunchData>(app::name_messages));
        };
    }

    auto createSMSActivatedCallback(app::Application *app, const ContactRecord &record)
    {
        Expects(not record.numbers.empty());
        return [app, number = record.numbers[0].number]([[maybe_unused]] gui::Item &_item) {
            auto query = std::make_unique<db::query::ThreadGetByNumber>(number);
            auto task  = app::AsyncQuery::createFromQuery(std::move(query), db::Interface::Name::SMSThread);

            auto queryCallback = [app](db::QueryResult *msg) -> bool {
                Expects(typeid(*msg) == typeid(db::query::ThreadGetByNumberResult));
                auto result  = static_cast<db::query::ThreadGetByNumberResult *>(msg);
                auto data    = std::make_unique<SMSThreadData>(std::make_shared<ThreadRecord>(result->getThread()));
                auto request = std::make_shared<app::manager::SwitchRequest>(
                    app->GetName(), app::name_messages, gui::name::window::thread_view, std::move(data));
                return app->bus.sendUnicast(std::move(request), app::manager::ApplicationManager::ServiceName);
            };
            task->setCallback(std::move(queryCallback));
            task->execute(app, static_cast<app::ApplicationDesktop *>(app));
            return true;
        };
    }

    void setSMSActivatedCallback(NotificationListItem *item, Notification provider, app::Application *app)
    {
        if (provider->hasRecord() && not provider->getRecord().numbers.empty()) {
            item->activatedCallback = createSMSActivatedCallback(app, provider->getRecord());
        }
        else {
            item->activatedCallback = createSMSActivatedCallback(app);
        }
    }

    void setSMSOnInputCallback(NotificationListItem *item, app::Application *app)
    {
        item->inputCallback = [app]([[maybe_unused]] Item &item, const InputEvent &inputEvent) {
            if (inputEvent.isShortRelease(KeyCode::KEY_RF)) {
                DBServiceAPI::GetQuery(
                    app,
                    db::Interface::Name::Notifications,
                    std::make_unique<db::query::notifications::Clear>(NotificationsRecord::Key::Sms));
                return true;
            }
            return false;
        };
    }

    void setCallFocusChangedCallback(NotificationListItem *item, Notification provider, ActiveNotificationsModel *model)
    {
        item->focusChangedCallback = [model, canCall = provider->hasRecord()](gui::Item &_item) {
            if (_item.focus) {
                UTF8 bottomBarLeftText = canCall ? UTF8{utils::translate("common_call")} : UTF8{};
                model->setParentBottomBar(
                    bottomBarLeftText, utils::translate("app_desktop_show"), utils::translate("app_desktop_clear"));
            }
            return true;
        };
    }

    void setCallActivatedCallback(NotificationListItem *item, app::Application *app)
    {
        item->activatedCallback = [app]([[maybe_unused]] gui::Item &_item) {
            return app::manager::Controller::sendAction(app, app::manager::actions::ShowCallLog);
        };
    }

    auto createCallOnRightFunctionalCallback(app::Application *app) -> std::function<void()>
    {
        return [app]() {
            DBServiceAPI::GetQuery(app,
                                   db::Interface::Name::Notifications,
                                   std::make_unique<db::query::notifications::Clear>(NotificationsRecord::Key::Calls));
        };
    }
    auto createCallOnLeftFunctionalCallback(app::Application *app, Notification provider) -> std::function<void()>
    {
        if (!provider->hasRecord()) {
            return nullptr;
        }
        if (const auto &record = provider->getRecord(); !record.numbers.empty()) {
            return [app, number = record.numbers[0].number]() {
                app::manager::Controller::sendAction(
                    app, app::manager::actions::Call, std::make_unique<app::ExecuteCallData>(number));
            };
        }
        return nullptr;
    }

    void setCallOnInputCallback(NotificationListItem *item, Notification provider, app::Application *app)
    {
        auto onRightFunctionalKeyCallback = createCallOnRightFunctionalCallback(app);
        auto onLeftFunctionalKeyCallback  = createCallOnLeftFunctionalCallback(app, provider);

        item->inputCallback = [keyRightFunctionalCb = std::move(onRightFunctionalKeyCallback),
                               keyLeftFunctionalCb  = std::move(onLeftFunctionalKeyCallback)](
                                  [[maybe_unused]] Item &item, const InputEvent &inputEvent) {
            if (inputEvent.isShortRelease()) {
                if (inputEvent.is(KeyCode::KEY_RF) && keyRightFunctionalCb != nullptr) {
                    keyRightFunctionalCb();
                    return true;
                }
                else if (inputEvent.is(KeyCode::KEY_LF) && keyLeftFunctionalCb != nullptr) {
                    keyLeftFunctionalCb();
                    return true;
                }
            }
            return false;
        };
    }

    void setTetheringActivatedCallback(NotificationListItem *item, app::Application *app)
    {
        item->activatedCallback = [app]([[maybe_unused]] gui::Item &_item) {
            app->switchWindow(gui::popup::window::tethering_off_window);
            return true;
        };
    }

    void setTetheringFocusChangedCallback(NotificationListItem *item, ActiveNotificationsModel *model)
    {
        item->focusChangedCallback = [model](gui::Item &_item) {
            if (_item.focus) {
                model->setParentBottomBar({}, utils::translate("common_disconnect"), {});
                return true;
            }
            return false;
        };
    }
} // namespace

ActiveNotificationsModel::ActiveNotificationsModel(AppWindow *parent) : parent(parent)
{}

void ActiveNotificationsModel::setParentBottomBar(const UTF8 &left, const UTF8 &center, const UTF8 &right)
{
    parent->setBottomBarText(left, BottomBar::Side::LEFT);
    parent->setBottomBarText(center, BottomBar::Side::CENTER);
    parent->setBottomBarText(right, BottomBar::Side::RIGHT);
}

auto ActiveNotificationsModel::create(const notifications::NotSeenSMSNotification *notification)
    -> NotificationListItem *
{
    auto item                  = NotificationsModel::create(notification);
    setSMSFocusChangedCallback(item, this);
    setSMSActivatedCallback(item, notification, parent->getApplication());
    setSMSOnInputCallback(item, parent->getApplication());
    item->setDismissible(true);
    return item;
}
auto ActiveNotificationsModel::create(const notifications::NotSeenCallNotification *notification)
    -> NotificationListItem *
{
    auto item = NotificationsModel::create(notification);
    setCallFocusChangedCallback(item, notification, this);
    setCallActivatedCallback(item, parent->getApplication());
    setCallOnInputCallback(item, notification, parent->getApplication());
    item->setDismissible(true);
    return item;
}

auto ActiveNotificationsModel::create(const notifications::TetheringNotification *notification)
    -> NotificationListItem *
{
    auto item               = NotificationsModel::create(notification);
    setTetheringActivatedCallback(item, parent->getApplication());
    setTetheringFocusChangedCallback(item, this);
    return item;
}
