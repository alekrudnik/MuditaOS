// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MenuWindow.hpp"
#include "../ApplicationDesktop.hpp"
#include "InputEvent.hpp"
#include "Item.hpp"
#include "Navigation.hpp"
#include "service-appmgr/Controller.hpp"

#include <tools/Common.hpp>
#include <Style.hpp>
#include <cassert>
#include <i18n/i18n.hpp>
#include <Image.hpp>

namespace style::design
{
    // all these might not be final - as designs for these didn't look like that

    inline const auto tile_w       = 130;
    inline const auto tile_h       = 130;
    inline const auto tile_margin  = 24 - (64 - 50) / 2; // this is waiting for proper adjust-to-design anyway
    inline const auto tile_text_y  = 50;
    inline const auto notify_dot_x = 80;
    inline const auto notify_dot_y = (64 - 50) / 2;
    inline const auto grid_offset  = 20;
} // namespace style::design

namespace
{
    static constexpr auto deepRefreshDot = "dot_12px_hard_alpha_W_G";
    static constexpr auto fastRefreshDot = "dot_12px_hard_alpha_W_M";

    gui::Image *buildThumbnail(gui::RefreshModes mode)
    {
        gui::Image *thumbnail =
            new gui::Image(mode == gui::RefreshModes::GUI_REFRESH_DEEP ? deepRefreshDot : fastRefreshDot);
        thumbnail->setPosition(style::design::notify_dot_x, style::design::notify_dot_y);
        return thumbnail;
    }
} // namespace

