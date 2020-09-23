#include "Application.hpp"
#include "BusChannelsCustom.hpp"                         // for BusChannels...
#include "Common.hpp"                                    // for RefreshModes
#include "GuiTimer.hpp"                                  // for GuiTimer
#include "Item.hpp"                                      // for Item
#include "MessageType.hpp"                               // for MessageType
#include "Service/Timer.hpp"                             // for Timer
#include "SettingsRecord.hpp"                            // for SettingsRecord
#include "Timer.hpp"                                     // for Timer
#include "Translator.hpp"                                // for KeyInputSim...
#include "common_data/EventStore.hpp"                    // for Battery
#include "common_data/RawKey.hpp"                        // for RawKey, key...
#include "gui/input/InputEvent.hpp"                      // for InputEvent
#include "log/debug.hpp"                                 // for DEBUG_APPLI...
#include "log/log.hpp"                                   // for LOG_INFO
#include "messages/AppMessage.hpp"                       // for AppSwitchMe...
#include "service-appmgr/ApplicationManager.hpp"         // for Application...
#include "service-cellular/messages/CellularMessage.hpp" // for CellularNot...
#include "service-db/api/DBServiceAPI.hpp"               // for DBServiceAPI
#include "service-evtmgr/messages/BatteryMessages.hpp"   // for BatteryLeve...
#include "service-evtmgr/messages/EVMessages.hpp"        // for RtcMinuteAl...
#include "service-evtmgr/messages/KbdMessage.hpp"        // for KbdMessage
#include "service-gui/messages/DrawMessage.hpp"          // for DrawMessage
#include "task.h"                                        // for xTaskGetTic...
#include "windows/AppWindow.hpp"                         // for AppWindow
#include <Text.hpp>                                      // for Text
#include <algorithm>                                     // for find
#include <iterator>                                      // for distance, next
#include <type_traits>                                   // for add_const<>...

namespace gui
{
    class DrawCommand;
}

namespace app::audioConsts
{
    const std::vector<audio::PlaybackType> typesToMute = {audio::PlaybackType::Notifications,
                                                          audio::PlaybackType::CallRingtone,
                                                          audio::PlaybackType::TextMessageRingtone};
} // namespace app::audioConsts

namespace app
{

    const char *Application::stateStr(Application::State st)
    {
        switch (st) {
        case State::NONE:
            return "NONE";
        case State::DEACTIVATED:
            return "DEACTIVATED";
        case State::INITIALIZING:
            return "INITIALIZING";
        case State::ACTIVATING:
            return "ACTIVATING";
        case State::ACTIVE_FORGROUND:
            return "ACTIVE_FORGROUND";
        case State::ACTIVE_BACKGROUND:
            return "ACTIVE_BACKGROUND";
        case State::DEACTIVATING:
            return "DEACTIVATING";
        default:
            return "FixIt";
        }
    }

    Application::Application(
        std::string name, std::string parent, bool startBackground, uint32_t stackDepth, sys::ServicePriority priority)
        : Service(name, parent, stackDepth, priority),
          startBackground{startBackground}
    {
        keyTranslator = std::make_unique<gui::KeyInputSimpleTranslation>();
        busChannels.push_back(sys::BusChannels::ServiceCellularNotifications);
        if (startBackground) {
            setState(State::ACTIVE_BACKGROUND);
        }

        longPressTimer = std::make_unique<sys::Timer>("LongPress", this, key_timer_ms);
        longPressTimer->connect([&](sys::Timer &) { longPressTimerCallback(); });
    }

    Application::~Application()
    {
        for (auto it = windows.begin(); it != windows.end(); it++) {
            delete it->second;
        }
        windows.clear();
    }

    Application::State Application::getState()
    {
        return state;
    }

    void Application::setState(State st)
    {
#if DEBUG_APPLICATION_MANAGEMENT == 1
        LOG_DEBUG("[%s] (%s) -> (%s)", GetName().c_str(), stateStr(state), stateStr(st));
#endif
        state = st;
    }


