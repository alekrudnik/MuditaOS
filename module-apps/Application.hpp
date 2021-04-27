﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "AsyncTask.hpp"
#include "Audio/AudioCommon.hpp"      // for Volume, Play...
#include "Audio/Profiles/Profile.hpp" // for Profile, Pro...
#include "CallbackStorage.hpp"

#include "Service/Common.hpp"  // for ReturnCodes
#include "Service/Message.hpp" // for MessagePointer
#include "Service/Service.hpp" // for Service
#include "Timers/TimerHandle.hpp"
#include "SwitchData.hpp"                  // for SwitchData
#include "SystemManager/SystemManager.hpp" // for SystemManager
#include "bsp/keyboard/key_codes.hpp"      // for bsp
#include "gui/Common.hpp"                  // for ShowMode
#include "projdefs.h"                      // for pdMS_TO_TICKS
#include <PhoneModes/Observer.hpp>

#include <service-appmgr/ApplicationManifest.hpp>
#include <list>     // for list
#include <map>      // for allocator, map
#include <memory>   // for make_shared
#include <stdint.h> // for uint32_t
#include <string>   // for string
#include <utility>  // for move, pair
#include <vector>   // for vector
#include "TopBarManager.hpp"
#include "WindowsFactory.hpp"
#include "WindowsStack.hpp"
#include "popups/Popups.hpp"

namespace app
{
    class WindowsStack;

    namespace manager::actions
    {
        class NotificationsChangedParams;
    }
} // namespace app
namespace gui
{
    class AppWindow;
    class InputEvent;
    class Item;
    class PopupRequestParams;
    class KeyInputSimpleTranslation;
} // namespace gui
namespace settings
{
    class Settings;
}

namespace app
{
    class Application;
    class GuiTimer;

    class ActionHandledResponse; // Forward declaration
    using ActionResult = std::shared_ptr<ActionHandledResponse>;
    auto actionHandled() -> ActionResult;
    auto actionNotHandled() -> ActionResult;

    using ApplicationName = std::string;

    enum class StartupStatus
    {
        Success,
        Failure
    };

    enum class StartupReason
    {
        Launch,   // Default startup causing application MainWindow to be added to stack.
        OnAction, // Switch to application  was caused by action. Enum is used to prevent called applications to
        // switch to main window on application switch and allow declared handler to switch to desired window.
    };

    struct StartInBackground
    {
        StartInBackground(bool _value) : value{_value}
        {}

        explicit operator bool() const noexcept
        {
            return value;
        }

        bool value;
    };

    /// Type traits pattern used to enforce user-defined types to implement "GetManifest" function.
    template <class T> struct ManifestTraits;

    template <class, class = void> struct HasManifest : std::false_type
    {};

    /// Checks whether T implements "GetManifest" static method.
    /// Provides the member constant "value" that is equal to true if T implements "GetManifest" static method.
    /// Otherwise, "value" is equal to false.
    template <class T>
    struct HasManifest<T, std::void_t<decltype(&ManifestTraits<T>::GetManifest)>>
        : std::is_same<app::manager::ApplicationManifest, decltype(ManifestTraits<T>::GetManifest())>
    {};

    /// Retrieves the manifest of T, if T implements ManifestTraits.
    /// Otherwise, reports an error during compile time.
    template <class T, std::enable_if_t<HasManifest<T>::value, int> = 0>
    auto ManifestOf() -> manager::ApplicationManifest
    {
        return ManifestTraits<T>::GetManifest();
    }

