// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "application-settings-new/ApplicationSettings.hpp"
#include "locks/data/LockStyle.hpp"
#include "ChangePasscodeWindow.hpp"
#include "DialogMetadata.hpp"
#include "DialogMetadataMessage.hpp"
#include "windows/Dialog.hpp"

namespace lock_style        = style::window::pin_lock;
namespace screen_lock_style = style::window::screen_pin_lock;

namespace gui
{
    ChangePasscodeWindow::ChangePasscodeWindow(app::Application *app)
        : LockWindow(app, gui::window::name::change_passcode)
    {
        buildInterface();
    }

    auto ChangePasscodeWindow::onInput(const InputEvent &inputEvent) -> bool
    {
        auto &lock = lockHandler.getLock();
        if (lock.isState(Lock::LockState::Unlocked) && inputEvent.isShortRelease()) {
            application->returnToPreviousWindow();
        }
        if (!inputEvent.isShortRelease()) {
            return AppWindow::onInput(inputEvent);
        }

        if (inputEvent.isDigit() && lock.canPut()) {
            lockBox->putChar(lock.getCharCount());
            lock.putNextChar(inputEvent.numericValue());
            return true;
        }
        else if (inputEvent.is(KeyCode::KEY_PND)) {
            lock.popChar();
            lockBox->popChar(lock.getCharCount());
            return true;
        }
        else if (inputEvent.is(KeyCode::KEY_ENTER)) {
            processPasscode();
            setVisibleState();
            return true;
        }
        return AppWindow::onInput(inputEvent);
    }

    void ChangePasscodeWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void ChangePasscodeWindow::buildBottomBar()
    {
        LockWindow::buildBottomBar();
        setBottomBarWidgetsActive(false, true, true);
    }

    void ChangePasscodeWindow::buildInterface()
    {
        AppWindow::buildInterface();
        LockWindow::build();

        lockBox = std::make_unique<PhoneLockBaseBox>(this);
        lockBox->buildLockBox(lockHandler.getLock().getMaxInputSize());

        lockImage = new gui::Image(this, lock_style::image::x, lock_style::image::y, 0, 0, "pin_lock");

        primaryText->setPosition(lockImage->getHeight() + screen_lock_style::primary_text::y, gui::Axis::Y);
        pinLabelsBox->setPosition(lockImage->getHeight() + screen_lock_style::pin_label::y, gui::Axis::Y);
    }

    void ChangePasscodeWindow::buildTitleBar()
    {
        setTitle(utils::translate("app_settings_security_change_passcode"));
    }

    void ChangePasscodeWindow::destroyInterface()
    {
        erase();
    }

    void ChangePasscodeWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        if (data != nullptr) {
            auto changePasscodeData = static_cast<ChangePasscodeData *>(data);
            changePasscodeAction    = changePasscodeData->changePasscodeAction;
            if (ChangePasscodeAction::OnlyProvideNewPasscode == changePasscodeAction) {
                lockState = Lock::LockState::NewInputRequired;
            }
        }
        setVisibleState();
    }

    void ChangePasscodeWindow::processPasscode()
    {
        switch (lockState) {
        case Lock::LockState::InputRequired:
        case Lock::LockState::InputInvalidRetryRequired: {
            auto app  = static_cast<app::ApplicationSettingsNew *>(application);
            lockState = lockHandler.checkPasscode(app->getLockPassHash());
            if (ChangePasscodeAction::OnlyCheckCurrentPasscode == changePasscodeAction &&
                lockState == Lock::LockState::NewInputRequired) {
                application->setLockScreenPasscodeOn(false);

                auto metaData = std::make_unique<gui::DialogMetadataMessage>(
                    gui::DialogMetadata{utils::translate("app_settings_security_change_passcode"),
                                        "success_icon_W_G",
                                        utils::translate("app_settings_security_passcode_disabled"),
                                        "",
                                        [this]() {
                                            application->switchWindow(gui::window::name::security);
                                            return true;
                                        }});
                application->switchWindow(
                    gui::window::name::dialog_confirm, gui::ShowMode::GUI_SHOW_INIT, std::move(metaData));
                return;
            }
            break;
        }
        case Lock::LockState::NewInputRequired:
        case Lock::LockState::NewInputInvalidRetryRequired: {
            lockState = lockHandler.newPasscodeProvided();
            break;
        }
        case Lock::LockState::NewInputConfirmRequired:
        case Lock::LockState::NewInputInvalid: {
            lockState = lockHandler.newPasscodeConfirmed();
            if (lockState == Lock::LockState::Unlocked) {
                auto app = static_cast<app::ApplicationSettingsNew *>(application);
                app->setLockPassHash(lockHandler.getNewPasscodeHash());
            }
            break;
        }
        default:
            break;
        }
    }

    void ChangePasscodeWindow::setVisibleState()
    {
        lockBox->clear();
        switch (lockState) {
        case Lock::LockState::InputRequired: {
            setText("app_settings_security_type_current_passcode", LockWindow::TextType::Primary);
            secondaryText->setVisible(false);
            break;
        }
        case Lock::LockState::NewInputRequired: {
            setText("app_settings_security_enter_new_passcode", LockWindow::TextType::Primary);
            secondaryText->setVisible(false);
            break;
        }
        case Lock::LockState::NewInputConfirmRequired: {
            setText("app_settings_security_confirm_new_passcode", LockWindow::TextType::Primary);
            secondaryText->setVisible(false);
            break;
        }
        case Lock::LockState::InputInvalidRetryRequired:
        case Lock::LockState::NewInputInvalidRetryRequired:
        case Lock::LockState::NewInputInvalid: {
            setText("app_settings_security_wrong_passcode", LockWindow::TextType::Secondary);
            break;
        }
        case Lock::LockState::Unlocked: {
            application->setLockScreenPasscodeOn(true);

            auto metaData = std::make_unique<gui::DialogMetadataMessage>(
                gui::DialogMetadata{utils::translate("app_settings_security_change_passcode"),
                                    "success_icon_W_G",
                                    utils::translate("app_settings_security_passcode_changed_successfully"),
                                    "",
                                    [this]() {
                                        application->switchWindow(gui::window::name::security);
                                        return true;
                                    }});
            application->switchWindow(
                gui::window::name::dialog_confirm, gui::ShowMode::GUI_SHOW_INIT, std::move(metaData));
            break;
        }
        default: {
            break;
        }
        }
    }
} // namespace gui