    void Application::longPressTimerCallback()
    {
        // TODO if(check widget type long press trigger)
        uint32_t time = xTaskGetTickCount();
        if (keyTranslator->timeout(time)) {
            // previous key press was over standard keypress timeout - send long press
            gui::InputEvent iev = keyTranslator->translate(time);
            messageInputEventApplication(this, this->GetName(), iev);
            // clean previous key
            keyTranslator->prev_key_press = {};
            longPressTimer->stop();
        }
    }

    void Application::render(gui::RefreshModes mode)
    {
        if (getCurrentWindow() == nullptr) {
            LOG_ERROR("Current window is not defined");
            return;
        }

        // send drawing commands only when if application is in active and visible.
        if (state == State::ACTIVE_FORGROUND) {
            auto currwin = getCurrentWindow();
            if (Store::Battery::get().state == Store::Battery::State::Charging) {
                currwin->batteryCharging(true);
            }
            else {
                currwin->updateBatteryLevel(Store::Battery::get().level);
            }
            currwin->setSIM();
            currwin->updateSignalStrength();

            std::list<gui::DrawCommand *> commandsList = currwin->buildDrawList();

            if (shutdownInProgress) {
                auto msg =
                    std::make_shared<sgui::DrawMessage>(commandsList, mode, sgui::DrawMessage::DrawCommand::SHUTDOWN);
                sys::Bus::SendUnicast(msg, "ServiceGUI", this);
            }
            else if (suspendInProgress) {
                auto msg =
                    std::make_shared<sgui::DrawMessage>(commandsList, mode, sgui::DrawMessage::DrawCommand::SUSPEND);
                sys::Bus::SendUnicast(msg, "ServiceGUI", this);
            }
            else {
                auto msg =
                    std::make_shared<sgui::DrawMessage>(commandsList, mode, sgui::DrawMessage::DrawCommand::NORMAL);
                sys::Bus::SendUnicast(msg, "ServiceGUI", this);
            }
        }

        if (suspendInProgress)
            suspendInProgress = false;
    }
    void Application::blockEvents(bool isBlocked)
    {
        acceptInput = isBlocked;
    }
    void Application::switchWindow(const std::string &windowName,
                                   gui::ShowMode cmd,
                                   std::unique_ptr<gui::SwitchData> data)
    {

        std::string window;
#if DEBUG_APPLICATION_MANAGEMENT == 1
        LOG_INFO("switching [%s] to window: %s data description: %s",
                 GetName().c_str(),
                 windowName.length() ? windowName.c_str() : gui::name::window::main_window.c_str(),
                 data ? data->getDescription().c_str() : "");
#endif

        // case to handle returning to previous application
        if (windowName == "LastWindow") {
            window = getCurrentWindow()->getName();
            auto msg =
                std::make_shared<AppSwitchWindowMessage>(window, getCurrentWindow()->getName(), std::move(data), cmd);
            sys::Bus::SendUnicast(msg, this->GetName(), this);
        }
        else {
            window   = windowName.empty() ? gui::name::window::main_window : windowName;
            auto msg = std::make_shared<AppSwitchWindowMessage>(
                window, getCurrentWindow() ? getCurrentWindow()->getName() : "", std::move(data), cmd);
            sys::Bus::SendUnicast(msg, this->GetName(), this);
        }
    }

    void Application::returnToPreviousWindow(const uint32_t times)
    {
        auto prevWindow = getPrevWindow(times);
        if (prevWindow == gui::name::window::no_window) {
            LOG_INFO("Back to previous application");
            cleanPrevWindw();
            sapm::ApplicationManager::messageSwitchPreviousApplication(this);
        }
        else {
            LOG_INFO("Back to previous window %s", prevWindow.c_str());
            switchWindow(prevWindow, gui::ShowMode::GUI_SHOW_RETURN);
        }
    }

