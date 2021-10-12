// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AppWindow.hpp"
#include "ApplicationCommon.hpp"
#include "data/AlarmPopupRequestParams.hpp"
#include "service-db/Settings.hpp"
#include "service-db/agents/settings/SystemSettings.hpp"
#include "popups/data/PopupData.hpp"

namespace app
{
    /// TODO should be done with: attachPopups(const std::vector<gui::popup::ID> &popupsList)
    /// TODO what with these popups...
    /// 1. add default popup blueprint?
    /// 2. just do not handle if not registered...
    /// else {
    ///     switchWindow(gui::popup::resolveWindowName(id));
    /// }
    /// Blueprints should be classes
    void ApplicationCommon::registerPopupBlueprints()
    {
        using namespace gui::popup;
        popupBlueprint.registerBlueprint(
            ID::PhoneModes, [&](gui::popup::ID /*id*/, std::unique_ptr<gui::PopupRequestParams> &params) {
                auto popupParams = dynamic_cast<gui::PhoneModePopupRequestParams *>(params.get());
                if (popupParams == nullptr) {
                    return false;
                }
                const auto &mode = popupParams->getPhoneMode();
                auto flightModeSetting =
                    settings->getValue(settings::Cellular::offlineMode, settings::SettingsScope::Global);
                bool flightMode = flightModeSetting == "1";

                const auto &popupName = resolveWindowName(gui::popup::ID::PhoneModes);
                if (const auto currentWindowName = getCurrentWindow()->getName(); currentWindowName == popupName) {
                    updateCurrentWindow(std::make_unique<gui::ModesPopupData>(mode, flightMode));
                }
                else {
                    switchWindowPopup(popupName, std::make_unique<gui::ModesPopupData>(mode, flightMode));
                }
                return true;
            });
        popupBlueprint.registerBlueprint(
            ID::Volume, [&](gui::popup::ID /*id*/, std::unique_ptr<gui::PopupRequestParams> &params) {
                auto volumeParams = dynamic_cast<const gui::VolumePopupRequestParams *>(params.get());
                if (volumeParams == nullptr) {
                    return false;
                }
                LOG_INFO("Playback: %s, volume: %s",
                         audio::str(volumeParams->getAudioContext().second).c_str(),
                         std::to_string(volumeParams->getVolume()).c_str());
                auto volume          = volumeParams->getVolume();
                auto context         = volumeParams->getAudioContext();
                const auto popupName = resolveWindowName(gui::popup::ID::Volume);
                if (const auto currentWindowName = getCurrentWindow()->getName(); currentWindowName == popupName) {
                    updateCurrentWindow(std::make_unique<gui::VolumePopupData>(volume, context));
                }
                else {
                    switchWindowPopup(popupName, std::make_unique<gui::VolumePopupData>(volume, context));
                }
                return true;
            });
        popupBlueprint.registerBlueprint(
            ID::PhoneLock, [&](gui::popup::ID id, std::unique_ptr<gui::PopupRequestParams> & /*params*/) {
                switchWindowPopup(gui::popup::resolveWindowName(id), nullptr, SwitchReason::PhoneLock);
                return true;
            });
        auto phoneLockBlueprint = [&](gui::popup::ID id, std::unique_ptr<gui::PopupRequestParams> &params) {
            LOG_ERROR("HANDLE LOCK BLUEPRINT");
            auto popupParams = dynamic_cast<gui::PhoneUnlockInputRequestParams *>(params.get());
            if (popupParams == nullptr) {
                LOG_ERROR("this is most probably due to wrong unique_ptr handling - please check");
                return false;
            }

            popupParams->setDisposition(gui::popup::Disposition{gui::popup::Disposition::Priority::Normal,
                                                                gui::popup::Disposition::WindowType::Popup,
                                                                params->getPopupId()});
            switchWindowPopup(
                gui::popup::resolveWindowName(id),
                std::make_unique<locks::LockData>(popupParams->getLock(), popupParams->getPhoneLockInputTypeAction()));
            return true;
        };
        popupBlueprint.registerBlueprint(ID::PhoneLockInput, phoneLockBlueprint);
        popupBlueprint.registerBlueprint(ID::PhoneLockChangeInfo, phoneLockBlueprint);
        auto simLockBlueprint = [&](gui::popup::ID id, std::unique_ptr<gui::PopupRequestParams> &params) {
            auto popupParams = dynamic_cast<const gui::SimUnlockInputRequestParams *>(params.get());
            if (popupParams == nullptr) {
                return false;
            }
            switchWindowPopup(gui::popup::resolveWindowName(id),
                              std::make_unique<locks::SimLockData>(popupParams->getLock(),
                                                                   popupParams->getSimInputTypeAction(),
                                                                   popupParams->getErrorCode()));
            return true;
        };
        popupBlueprint.registerBlueprint(ID::SimLock, simLockBlueprint);
        popupBlueprint.registerBlueprint(ID::SimInfo, simLockBlueprint);
        popupBlueprint.registerBlueprint(
            ID::Alarm, [&](gui::popup::ID id, std::unique_ptr<gui::PopupRequestParams> &params) {
                auto popupParams = dynamic_cast<gui::AlarmPopupRequestParams *>(params.get());
                if (popupParams == nullptr) {
                    return false;
                }
                popupParams->setDisposition(gui::popup::Disposition{gui::popup::Disposition::Priority::High,
                                                                    gui::popup::Disposition::WindowType::Popup,
                                                                    params->getPopupId()});
                switchWindowPopup(gui::popup::resolveWindowName(id),
                                  std::make_unique<gui::AlarmPopupRequestParams>(popupParams));
                return true;
            });
    }
} // namespace app