namespace gui
{
    inline const auto APP_SETTINGS_NEW = "ApplicationSettingsNew";
    Tile::Tile(UTF8 icon,
               std::string title,
               std::function<bool(Item &)> activatedCallback,
               std::function<bool()> hasNotificationsCallback)
    {
        setSize(style::design::tile_w, style::design::tile_h);

        auto it = new gui::Item();
        it->setSize(style::design::tile_w, style::design::tile_h - 2 * style::design::tile_margin);
        it->setPosition(area().x, area().y + style::design::tile_margin);

        auto *img = new gui::Image(icon);
        center(it, img, Axis::X);
        it->addWidget(img);

        auto *desc =
            new gui::Label(it, 0, it->area().h - style::design::tile_text_y, it->area().w, style::design::tile_text_y);
        desc->setPenWidth(style::window::default_border_no_focus_w);
        desc->setFont(style::window::font::verysmall);
        desc->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Bottom));
        desc->setText(utils::translate(title));

        if (hasNotificationsCallback != nullptr) {
            onNotificationsChangeCallback =
                [this, it, hasNotifications = std::move(hasNotificationsCallback)](gui::RefreshModes mode) -> bool {
                if (hasNotifications() && notificationThumbnail == nullptr) {
                    notificationThumbnail = buildThumbnail(mode);
                    it->addWidget(notificationThumbnail);
                    return true;
                }
                else if (!hasNotifications() && notificationThumbnail != nullptr) {
                    it->erase(notificationThumbnail);
                    notificationThumbnail = nullptr;
                }
                return false;
            };
            onNotificationsChangeCallback(gui::RefreshModes::GUI_REFRESH_DEEP);
        }

        this->activatedCallback = activatedCallback;
        this->setPenWidth(style::window::default_border_no_focus_w);
        this->setPenFocusWidth(style::window::default_border_focus_w);
        this->setEdges(RectangleEdge::Top | RectangleEdge::Bottom);
        addWidget(it);
    }
    bool Tile::onNotificationsChange(gui::RefreshModes mode)
    {
        if (onNotificationsChangeCallback != nullptr) {
            return onNotificationsChangeCallback(mode);
        }
        return false;
    }

    MenuPage::MenuPage(gui::Item *parent, UTF8 title, std::vector<Tile *> tiles) : title(std::move(title))
    {
        if (parent) {
            parent->addWidget(this);
        }
        grid.x = style::design::tile_w;
        grid.y = style::design::tile_h;
        for (auto &tile : tiles) {
            addWidget(tile);
        }
    }

    void MenuPage::setFirstTimeSelection()
    {
        if (visible && first_time_selection) {
            if (children.size() > first_time_selected) {
                setFocusItem(*std::next(children.begin(), first_time_selected));
                first_time_selection = false;
            }
        }
    }

    bool MenuPage::refresh(gui::RefreshModes mode)
    {
        bool visibleStateChanged = false;
        for (auto child : children) {
            if (auto tile = dynamic_cast<Tile *>(child); tile != nullptr) {
                visibleStateChanged |= tile->onNotificationsChange(mode);
            }
        }
        return visibleStateChanged;
    }

    MenuWindow::MenuWindow(app::Application *app) : AppWindow(app, app::window::name::desktop_menu)
    {
        buildInterface();
    }

    void MenuWindow::rebuild()
    {
        focusItem = nullptr;
        destroyInterface();
        buildInterface();
    }

    void MenuWindow::buildInterface()
    {
        AppWindow::buildInterface();
        bottomBar->setActive(BottomBar::Side::CENTER, true);
        bottomBar->setActive(BottomBar::Side::RIGHT, true);
        bottomBar->setText(BottomBar::Side::CENTER, utils::translate(style::strings::common::open));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::translate(style::strings::common::back));

        auto app = dynamic_cast<app::ApplicationDesktop *>(application);
        assert(app);

        mainMenu = new MenuPage(
            this,
            utils::translate("app_desktop_menu_title"),
            {

                new gui::Tile{
                    "menu_tools_W_G",
                    "app_desktop_menu_tools",
                    [=](gui::Item &item) {
                        {
                            switchMenu(toolsMenu);
                            return true;
                        }
                    },
                },
                new gui::Tile("menu_alarm_W_G",
                              "app_desktop_menu_alarm",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationAlarmClock"));
                              }),

                new gui::Tile("menu_calendar_W_G",
                              "app_desktop_menu_calendar",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationCalendar"));
                              }),

                new gui::Tile{"menu_phone_W_G",
                              "app_desktop_menu_phone",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationCallLog"));
                              },
                              [=]() { return app->notifications.notRead.Calls > 0; }},

                new gui::Tile("menu_contacts_W_G",
                              "app_desktop_menu_contacts",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationPhonebook"));
                              }),

                new gui::Tile{"menu_messages_W_G",
                              "app_desktop_menu_messages",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationMessages"));
                              },
                              [=]() { return app->notifications.notRead.SMS > 0; }},
                new gui::Tile{"menu_music_player_W_G",
                              "app_desktop_menu_music",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationMusicPlayer"));
                              }},
                new gui::Tile{"menu_meditation_W_G",
                              "app_desktop_menu_meditation",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationMeditation"));
                              }},
                new gui::Tile{"menu_settings_W_G", "app_desktop_menu_settings_new", [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>(APP_SETTINGS_NEW));
                              }}});

        toolsMenu = new MenuPage(
            this,
            utils::translate("app_desktop_tools_title"),
            {
                new gui::Tile{"menu_tools_notes_W_G",
                              "app_desktop_tools_notes",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationNotes"));
                              }},
                new gui::Tile{"menu_tools_calculator_W_G",
                              "app_desktop_tools_calculator",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationCalculator"));
                              }},
                new gui::Tile{"menu_tools_recorder_W_G",
                              "app_desktop_tools_antenna",
                              [=](gui::Item &item) {
                                  return app::manager::Controller::sendAction(
                                      application,
                                      app::manager::actions::Launch,
                                      std::make_unique<app::ApplicationLaunchData>("ApplicationAntenna"));
                              }},
            });

        using namespace style::window;
        mainMenu->setSize(this->area().w - default_left_margin - default_right_margin,
                          bottomBar->area().pos(Axis::Y) - this->title->getOffset(Axis::Y) -
                              style::design::grid_offset);
        mainMenu->setPosition(default_left_margin, title->getOffset(Axis::Y) + style::design::grid_offset);
        mainMenu->setVisible(false);

        toolsMenu->setSize(this->area().w - default_left_margin - default_right_margin,
                           bottomBar->area().pos(Axis::Y) - this->title->getOffset(Axis::Y) -
                               style::design::grid_offset);
        toolsMenu->setPosition(default_left_margin, title->getOffset(Axis::Y) + style::design::grid_offset);
        toolsMenu->setVisible(false);

        switchMenu(mainMenu);
        mainMenu->setFirstTimeSelection();
    }

    void MenuWindow::destroyInterface()
    {
        erase();
        invalidate();
    }

    void MenuWindow::invalidate() noexcept
    {
        mainMenu  = nullptr;
        toolsMenu = nullptr;
    }

    bool MenuWindow::onInput(const InputEvent &inputEvent)
    {
        if ((inputEvent.state == InputEvent::State::keyReleasedShort) && (inputEvent.keyCode == KeyCode::KEY_RF) &&
            (toolsMenu->visible)) {
            switchMenu(mainMenu);
            return true;
        }
        return AppWindow::onInput(inputEvent);
    }

    void MenuWindow::switchMenu(MenuPage *page)
    {
        if (toolsMenu != page) {
            toolsMenu->setVisible(false);
        }
        if (mainMenu != page) {
            mainMenu->setVisible(false);
        }

        setTitle(page->title);
        setFocusItem(page);
    }

    void MenuWindow::refresh()
    {
        if (application->getCurrentWindow() == this) {
            if (mainMenu->refresh(RefreshModes::GUI_REFRESH_FAST)) {
                application->refreshWindow(RefreshModes::GUI_REFRESH_FAST);
            }
        }
        else {
            mainMenu->refresh(RefreshModes::GUI_REFRESH_DEEP);
        }
    }
} /* namespace gui */