    void Application::refreshWindow(gui::RefreshModes mode)
    {
        auto msg = std::make_shared<AppRefreshMessage>(mode);
        sys::Bus::SendUnicast(msg, this->GetName(), this);
    }

    sys::Message_t Application::DataReceivedHandler(sys::DataMessage *msgl)
    {
        auto msg = dynamic_cast<CellularNotificationMessage *>(msgl);
        if (msg != nullptr && msg->type == CellularNotificationMessage::Type::SignalStrengthUpdate) {
            return handleSignalStrengthUpdate(msgl);
        }
        else if (msgl->messageType == MessageType::AppInputEvent) {
            return handleInputEvent(msgl);
        }
        else if (msgl->messageType == MessageType::KBDKeyEvent) {
            return handleKBDKeyEvent(msgl);
        }
        else if (msgl->messageType == MessageType::EVMBatteryLevel) {
            return handleBatteryLevel(msgl);
        }
        else if (msgl->messageType == MessageType::EVMChargerPlugged) {
            return handleChargerPlugged(msgl);
        }
        else if (msgl->messageType == MessageType::EVMMinuteUpdated) {
            return handleMinuteUpdated(msgl);
        }
        else if (msgl->messageType == MessageType::AppSwitch) {
            return handleApplicationSwitch(msgl);
        }
        else if (msgl->messageType == MessageType::AppSwitchWindow) {
            return handleSwitchWindow(msgl);
        }
        else if (msgl->messageType == MessageType::AppClose) {
            return handleAppClose(msgl);
        }
        else if (msgl->messageType == MessageType::AppRebuild) {
            return handleAppRebuild(msg);
        }
        else if (msgl->messageType == MessageType::AppRefresh) {
            return handleAppRefresh(msgl);
        }
        else if (dynamic_cast<sevm::SIMMessage *>(msgl) != nullptr) {
            return handleSIMMessage(msgl);
        }
        return msgNotHandled();
    }

    sys::Message_t Application::handleSignalStrengthUpdate(sys::DataMessage *msgl)
    {
        if ((state == State::ACTIVE_FORGROUND) && getCurrentWindow()->updateSignalStrength()) {
            refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }
        return msgHandled();
    }

    sys::Message_t Application::handleInputEvent(sys::DataMessage *msgl)
    {
        AppInputEventMessage *msg = reinterpret_cast<AppInputEventMessage *>(msgl);
        if (msg->getEvent().state == gui::InputEvent::State::keyPressed) {
            longPressTimer->start();
        }
        else if (msg->getEvent().state == gui::InputEvent::State::keyReleasedShort) {
            longPressTimer->stop();
        }
        if (getCurrentWindow() != nullptr && getCurrentWindow()->onInput(msg->getEvent())) {
            refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }
        return msgHandled();
    }

    sys::Message_t Application::handleKBDKeyEvent(sys::DataMessage *msgl)
    {
        if (this->getState() != app::Application::State::ACTIVE_FORGROUND) {
            LOG_FATAL("!!! Terrible terrible damage! application with no focus grabbed key!");
        }
        sevm::KbdMessage *msg = static_cast<sevm::KbdMessage *>(msgl);
        gui::InputEvent iev   = keyTranslator->translate(msg->key);
        if (iev.keyCode != gui::KeyCode::KEY_UNDEFINED) {
            messageInputEventApplication(this, this->GetName(), iev);
        }
        return msgHandled();
    }

    sys::Message_t Application::handleBatteryLevel(sys::DataMessage *msgl)
    {
        auto msg = static_cast<sevm::BatteryLevelMessage *>(msgl);
        LOG_INFO("Application battery level: %d", msg->levelPercents);

        if (getCurrentWindow()->updateBatteryLevel(msg->levelPercents)) {
            refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }
        return msgHandled();
    }