    /// This is template for creating new applications. Main difference between Application and service is that:
    /// 1. Application has access to GUI and Input
    /// 2. Application lifetime is managed with app::manager::ApplicationManager
    class Application : public sys::Service, public AsyncCallbacksDeleter
    {
      public:
        /// state in which application is right now
        /// @note do not mistake with application manager state
        enum class State
        {
            /// Application doesn't exist
            NONE,
            /// Application: Object has been created and underlying service is waiting to execute init handler method.
            /// Application Manager: Launcher for the application has been provided. Application can be started using
            /// provided launcher. The other possibility is that Appication Manager received CLOSING_FINISHED message.
            DEACTIVATED,
            /// Application: Set after entering the init handler of the application. In this state application will
            /// request in a blocking way the db configuration of the phone. Before exiting the init handler application
            /// must send APP_REGISTER message to Application Manager. Application Manager: Initialization is triggered
            /// by the switch message sent by other application to the application manager. Launcher for the application
            /// has been executed upon receiving switch command. Optional switch data has been saved and it will be
            /// provided when application manager receives registration message.
            INITIALIZING,
            /// Application manager sent variant of switch command to the selected application and it's now waiting for
            /// confirmation from the application
            ACTIVATING,
            /// Application is working and has focus and can render
            ACTIVE_FORGROUND,
            /// Applicatino lost focus but it is still working
            ACTIVE_BACKGROUND,
            /// Application: Close request message has been received. As a response application must send close request
            /// acknowledge message. Application must start closing all workers and releasing resources. After all
            /// workers are closed and resources released application sends to application manager CLOSING_FINISHED
            /// message Application Manager sent signal to application that it should be closed. Application must
            /// perform all necessary cleanup and request System Manager to close it.
            DEACTIVATING
        };

        /// c_str() function for Application::State
        static const char *stateStr(State st);

        using OnActionReceived = std::function<ActionResult(manager::actions::ActionParamsPtr &&)>;

      private:
        std::string default_window;
        State state = State::DEACTIVATED;
        bool lockScreenPasscodeIsOn;

        sys::MessagePointer handleSignalStrengthUpdate(sys::Message *msgl);
        sys::MessagePointer handleNetworkAccessTechnologyUpdate(sys::Message *msgl);
        sys::MessagePointer handleInputEvent(sys::Message *msgl);
        sys::MessagePointer handleKBDKeyEvent(sys::Message *msgl);
        sys::MessagePointer handleBatteryStatusChange();
        sys::MessagePointer handleMinuteUpdated(sys::Message *msgl);
        sys::MessagePointer handleAction(sys::Message *msgl);
        sys::MessagePointer handleApplicationSwitch(sys::Message *msgl);
        sys::MessagePointer handleApplicationSwitchLaunch(sys::Message *msgl);
        sys::MessagePointer handleApplicationSwitchOnAction(sys::Message *msgl);
        sys::MessagePointer handleSwitchWindow(sys::Message *msgl);
        sys::MessagePointer handleUpdateWindow(sys::Message *msgl);
        sys::MessagePointer handleAppClose(sys::Message *msgl);
        sys::MessagePointer handleAppRebuild(sys::Message *msgl);
        sys::MessagePointer handleAppRefresh(sys::Message *msgl);
        sys::MessagePointer handleGetDOM(sys::Message *msgl);
        sys::MessagePointer handleAppFocusLost(sys::Message *msgl);
        sys::MessagePointer handleSIMMessage(sys::Message *msgl);

        virtual bool isPopupPermitted(gui::popup::ID popupId) const;

        std::list<std::unique_ptr<app::GuiTimer>> gui_timers;
        std::unordered_map<manager::actions::ActionId, OnActionReceived> receivers;

      public:
        sys::TimerHandle longPressTimer;
        void clearLongPressTimeout();

        Application(std::string name,
                    std::string parent                  = "",
                    sys::phone_modes::PhoneMode mode    = sys::phone_modes::PhoneMode::Connected,
                    StartInBackground startInBackground = {false},
                    uint32_t stackDepth                 = 4096,
                    sys::ServicePriority priority       = sys::ServicePriority::Idle);

        virtual ~Application() noexcept;

        Application::State getState();
        void setState(State st);

        /// Method responsible for rendering currently active window.
        /// 1. queries for static data for all windows form Store (i.e. battery level, sim card level)
        /// 2. loads rendering commands and send them to GUI renderer (sgui::ServiceGUI)
        void render(gui::RefreshModes mode);

        /// Responsible for sending updated data to the current window.
        /// @note Sending different window name than the current one won't cause switch.
        /// @param windowName name of the window that suppose to be updated.
        /// @param data contextual data for windows flow control.
        void updateWindow(const std::string &windowName, std::unique_ptr<gui::SwitchData> data);

