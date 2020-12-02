﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MmiPullWindow.hpp"
#include "application-desktop/widgets/DesktopInputWidget.hpp"
#include <service-appmgr/model/ApplicationManager.hpp>
#include <service-appmgr/data/MmiActionsParams.hpp>

#include <i18n/i18n.hpp>
#include <string>

using namespace gui;

// move to style
namespace style::desktop
{
    namespace text
    {
        constexpr uint32_t x = 30;
        constexpr uint32_t y = 120;
        constexpr uint32_t w = 440;
        constexpr uint32_t h = 320;
    } // namespace text

    namespace inputWidget
    {
        constexpr uint32_t x = 30;
        constexpr uint32_t y = 450;
        constexpr uint32_t w = 450;
        constexpr uint32_t h = 50;
    } // namespace inputWidget

} // namespace style::desktop

MmiPullWindow::MmiPullWindow(app::Application *app, const std::string &name) : gui::AppWindow(app, name)
{
    AppWindow::buildInterface();

    topBar->setActive(TopBar::Elements::TIME, true);
    topBar->setActive(TopBar::Elements::SIM, false);
    bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("app_desktop_replay"));
    bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
    text = new Text(
        this, style::desktop::text::x, style::desktop::text::y, style::desktop::text::w, style::desktop::text::h);
    text->setTextType(TextType::MULTI_LINE);
    text->setEditMode(EditMode::BROWSE);
    text->setEdges(RectangleEdge::None);
    text->setFont(style::window::font::medium);
    text->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Top));
    setTitle(utils::localize.get("app_desktop_info"));

    InputBox = new DesktopInputWidget(application,
                                      this,
                                      style::desktop::inputWidget::x,
                                      style::desktop::inputWidget::y,
                                      style::desktop::inputWidget::w,
                                      style::desktop::inputWidget::h);

    InputBox->inputText->setInputMode(new InputMode(
        {InputMode::digit},
        [=](const UTF8 &text1) { bottomBarTemporaryMode(text1); },
        [=]() { bottomBarRestoreFromTemporaryMode(); },
        [=]() { selectSpecialCharacter(); }));

    addWidget(InputBox);
}

std::string MmiPullWindow::removePhrase(std::string str, std::string phrase)
{
    auto find_pos = str.find(phrase);
    while (find_pos != std::string::npos) {
        str.replace(find_pos, phrase.size(), "");
        find_pos = str.find(phrase, find_pos);
    }
    return str;
}

void MmiPullWindow::onBeforeShow(ShowMode mode, SwitchData *data)
{
    auto metadata = dynamic_cast<app::manager::actions::MMIParams *>(data);
    if (metadata != nullptr) {
        text->setText(removePhrase(metadata->getData(), "\r"));
    }
    InputBox->setVisible(true);
    setFocusItem(InputBox->inputText);
}

bool MmiPullWindow::onInput(const InputEvent &inputEvent)
{
    return AppWindow::onInput(inputEvent);
}

void MmiPullWindow::destroyInterface()
{
    erase();
}

MmiPullWindow::~MmiPullWindow()
{
    destroyInterface();
}