    sys::Message_t Application::handleChargerPlugged(sys::DataMessage *msgl)
    {
        auto *msg = static_cast<sevm::BatteryPlugMessage *>(msgl);
        if (msg->plugged == true) {
            LOG_INFO("Application charger connected");
            getCurrentWindow()->batteryCharging(true);
            refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }
        else {
            LOG_INFO("Application charger disconnected");
            getCurrentWindow()->batteryCharging(false);
            refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }

        refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        return msgHandled();
    }

    sys::Message_t Application::handleMinuteUpdated(sys::DataMessage *msgl)
    {
        auto *msg = static_cast<sevm::RtcMinuteAlarmMessage *>(msgl);
        getCurrentWindow()->updateTime(msg->timestamp, !settings.timeFormat12);
        if (state == State::ACTIVE_FORGROUND) {
            refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }
        return msgHandled();
    }

    sys::Message_t Application::handleApplicationSwitch(sys::DataMessage *msgl)
    {
        auto *msg             = static_cast<AppSwitchMessage *>(msgl);
        bool handled          = false;
        LOG_DEBUG("AppSwitch: %s", msg->getTargetApplicationName().c_str());
        // Application is starting or it is in the background. Upon switch command if name if correct it goes
        // foreground
        if ((state == State::ACTIVATING) || (state == State::INITIALIZING) || (state == State::ACTIVE_BACKGROUND)) {

            if (msg->getTargetApplicationName() == this->GetName()) {
                setState(State::ACTIVE_FORGROUND);
                if (sapm::ApplicationManager::messageConfirmSwitch(this)) {
                    LOG_INFO("target Window: %s : target description: %s",
                             msg->getTargetWindowName().c_str(),
                             msg->getData() ? msg->getData()->getDescription().c_str() : "");
                    switchWindow(msg->getTargetWindowName(), std::move(msg->getData()));
                    handled = true;
                }
                else {
                    // TODO send to itself message to close
                    LOG_ERROR("Failed to communicate ");
                }
            }
            else {
                LOG_ERROR("Received switch message outside of activation flow");
            }
        }
        else if (state == State::ACTIVE_FORGROUND) {
            if (msg->getTargetApplicationName() == this->GetName()) {
                // if window name and data are null pointers this is a message informing
                // that application should go to background mode
                if ((msg->getTargetWindowName() == "") && (msg->getData() == nullptr)) {
                    setState(State::ACTIVE_BACKGROUND);
                    if (sapm::ApplicationManager::messageConfirmSwitch(this)) {
                        handled = true;
                    }
                    else {
                        // TODO send to itself message to close
                        LOG_ERROR("Failed to communicate ");
                    }
                }
                // if application is in front and receives message with defined window it should
                // change to that window.
                else {
                    switchWindow(msg->getTargetWindowName(), std::move(msg->getData()));
                    handled = true;
                }
            }
            else {
                LOG_ERROR("Received switch message outside of activation flow");
            }
        }
        else {
            LOG_ERROR("Wrong internal application %s switch to ACTIVE state form %s",
                      msg->getTargetApplicationName().c_str(),
                      stateStr(state));
        }
        if (handled) {
            return msgHandled();
        }
        return msgNotHandled();
    }

    sys::Message_t Application::handleSwitchWindow(sys::DataMessage *msgl)
    {
        auto msg = static_cast<AppSwitchWindowMessage *>(msgl);
        // check if specified window is in the application
        auto it = windows.find(msg->getWindowName());
        if (it != windows.end()) {
            auto switchData = std::move(msg->getData());
            if (switchData && switchData->ignoreCurrentWindowOnStack) {
                popToWindow(getPrevWindow());
            }
            getCurrentWindow()->onClose();
            setActiveWindow(msg->getWindowName());

            getCurrentWindow()->handleSwitchData(switchData.get());

            // check if this is case where application is returning to the last visible window.
            if ((switchData != nullptr) && (msg->LastSeenWindow)) {}
            else {
                getCurrentWindow()->onBeforeShow(msg->getCommand(), switchData.get());
                auto ret = dynamic_cast<gui::SwitchSpecialChar *>(switchData.get());
                if (ret != nullptr && switchData != nullptr) {
                    auto text = dynamic_cast<gui::Text *>(getCurrentWindow()->getFocusItem());
                    if (text != nullptr) {
                        text->addText(ret->getDescription());
                    }
                }
            }
            refreshWindow(gui::RefreshModes::GUI_REFRESH_DEEP);
        }
        else {
            LOG_ERROR("No such window: %s", msg->getWindowName().c_str());
        }
        return msgHandled();
    }