        /// Method sending switch command for another window. It will switch window within active application.
        /// To switch windows between applications use app::manager::Controller::switchApplication
        /// it will effectively trigger setActiveWindow and change on windows stack
        ///
        /// @param windowName name of window to show, only required parameter, our windows stack uses string names as
        /// id's
        /// @param cmd command for Application, right now it's either: Start or Return. As user only Start is used.
        /// @param data contextual data for UI Windows flow steering. Base SwitchData enables user to:
        ///        1. ignoreCurrentWindowOnStack: omit window from windows stack, so that it won't be shown when back (
        ///        RF key) is used
        ///        2. disableAppClose: for use with messageSwitchApplication(...) when switching through app to inform
        ///        that we dont want to close app calling
        void switchWindow(const std::string &windowName,
                          gui::ShowMode cmd                     = gui::ShowMode::GUI_SHOW_INIT,
                          std::unique_ptr<gui::SwitchData> data = nullptr);

        /// Same as switchWindow above
        inline void switchWindow(const std::string &windowName, std::unique_ptr<gui::SwitchData> data)
        {
            switchWindow(windowName, gui::ShowMode::GUI_SHOW_INIT, std::move(data));
        };

        /// Method used to go back to desired window by using the index difference on stack value
        ///@param ignoredWindowsNumber: defines how many windows will be skipped while going back on stack
        void returnToPreviousWindow(const uint32_t times = 1);

        /// Method refreshing active window
        void refreshWindow(gui::RefreshModes mode);

        /// to not hide overload from parent
        using Service::DataReceivedHandler;

        /// Mehtod to handle bus messages, all message types are defined in Message.hpp
        /// Message types are in MessageType
        sys::MessagePointer DataReceivedHandler(sys::DataMessage *msgl);

        /// initialization function
        /// 1. it has to be called for each and every application instance, it registeres application in
        /// ApplicationManager
        ///    and notify ApplicationManager that it's in Initialization state
        /// 2. It gets settings from database
        sys::ReturnCodes InitHandler() override;

        /// Deinitialization function.
        /// Called upon Application exit and/or termination request.
        sys::ReturnCodes DeinitHandler() override;

        /// function to set active window for application
        /// if none window is selected main window is used
        /// if window name is "LastWindow" then previous window is selected
        /// it modifies windows stack
        void setActiveWindow(const std::string &windowName);

        /// see suspendInProgress documentation
        virtual void setSuspendFlag(bool val)
        {
            suspendInProgress = val;
        };

        // Latching close system in progress flag
        virtual void setSystemCloseInProgress()
        {
            systemCloseInProgress = true;
        }

        bool adjustCurrentVolume(const int step);
        bool increaseCurrentVolume(const audio::Volume step = audio::defaultVolumeStep)
        {
            return adjustCurrentVolume(step);
        }
        bool decreaseCurrentVolume(const audio::Volume step = audio::defaultVolumeStep)
        {
            return adjustCurrentVolume(-step);
        }

        void toggleTorchOnOff();

        void toggleTorchColor();

        void cancelCallbacks(AsyncCallbackReceiver::Ptr receiver) override;

        /// @defgroup Application Application static functions
        /// All this functions are meant to be used in ApplicationManager only
        /// @note consider moving these as private elements of ApplicationManager i.e. under names
        /// message::switchApplication etc.
        /// @{
        static void requestAction(sys::Service *sender,
                                  const ApplicationName &applicationName,
                                  manager::actions::ActionId actionId,
                                  manager::actions::ActionParamsPtr &&data);
        static void messageSwitchApplication(sys::Service *sender,
                                             std::string application,
                                             std::string window,
                                             std::unique_ptr<gui::SwitchData> data,
                                             StartupReason startupReason);
        static void messageCloseApplication(sys::Service *sender, std::string application);
        static void messageRebuildApplication(sys::Service *sender, std::string application);
        static void messageApplicationLostFocus(sys::Service *sender, std::string application);
        /// @}

