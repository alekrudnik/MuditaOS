﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CallLogMainWindow.hpp"
#include "data/CallLogInternals.hpp"
#include "ApplicationCallLog.hpp"
#include "widgets/CalllogItem.hpp"

#include <service-db/DBCalllogMessage.hpp>
#include <i18n/i18n.hpp>
#include <Label.hpp>
#include <Margins.hpp>
#include <Style.hpp>
#include <InputEvent.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <service-db/DBNotificationMessage.hpp>

using namespace style;
using namespace callLogStyle;

namespace gui
{

    CallLogMainWindow::CallLogMainWindow(app::Application *app)
        : AppWindow(app, calllog::settings::MainWindowStr), calllogModel{std::make_shared<CalllogModel>(app)}
    {

        buildInterface();
    }

    void CallLogMainWindow::rebuild()
    {
        if (list == nullptr) {
            return;
        }
        list->rebuildList(gui::listview::RebuildType::InPlace);
    }

    void CallLogMainWindow::buildInterface()
    {
        AppWindow::buildInterface();

        setTitle(utils::translate("app_calllog_title_main"));

        bottomBar->setText(BottomBar::Side::LEFT, utils::translate(style::strings::common::call));
        bottomBar->setText(BottomBar::Side::CENTER, utils::translate(style::strings::common::open));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::translate(style::strings::common::back));

        list = new gui::ListView(this,
                                 mainWindow::x,
                                 mainWindow::y,
                                 mainWindow::w,
                                 mainWindow::h,
                                 calllogModel,
                                 gui::listview::ScrollBarType::Fixed);

        setFocusItem(list);

        emptyListIcon =
            new gui::Icon(this,
                          0,
                          ::style::window::default_vertical_pos,
                          ::style::window_width,
                          ::style::window_height - ::style::window::default_vertical_pos - ::style::footer::height,
                          "info_icon_W_G",
                          utils::translate("app_calllog_no_calls"));

        list->emptyListCallback    = [this]() { onEmptyList(); };
        list->notEmptyListCallback = [this]() { onListFilled(); };
    }

    void CallLogMainWindow::destroyInterface()
    {
        erase();
    }

    void CallLogMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        if (mode == ShowMode::GUI_SHOW_INIT) {
            list->rebuildList();
        }
        auto app = dynamic_cast<app::ApplicationCallLog *>(application);
        assert(app != nullptr);
        app->setAllEntriesRead();
    }

    bool CallLogMainWindow::onDatabaseMessage(sys::Message *msg)
    {
        auto notification = dynamic_cast<db::NotificationMessage *>(msg);
        if (notification != nullptr) {
            if (notification->interface == db::Interface::Name::Calllog && notification->dataModified()) {
                rebuild();
                return true;
            }
        }
        return false;
    }

    void CallLogMainWindow::onEmptyList()
    {
        bottomBar->setActive(gui::BottomBar::Side::LEFT, false);
        bottomBar->setActive(gui::BottomBar::Side::CENTER, false);
        emptyListIcon->setVisible(true);
    }

    void CallLogMainWindow::onListFilled()
    {
        bottomBar->setActive(gui::BottomBar::Side::LEFT, true);
        bottomBar->setActive(gui::BottomBar::Side::CENTER, true);
        emptyListIcon->setVisible(false);
    }
} /* namespace gui */