    sys::Message_t Application::handleAppClose(sys::DataMessage *msgl)
    {
        setState(State::DEACTIVATING);
        sapm::ApplicationManager::messageConfirmClose(this);
        return msgHandled();
    }

    sys::Message_t Application::handleAppRebuild(sys::DataMessage *msgl)
    {
        LOG_INFO("Application %s rebuilding gui", GetName().c_str());
        for (auto it = windows.begin(); it != windows.end(); it++) {
            LOG_DEBUG("Rebuild: %s", it->first.c_str());
            if (!it->second) {
                LOG_ERROR("NO SUCH WINDOW");
            }
            else {
                it->second->rebuild();
            }
        }
        LOG_INFO("Refresh app with focus!");
        if (state == State::ACTIVE_FORGROUND) {
            refreshWindow(gui::RefreshModes::GUI_REFRESH_DEEP);
        }
        LOG_INFO("App rebuild done");
        return msgHandled();
    }

    sys::Message_t Application::handleAppRefresh(sys::DataMessage *msgl)
    {
        AppRefreshMessage *msg = reinterpret_cast<AppRefreshMessage *>(msgl);
        render(msg->getMode());
        return msgHandled();
    }

    sys::Message_t Application::handleSIMMessage(sys::DataMessage *msgl)
    {
        getCurrentWindow()->setSIM();
        refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        return msgHandled();
    }

    sys::ReturnCodes Application::InitHandler()
    {
        bool initState = true;
        setState(State::INITIALIZING);
        //	uint32_t start = xTaskGetTickCount();
        settings = DBServiceAPI::SettingsGet(this);
        //	uint32_t stop = xTaskGetTickCount();
        //	LOG_INFO("DBServiceAPI::SettingsGet %d", stop-start);
        initState = (settings.dbID == 1);

        // send response to application manager true if successful, false otherwise.
        sapm::ApplicationManager::messageRegisterApplication(this, initState, startBackground);
        sys::ReturnCodes retCode = (initState ? sys::ReturnCodes::Success : sys::ReturnCodes::Failure);
        return retCode;
    }

    sys::ReturnCodes Application::DeinitHandler()
    {
        LOG_INFO("Closing an application: %s", GetName().c_str());
        for (const auto &[windowName, window] : windows) {
            LOG_INFO("Closing a window: %s", windowName.c_str());
            window->onClose();
        }
        return sys::ReturnCodes::Success;
    }

    void Application::setActiveWindow(const std::string &windowName)
    {
        pushWindow(windowName);
        acceptInput = true;
    }

    bool Application::setVolume(const audio::Volume &value,
                                const audio::Profile::Type &profileType,
                                const audio::PlaybackType &playbackType)
    {
        const auto ret = AudioServiceAPI::SetSetting(this, audio::Setting::Volume, value, profileType, playbackType);
        return ret == audio::RetCode::Success;
    }

    bool Application::adjustCurrentVolume(const int step)
    {
        audio::Volume volume;
        if (step < 0) {
            AudioServiceAPI::Stop(this, app::audioConsts::typesToMute);
        }
        auto ret = AudioServiceAPI::GetSetting(this, audio::Setting::Volume, volume);
        if (ret == audio::RetCode::Success) {
            ret = ((static_cast<int>(volume) + step) < 0)
                      ? audio::RetCode::Success
                      : AudioServiceAPI::SetSetting(this, audio::Setting::Volume, volume + step);
        }

        return ret == audio::RetCode::Success;
    }