      protected:
        void longPressTimerCallback();
        /// Method used to register all windows and widgets in application
        virtual void createUserInterface() = 0;
        /// Method closing application's windows.
        virtual void destroyUserInterface() = 0;

        /// Handle the change of phone mode and tethering mode
        /// @param mode new phone mode
        /// @param tethering new tethering mode
        void handlePhoneModeChanged(sys::phone_modes::PhoneMode mode);

        /// Handles volume changed event
        /// @param volume current volume level
        /// @param context audio context which contains current profile and playback
        void handleVolumeChanged(audio::Volume volume, audio::Context context);

        /// @ingrup AppWindowStack
        WindowsStack windowsStack;
        WindowsFactory windowsFactory;

        /// Method used to attach popups windows to application
        void attachPopups(const std::vector<gui::popup::ID> &popupsList);
        void showPopup(gui::popup::ID id, const gui::PopupRequestParams *params);
        void abortPopup(gui::popup::ID id);

      public:
        /// @ingrup AppWindowStack
        /// get to the first time we entered this &window
        bool popToWindow(const std::string &window);
        /// push window to the top of windows stack
        /// @ingrup AppWindowStack
        void pushWindow(const std::string &newWindow);
        /// getter for previous window name
        /// @ingrup AppWindowStack
        const std::string getPrevWindow(uint32_t count = 1) const;
        /// clears windows stack
        /// @ingrup AppWindowStack
        void cleanPrevWindw();
        /// getter for current wisible window in application
        /// if there is none - returns default window
        /// @ingrup AppWindowStack
        gui::AppWindow *getCurrentWindow();
        bool isCurrentWindow(const std::string &windowName) const noexcept;

        gui::AppWindow *getWindow(const std::string &name);
        /// to avoid conflicts with connect below
        using Service::connect;
        /// connects item with GuiTimer and stores it in app
        void connect(GuiTimer *timer, gui::Item *item);

      protected:
        /// key translator to handle
        /// 1. long press transformation (right now we support long press only via top application keyTralator handling)
        /// 2. simple translation of keys 1 to 1 with keyboard
        std::unique_ptr<gui::KeyInputSimpleTranslation> keyTranslator;
        /// Flag defines how application will behave after registration. It can go forground or background
        StartInBackground startInBackground{false};
        /// Flag which defines whether application initialized suspend mode, this will influence how render message will
        /// sent to gui service. If suspend is true, application manager will receive information from both eink and gui
        /// services if last rendering mesage will be processed.
        bool suspendInProgress = false;

        bool systemCloseInProgress = false;
        /// Storage for asynchronous tasks callbacks.
        std::unique_ptr<CallbackStorage> callbackStorage;
        friend class AsyncTask; // Async tasks need access to application internals, e.g. callback storage, to make
                                // their API simple.

        /// informs self that there was key press
        /// used to provide a way for long press/multipress handling in application
        static void messageInputEventApplication(sys::Service *sender,
                                                 std::string application,
                                                 const gui::InputEvent &event);

        void addActionReceiver(manager::actions::ActionId actionId, OnActionReceived &&callback);
        virtual void handle(manager::actions::NotificationsChangedParams *params);

        std::unique_ptr<TopBarManager> topBarManager;

        /// application's settings
        std::unique_ptr<settings::Settings> settings;
        sys::phone_modes::PhoneMode phoneMode;

      public:
        void setLockScreenPasscodeOn(bool screenPasscodeOn) noexcept;
        bool isLockScreenPasscodeOn() const noexcept;
        const gui::top_bar::Configuration &getTopBarConfiguration() const noexcept;
    };

    /// Parameter pack used by application launch action.
    class ApplicationLaunchData : public manager::actions::ActionParams
    {
      public:
        explicit ApplicationLaunchData(const app::ApplicationName &appName)
            : manager::actions::ActionParams{"Application launch parameters"}, targetAppName{appName}
        {}

        [[nodiscard]] auto getTargetApplicationName() const noexcept
        {
            return targetAppName;
        }

      private:
        ApplicationName targetAppName;
    };
} /* namespace app */
