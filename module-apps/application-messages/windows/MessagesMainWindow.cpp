#include "MessagesMainWindow.hpp"

#include "NewMessage.hpp"
#include "OptionsWindow.hpp"
#include "../ApplicationMessages.hpp"
#include "../MessagesStyle.hpp"
#include "../data/SMSdata.hpp"
#include "../widgets/ThreadItem.hpp"
#include "../windows/ThreadViewWindow.hpp"
#include "application-messages/windows/SearchStart.hpp"

#include <service-appmgr/ApplicationManager.hpp>
#include <service-db/messages/DBThreadMessage.hpp>
#include <i18/i18.hpp>
#include <Margins.hpp>
#include <service-db/api/DBServiceAPI.hpp>
#include <service-cellular/api/CellularServiceAPI.hpp>
#include <application-phonebook/data/PhonebookItemData.hpp>
#include <Style.hpp>
#include <log/log.hpp>
#include <time/time_conversion.hpp>
#include <module-db/queries/notifications/QueryNotificationsClear.hpp>

#include <functional>
#include <memory>
#include <cassert>
#include <module-services/service-db/messages/DBNotificationMessage.hpp>

namespace gui
{

    MessagesMainWindow::MessagesMainWindow(app::Application *app) : AppWindow(app, gui::name::window::main_window)
    {
        buildInterface();
    }

    void MessagesMainWindow::rebuild()
    {
        list->rebuildList();
    }

    void MessagesMainWindow::buildInterface()
    {

        namespace msgThreadStyle = style::messages::threads;

        AppWindow::buildInterface();

        threadModel = std::make_shared<ThreadModel>(this->application);

        list = new gui::ListView(this,
                                 msgThreadStyle::listPositionX,
                                 msgThreadStyle::ListPositionY,
                                 msgThreadStyle::listWidth,
                                 msgThreadStyle::listHeight,
                                 threadModel);
        list->setScrollTopMargin(style::margins::small);
        list->setPenFocusWidth(0);
        list->setPenWidth(0);

        bottomBar->setActive(BottomBar::Side::LEFT, true);
        bottomBar->setActive(BottomBar::Side::CENTER, true);
        bottomBar->setActive(BottomBar::Side::RIGHT, true);
        bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get(style::strings::common::options));
        bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get(style::strings::common::open));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));

        topBar->setActive(TopBar::Elements::TIME, true);

        setTitle(utils::localize.get("app_messages_title_main"));

        leftArrowImage  = new gui::Image(this, 30, 62, 0, 0, "arrow_left");
        rightArrowImage = new gui::Image(this, 480 - 30 - 13, 62, 0, 0, "arrow_right");
        newMessageImage = new gui::Image(this, 48, 55, 0, 0, "cross");
        searchImage     = new gui::Image(this, 480 - 48 - 26, 55, 0, 0, "search");

        emptyListIcon = new Icon(this,
                                 0,
                                 style::header::height,
                                 style::window_width,
                                 style::window_height - style::header::height - style::footer::height,
                                 "phonebook_empty_grey_circle_W_G",
                                 utils::localize.get("app_messages_no_messages"));

        list->setVisible(true);
        list->focusChangedCallback = [=](gui::Item & /*item*/) {
            bottomBar->setActive(BottomBar::Side::LEFT, true);
            bottomBar->setActive(BottomBar::Side::CENTER, true);
            rightArrowImage->setVisible(true);
            searchImage->setVisible(true);
            return true;
        };

        setFocusItem(list);

        emptyListIcon->setVisible(false);
        emptyListIcon->focusChangedCallback = [=](gui::Item & /*item*/) {
            bottomBar->setActive(BottomBar::Side::LEFT, false);
            bottomBar->setActive(BottomBar::Side::CENTER, false);
            rightArrowImage->setVisible(false);
            searchImage->setVisible(false);
            return true;
        };
    }

    void MessagesMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {

        LOG_INFO("Data: %s", data ? data->getDescription().c_str() : "");
        {
            if (auto pdata = dynamic_cast<PhonebookSearchReuqest *>(data)) {
                auto thread = DBServiceAPI::ThreadGetByContact(application, pdata->result->ID);
                if (thread) {
                    application->switchWindow(gui::name::window::thread_view,
                                              std::make_unique<SMSThreadData>(std::move(thread)));
                }
                else {
                    LOG_FATAL("No thread and thread not created!");
                }
            }
        }

        if (mode == ShowMode::GUI_SHOW_INIT || data == nullptr) {
            rebuild();
        }

        if (threadModel->requestRecordsCount() == 0) {
            emptyListIcon->setVisible(true);
            setFocusItem(emptyListIcon);
        }
        else {
            emptyListIcon->setVisible(false);
        }

        DBServiceAPI::GetQuery(application,
                               db::Interface::Name::Notifications,
                               std::make_unique<db::query::notifications::Clear>(NotificationsRecord::Key::Sms));
    }

    bool MessagesMainWindow::onInput(const InputEvent &inputEvent)
    {
        auto app = dynamic_cast<app::ApplicationMessages *>(application);
        assert(app);
        // check if any of the lower inheritance onInput methods catch the event
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }
        else {
            if (inputEvent.state == InputEvent::State::keyReleasedShort) {
                switch (inputEvent.keyCode) {
                case gui::KeyCode::KEY_LEFT:
                    application->switchWindow(gui::name::window::new_sms, nullptr);
                    return true;
                case gui::KeyCode::KEY_RIGHT: {
                    app->switchWindow(gui::name::window::thread_sms_search, nullptr);
                    return true;
                } break;
                default:
                    LOG_DEBUG("SMS main window not handled key: %d", static_cast<int>(inputEvent.keyCode));
                    break;
                }
            }
        }
        return false;
    }

    bool MessagesMainWindow::onDatabaseMessage(sys::Message *msgl)
    {
        auto *msgResponse = dynamic_cast<DBThreadResponseMessage *>(msgl);
        if (msgResponse &&
            threadModel->updateRecords(
                std::move(msgResponse->records), msgResponse->offset, msgResponse->limit, msgResponse->count)) {
            return true;
        }

        auto *msgNotification = dynamic_cast<db::NotificationMessage *>(msgl);
        if (msgNotification != nullptr) {
            // whatever notification had happened, rebuild
            if (msgNotification->interface == db::Interface::Name::SMSThread ||
                msgNotification->interface == db::Interface::Name::SMS) {

                if (msgNotification->type == db::Query::Type::Delete ||
                    msgNotification->type == db::Query::Type::Update ||
                    msgNotification->type == db::Query::Type::Create) {

                    LOG_INFO("I CO MAMY type? %d", (int)msgNotification->type);
                    LOG_INFO("I CO MAMY intefejs? %d", (int)msgNotification->interface);

                    rebuild();
                    if (this == application->getCurrentWindow()) {
                        application->refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
                    }
                    return true;
                }
            }
        }
        return false;
    } // namespace gui

} /* namespace gui */