    void Application::messageSwitchApplication(sys::Service *sender,
                                               std::string application,
                                               std::string window,
                                               std::unique_ptr<gui::SwitchData> data)
    {
        auto msg = std::make_shared<AppSwitchMessage>(application, window, std::move(data));
        sys::Bus::SendUnicast(msg, application, sender);
    }

    void Application::messageRefreshApplication(sys::Service *sender,
                                                std::string application,
                                                std::string window,
                                                gui::SwitchData *data)
    {
        auto msg = std::make_shared<AppMessage>(MessageType::AppRefresh);
        sys::Bus::SendUnicast(msg, application, sender);
    }

    void Application::messageCloseApplication(sys::Service *sender, std::string application)
    {

        auto msg = std::make_shared<AppMessage>(MessageType::AppClose);
        sys::Bus::SendUnicast(msg, application, sender);
    }

    void Application::messageRebuildApplication(sys::Service *sender, std::string application)
    {
        auto msg = std::make_shared<AppRebuildMessage>();
        sys::Bus::SendUnicast(msg, application, sender);
    }

    void Application::messageInputEventApplication(sys::Service *sender,
                                                   std::string application,
                                                   const gui::InputEvent &event)
    {
        auto msg = std::make_shared<AppInputEventMessage>(event);
        sys::Bus::SendUnicast(msg, application, sender);
    }

    bool Application::popToWindow(const std::string &window)
    {
        if (window == gui::name::window::no_window) {
            bool ret = false;
            if (windowStack.size() <= 1) {
                windowStack.clear();
                ret = true;
            }
            return ret;
        }

        auto ret = std::find(windowStack.begin(), windowStack.end(), window);
        if (ret != windowStack.end()) {
            LOG_INFO(
                "Pop last window(s) [%d] :  %s", static_cast<int>(std::distance(ret, windowStack.end())), ret->c_str());
            windowStack.erase(std::next(ret), windowStack.end());
            return true;
        }
        return false;
    }

    void Application::pushWindow(const std::string &newWindow)
    {
        // handle if window was already on
        if (popToWindow(newWindow)) {
            return;
        }
        else {
            windowStack.push_back(newWindow);
        }
#if DEBUG_APPLICATION_MANAGEMENT == 1
        LOG_DEBUG("[%d] newWindow: %s", windowStack.size(), newWindow.c_str());
        for (auto &el : windowStack) {
            LOG_DEBUG("-> %s", el.c_str());
        }
        LOG_INFO("\n\n");
#endif
    };

    const std::string Application::getPrevWindow(uint32_t count) const
    {
        if (this->windowStack.size() <= 1 || count > this->windowStack.size()) {
            return gui::name::window::no_window;
        }
        return *std::prev(windowStack.end(), count + 1);
    }

    void Application::Application::cleanPrevWindw()
    {
        this->windowStack.clear();
    }

    gui::AppWindow *Application::getWindow(const std::string &window)
    {
        auto it = windows.find(window);
        if (it != windows.end()) {
            return it->second;
        }
        return nullptr;
    }

    gui::AppWindow *Application::getCurrentWindow()
    {
        std::string window = "";
        if (windowStack.size() == 0) {
            window = gui::name::window::main_window;
        }
        else {
            window = windowStack.back();
        }

        return getWindow(window);
    }

    void Application::attachWindow(gui::AppWindow *window)
    {
        windows.insert({window->getName(), window});
    }

    void Application::connect(std::unique_ptr<app::GuiTimer> &&timer, gui::Item *item)
    {
        timer->sysapi.connect(item);
        item->attachTimer(std::move(timer));
    }
} /* namespace app */
