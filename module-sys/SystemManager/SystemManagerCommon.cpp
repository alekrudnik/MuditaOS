﻿// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <SystemManager/SystemManagerCommon.hpp>

#include <apps-common/ApplicationCommon.hpp>
#include <SystemManager/DependencyGraph.hpp>
#include "graph/TopologicalSort.hpp"

#include "thread.hpp"
#include "ticks.hpp"
#include "critical.hpp"
#include <algorithm>
#include <service-evtmgr/KbdMessage.hpp>
#include <service-evtmgr/BatteryMessages.hpp>
#include <service-evtmgr/Constants.hpp>
#include <service-evtmgr/EventManagerServiceAPI.hpp>
#include <service-evtmgr/EVMessages.hpp>
#include <service-appmgr/messages/UserPowerDownRequest.hpp>
#include <service-desktop/Constants.hpp>
#include <service-appmgr/Constants.hpp>
#include <service-appmgr/Controller.hpp>
#include <service-cellular/Constans.hpp>
#include <system/messages/DeviceRegistrationMessage.hpp>
#include <system/messages/SentinelRegistrationMessage.hpp>
#include <system/messages/RequestCpuFrequencyMessage.hpp>
#include <system/messages/HoldCpuFrequency.hpp>
#include <time/ScopedTime.hpp>
#include "Timers/TimerFactory.hpp"
#include <service-appmgr/StartupType.hpp>
#include <purefs/vfs_subsystem.hpp>
#include <service-gui/Common.hpp>
#include <service-db/DBServiceName.hpp>
#include <module-gui/gui/Common.hpp>
#include <service-eink/Common.hpp>

const inline size_t systemManagerStack = 4096 * 2;

namespace sys
{
    namespace
    {
        constexpr std::chrono::milliseconds preShutdownRoutineTimeout{1500};
        constexpr std::chrono::milliseconds lowBatteryShutdownDelayTime{5000};
    } // namespace

    namespace state
    {
        namespace restore
        {
            static constexpr std::array whitelist = {service::name::service_desktop,
                                                     service::name::evt_manager,
                                                     service::name::eink,
                                                     service::name::appmgr,
                                                     service::name::cellular};
        }

        namespace regularClose
        {
            static constexpr std::array whitelist = {service::name::evt_manager, service::name::cellular};
        }

        namespace update
        {
            static constexpr std::array whitelist = {service::name::evt_manager,
                                                     service::name::cellular,
                                                     service::name::eink,
                                                     service::name::service_desktop};
        }

        template <typename T> static bool isOnWhitelist(const T &list, const std::string &serviceName)
        {
            return std::find(std::begin(list), std::end(list), serviceName) != std::end(list);
        }

    } // namespace state

    using namespace cpp_freertos;
    using namespace std;
    using namespace sys;

    void SystemManagerCommon::set(enum State state)
    {
        LOG_DEBUG("System manager state: [%s] -> [%s]", c_str(this->state), c_str(state));
        this->state = state;
    }

    SystemManagerCommon::SystemManagerCommon(std::vector<std::unique_ptr<BaseServiceCreator>> &&creators)
        : Service(service::name::system_manager, "", systemManagerStack), systemServiceCreators{std::move(creators)}
    {
        // Specify list of channels which System Manager is registered to
        bus.channels            = {BusChannel::SystemManagerRequests};
        lowBatteryShutdownDelay = sys::TimerFactory::createPeriodicTimer(
            this, "lowBatteryShutdownDelay", lowBatteryShutdownDelayTime, [this](sys::Timer &) {
                CloseSystemHandler(CloseReason::LowBattery);
            });
    }

    SystemManagerCommon::~SystemManagerCommon()
    {
        LOG_DEBUG("%s", (GetName() + ":destructor").c_str());
    }

    void SystemManagerCommon::LogPowerOffReason()
    {
        // Power off system
        switch (state) {
        case SystemManagerCommon::State::Reboot:
            LOG_INFO("  --->  REBOOT <--- ");
            break;
        case SystemManagerCommon::State::ShutdownReady:
            LOG_INFO("  ---> SHUTDOWN <--- ");
            break;
        case SystemManagerCommon::State::RebootToUsbMscMode:
            LOG_INFO("  ---> REBOOT TO USB MSC Mode <--- ");
            break;
        case SystemManagerCommon::State::RebootToUpdate:
            LOG_INFO("  ---> REBOOT TO UPDATER <--- ");
            break;
        case SystemManagerCommon::State::Running:
        case SystemManagerCommon::State::Suspend:
        case SystemManagerCommon::State::Shutdown:
            LOG_FATAL("State changed after reset/shutdown was requested to: %s! this is terrible failure!",
                      c_str(state));
            break;
        }
    }

    void SystemManagerCommon::PowerOff()
    {
        switch (state) {
        case State::Reboot:
            powerManager->Reboot();
            break;
        case State::ShutdownReady:
            powerManager->PowerOff();
            break;
        case State::RebootToUsbMscMode:
            powerManager->RebootToUsbMscMode();
            break;
        case State::RebootToUpdate:
            powerManager->RebootToUpdate(updateReason);
            break;
        case SystemManagerCommon::State::Running:
        case SystemManagerCommon::State::Suspend:
        case SystemManagerCommon::State::Shutdown:
            exit(1);
        }
    }

    void SystemManagerCommon::Run()
    {
        initialize();

        // in shutdown we need to wait till event manager tells us that it's ok to stfu
        while (state == State::Running) {
            processBus();
        }

        while (state == State::Shutdown) {
            handleShutdown();
        }

        DestroySystemService(service::name::evt_manager, this);
        CloseService();

        // it should be called before systemDeinit to make sure this log is dumped to the file
        LogPowerOffReason();

        if (systemDeinit) {
            systemDeinit();
        }

        // We disable all uninitialized devices
        deviceManager->DisableAllDevices();

        // Power off request (pending)
        PowerOff();

        powerManager.reset();
        cpuStatistics.reset();
        deviceManager.reset();

        // End of scheduler and back to the main and poweroff
        EndScheduler();
    }

    void SystemManagerCommon::initialize()
    {
        utils::time::Scoped timer{"Initialize"};
        InitHandler();
        if (systemInit) {
            systemInit();
        }

        StartSystemServices();
        if (userInit) {
            userInit();
        }

        powerManager->SetBootSuccess();
    }

    void SystemManagerCommon::StartSystemServices()
    {
        DependencyGraph depGraph{graph::nodesFrom(systemServiceCreators), std::make_unique<graph::TopologicalSort>()};
        const auto &sortedServices = [&depGraph]() {
            utils::time::Scoped timer{"DependencyGraph"};
            return depGraph.sort();
        }();

        LOG_INFO("Order of system services initialization:");
        for (const auto &service : sortedServices) {
            LOG_INFO("\t> %s", service.get().getName().c_str());
        }
        std::for_each(sortedServices.begin(), sortedServices.end(), [this](const auto &service) {
            const auto startTimeout = service.get().getStartTimeout().count();
            if (const auto success = RunSystemService(service.get().create(), this, startTimeout); !success) {
                LOG_FATAL("Unable to start service: %s", service.get().getName().c_str());
                throw SystemInitialisationError{"System startup failed: unable to start a system service."};
            }
        });

        postStartRoutine();
    }

    void SystemManagerCommon::StartSystem(InitFunction sysInit, InitFunction appSpaceInit, DeinitFunction sysDeinit)
    {
        cpuStatistics = std::make_unique<CpuStatistics>();
        powerManager  = std::make_unique<PowerManager>(*cpuStatistics);
        deviceManager = std::make_unique<DeviceManager>();

        systemInit   = std::move(sysInit);
        userInit     = std::move(appSpaceInit);
        systemDeinit = std::move(sysDeinit);
        // Start System manager
        StartService();

        freqTimer = sys::TimerFactory::createPeriodicTimer(
            this, "cpuTick", constants::timerInitInterval, [this](sys::Timer &) { FreqUpdateTick(); });
        freqTimer.start();

        powerManagerEfficiencyTimer = sys::TimerFactory::createPeriodicTimer(
            this, "logPowerManagerEfficiency", constants::powerManagerLogsTimerInterval, [this](sys::Timer &) {
                powerManager->LogPowerManagerEfficiency();
            });
        powerManagerEfficiencyTimer.start();
    }

    bool SystemManagerCommon::Restore(Service *s)
    {
        LOG_DEBUG("trying to enter restore state");
        auto ret = s->bus.sendUnicastSync(std::make_shared<SystemManagerCmd>(Code::Restore),
                                          service::name::system_manager,
                                          sys::constants::restoreTimeout);
        if (ret.first != ReturnCodes::Success) {
            LOG_WARN("Can't stop all services, %d ms wait time", sys::constants::restoreTimeout);
        }
        return true;
    }

    bool SystemManagerCommon::FactoryReset(Service *s)
    {
        return s->bus.sendUnicast(std::make_shared<SystemManagerCmd>(Code::FactoryReset, CloseReason::FactoryReset),
                                  service::name::system_manager);
    }

    bool SystemManagerCommon::Reboot(Service *s)
    {
        s->bus.sendUnicast(std::make_shared<SystemManagerCmd>(Code::Reboot, CloseReason::Reboot),
                           service::name::system_manager);
        return true;
    }

    bool SystemManagerCommon::RebootToUsbMscMode(Service *s)
    {
        s->bus.sendUnicast(std::make_shared<SystemManagerCmd>(Code::RebootToUsbMscMode, CloseReason::Reboot),
                           service::name::system_manager);
        return true;
    }

    bool SystemManagerCommon::RebootToUpdate(Service *s, UpdateReason updateReason)
    {
        s->bus.sendUnicast(std::make_shared<SystemManagerCmd>(Code::RebootToUpdate, CloseReason::Reboot, updateReason),
                           service::name::system_manager);
        return true;
    }

    bool SystemManagerCommon::SuspendService(const std::string &name, sys::Service *caller)
    {
        auto ret = caller->bus.sendUnicastSync(
            std::make_shared<SystemMessage>(SystemMessageType::SwitchPowerMode, ServicePowerMode::SuspendToRAM),
            name,
            1000);
        auto resp = std::static_pointer_cast<ResponseMessage>(ret.second);

        if (ret.first != ReturnCodes::Success && (resp->retCode != ReturnCodes::Success)) {
            LOG_FATAL("Service %s failed to enter low-power mode", name.c_str());
        }
        return true;
    }

    bool SystemManagerCommon::ResumeService(const std::string &name, sys::Service *caller)
    {
        auto ret = caller->bus.sendUnicastSync(
            std::make_shared<SystemMessage>(SystemMessageType::SwitchPowerMode, ServicePowerMode::Active), name, 1000);
        auto resp = std::static_pointer_cast<ResponseMessage>(ret.second);

        if (ret.first != ReturnCodes::Success && (resp->retCode != ReturnCodes::Success)) {
            LOG_FATAL("Service %s failed to exit low-power mode", name.c_str());
        }
        return true;
    }

    bool SystemManagerCommon::RunService(std::shared_ptr<Service> service, Service *caller, TickType_t timeout)
    {
        service->StartService();

        auto msg  = std::make_shared<SystemMessage>(SystemMessageType::Start);
        auto ret  = caller->bus.sendUnicastSync(msg, service->GetName(), timeout);
        auto resp = std::static_pointer_cast<ResponseMessage>(ret.second);

        if (ret.first == ReturnCodes::Success && (resp->retCode == ReturnCodes::Success)) {
            return true;
        }
        return false;
    }

    bool SystemManagerCommon::RunSystemService(std::shared_ptr<Service> service, Service *caller, TickType_t timeout)
    {
        CriticalSection::Enter();
        servicesList.push_back(service);
        CriticalSection::Exit();

        return RunService(std::move(service), caller, timeout);
    }

    bool SystemManagerCommon::RunApplication(std::shared_ptr<app::ApplicationCommon> app,
                                             Service *caller,
                                             TickType_t timeout)
    {
        CriticalSection::Enter();
        applicationsList.push_back(app);
        CriticalSection::Exit();

        return RunService(std::move(app), caller, timeout);
    }

    bool SystemManagerCommon::RequestServiceClose(const std::string &name, Service *caller, TickType_t timeout)
    {
        auto msg  = std::make_shared<SystemMessage>(SystemMessageType::Exit);
        auto ret  = caller->bus.sendUnicastSync(msg, name, timeout);
        auto resp = std::static_pointer_cast<ResponseMessage>(ret.second);

        if (ret.first != ReturnCodes::Success) {
            LOG_ERROR("Service to close: %s did not respond, error code %d", name.c_str(), static_cast<int>(ret.first));
            return false;
        }
        else if (resp->retCode != ReturnCodes::Success) {
            LOG_ERROR(
                "Service %s noticed failure at close, error code %d", name.c_str(), static_cast<int>(resp->retCode));
            return false;
        }
        return true;
    }

    template <typename T> void SystemManagerCommon::DestroyServices(const T &whitelist)
    {
        cpp_freertos::LockGuard lck(serviceDestroyMutex);
        for (auto service = servicesList.begin(); service != servicesList.end();) {
            if (sys::state::isOnWhitelist<T>(whitelist, (*service)->GetName())) {
                LOG_DEBUG("Delay closing %s", (*service)->GetName().c_str());
                ++service;
            }
            else {
                LOG_DEBUG("RequestServiceClose %s", (*service)->GetName().c_str());
                if (!RequestServiceClose((*service)->GetName(), this)) {
                    LOG_ERROR("Service %s did not respond -> to kill", (*service)->GetName().c_str());
                    kill(*service);
                }
                service = servicesList.erase(service);
            }
        }
    }

    bool SystemManagerCommon::DestroySystemService(const std::string &name, Service *caller)
    {
        cpp_freertos::LockGuard lck(serviceDestroyMutex);
        if (RequestServiceClose(name, caller)) {
            auto service = std::find_if(servicesList.begin(),
                                        servicesList.end(),
                                        [&name](std::shared_ptr<Service> const &s) { return s->GetName() == name; });
            if (service == servicesList.end()) {
                LOG_ERROR("No such service to destroy in the list: %s", name.c_str());
                return false;
            }
            servicesList.erase(service);
            return true;
        }
        return false;
    }

    bool SystemManagerCommon::DestroyApplication(const std::string &name, Service *caller)
    {
        cpp_freertos::LockGuard lck(appDestroyMutex);
        if (RequestServiceClose(name, caller)) {
            auto app = std::find_if(
                applicationsList.begin(),
                applicationsList.end(),
                [&name](std::shared_ptr<app::ApplicationCommon> const &s) { return s->GetName() == name; });
            if (app == applicationsList.end()) {
                LOG_ERROR("No such application to destroy in the list: %s", name.c_str());
                return false;
            }
            applicationsList.erase(app);
            return true;
        }
        return false;
    }

    std::string SystemManagerCommon::ServiceProcessor(const uint32_t &t)
    {
        if (t == 0) {
            return "Idle";
        }

        auto foo = [](auto &l, const uint32_t &t) {
            auto found = std::find_if(l.begin(), l.end(), [&t](auto &r) {
                auto right = uxTaskGetTCBNumber(r->GetHandle());
                auto left  = t;
                return left == right;
            });
            return found;
        };

        if (auto found = foo(applicationsList, t); found != std::end(applicationsList)) {
            return (*found)->GetName() + "::" + (*found)->getCurrentProcessing();
        }
        if (auto found = foo(servicesList, t); found != std::end(servicesList)) {
            return (*found)->GetName() + "::" + (*found)->getCurrentProcessing();
        }

        auto handle = xTaskGetByTCBNumber(t);
        if (handle != nullptr) {
            return pcTaskGetTaskName(handle);
        }

        return "none";
    }

    void SystemManagerCommon::preCloseRoutine(CloseReason closeReason)
    {
        for (const auto &service : servicesList) {
            auto msg = std::make_shared<ServiceCloseReasonMessage>(closeReason);
            bus.sendUnicast(std::move(msg), service->GetName());
            readyForCloseRegister.push_back(service->GetName());
        }

        // stored to be used later in CloseServices
        this->closeReason = closeReason;

        servicesPreShutdownRoutineTimeout = sys::TimerFactory::createPeriodicTimer(
            this, "servicesPreShutdownRoutine", preShutdownRoutineTimeout, [this](sys::Timer &) { CloseServices(); });
        servicesPreShutdownRoutineTimeout.start();
    }

    void SystemManagerCommon::postStartRoutine()
    {
        connect(sevm::BatteryStateChangeMessage(), [&](Message *) {
            switch (Store::Battery::get().levelState) {
            case Store::Battery::LevelState::Normal:
                batteryNormalLevelAction();
                break;
            case Store::Battery::LevelState::Shutdown:
                batteryShutdownLevelAction();
                break;
            case Store::Battery::LevelState::CriticalCharging:
                batteryCriticalLevelAction(true);
                break;
            case Store::Battery::LevelState::CriticalNotCharging:
                batteryCriticalLevelAction(false);
                break;
            }
            return MessageNone{};
        });
    }

    void SystemManagerCommon::batteryCriticalLevelAction(bool charging)
    {
        LOG_INFO("Battery Critical Level reached!");
    }

    void SystemManagerCommon::batteryShutdownLevelAction()
    {
        LOG_INFO("Battery level too low - shutting down the system...");
        CloseSystemHandler(CloseReason::LowBattery);
    }

    void SystemManagerCommon::batteryNormalLevelAction()
    {
        LOG_INFO("Battery level normal.");
    }

    void SystemManagerCommon::readyToCloseHandler(Message *msg)
    {
        if (!readyForCloseRegister.empty() && servicesPreShutdownRoutineTimeout.isActive()) {
            auto message = static_cast<ReadyToCloseMessage *>(msg);
            LOG_INFO("ready to close %s", message->sender.c_str());
            readyForCloseRegister.erase(
                std::remove(readyForCloseRegister.begin(), readyForCloseRegister.end(), message->sender),
                readyForCloseRegister.end());

            // All services responded
            if (readyForCloseRegister.empty()) {
                LOG_INFO("All services ready to close.");
                servicesPreShutdownRoutineTimeout.stop();
                CloseServices();
            }
        }
    }

    void SystemManagerCommon::kill(std::shared_ptr<Service> const &toKill)
    {
        auto ret = toKill->DeinitHandler();
        if (ret != sys::ReturnCodes::Success) {
            LOG_DEBUG("deinit handler: %s", c_str(ret));
        }
        toKill->CloseHandler();
    }

    ReturnCodes SystemManagerCommon::InitHandler()
    {
        isReady = true;

        connect(SystemManagerCmd(), [&](Message *msg) {
            if (msg->channel == BusChannel::SystemManagerRequests) {
                auto *data = static_cast<SystemManagerCmd *>(msg);

                switch (data->type) {
                case Code::CloseSystem:
                    CloseSystemHandler(data->closeReason);
                    break;
                case Code::Restore:
                    RestoreSystemHandler();
                    break;
                case Code::Reboot:
                    RebootHandler();
                    break;
                case Code::RebootToUpdate:
                    RebootToUpdateHandler(data->updateReason);
                    break;
                case Code::RebootToUsbMscMode:
                    RebootToUsbMscModeHandler(State::RebootToUsbMscMode);
                    break;
                case Code::FactoryReset:
                    CloseSystemHandler(CloseReason::FactoryReset);
                    break;
                case Code::None:
                    break;
                }
            }

            return MessageNone{};
        });

        connect(sevm::BatteryStatusChangeMessage(), [&](Message *) {
            if ((state == State::Shutdown) && (Store::Battery::get().state == Store::Battery::State::Discharging)) {
                set(State::ShutdownReady);
            }
            return MessageNone{};
        });

        connect(sevm::KbdMessage(), [&](Message *) {
            // we are in shutdown mode - we received that there was red key pressed -> we need to reboot
            if (state == State::Shutdown) {
                set(State::Reboot);
            }
            return MessageNone{};
        });

        connect(sevm::BatteryBrownoutMessage(), [&](Message *) {
            LOG_INFO("Battery Brownout voltage level reached! Closing system...");
            CloseSystemHandler(CloseReason::SystemBrownout);
            return MessageNone{};
        });

        connect(app::UserPowerDownRequest(), [&](Message *) {
            CloseSystemHandler(CloseReason::RegularPowerDown);
            return MessageNone{};
        });

        connect(ReadyToCloseMessage(), [&](Message *msg) {
            readyToCloseHandler(msg);
            return MessageNone{};
        });

        connect(typeid(sys::DeviceRegistrationMessage), [this](sys::Message *message) -> sys::MessagePointer {
            auto msg = static_cast<sys::DeviceRegistrationMessage *>(message);
            deviceManager->RegisterNewDevice(msg->getDevice());

            return sys::MessageNone{};
        });

        connect(typeid(sys::SentinelRegistrationMessage), [this](sys::Message *message) -> sys::MessagePointer {
            auto msg = static_cast<sys::SentinelRegistrationMessage *>(message);
            powerManager->RegisterNewSentinel(msg->getSentinel());

            return sys::MessageNone{};
        });

        connect(typeid(sys::SentinelRemovalMessage), [this](sys::Message *message) -> sys::MessagePointer {
            auto msg = static_cast<sys::SentinelRemovalMessage *>(message);
            powerManager->RemoveSentinel(msg->getSentinelName());

            return sys::MessageNone{};
        });

        connect(typeid(sys::HoldCpuFrequencyMessage), [this](sys::Message *message) -> sys::MessagePointer {
            auto msg = static_cast<sys::HoldCpuFrequencyMessage *>(message);
            powerManager->SetCpuFrequencyRequest(msg->getName(), msg->getRequest());
            if (msg->getHandle() != nullptr) {
                xTaskNotifyGive(msg->getHandle());
            }
            return sys::MessageNone{};
        });

        connect(typeid(sys::ReleaseCpuFrequencyMessage), [this](sys::Message *message) -> sys::MessagePointer {
            auto msg = static_cast<sys::ReleaseCpuFrequencyMessage *>(message);
            powerManager->ResetCpuFrequencyRequest(msg->getName());

            return sys::MessageNone{};
        });

        connect(typeid(sys::IsCpuPernament), [this](sys::Message *message) -> sys::MessagePointer {
            return std::make_shared<sys::IsCpuPernamentResponse>(powerManager->IsCpuPernamentFrequency());
        });

        connect(typeid(sys::HoldCpuFrequencyPermanentlyMessage), [this](sys::Message *message) -> sys::MessagePointer {
            auto msg = static_cast<sys::HoldCpuFrequencyPermanentlyMessage *>(message);
            powerManager->SetPernamentFrequency(msg->request);
            return std::make_shared<sys::HoldCpuFrequencyPermanentlyResponse>();
        });

        connect(typeid(sys::ReleaseCpuPermanentFrequencyMessage), [this](sys::Message *message) -> sys::MessagePointer {
            powerManager->ResetPernamentFrequency();
            return std::make_shared<sys::HoldCpuFrequencyPermanentlyResponse>();
        });

        connect(typeid(app::manager::CheckIfStartAllowedMessage), [this](sys::Message *) -> sys::MessagePointer {
            switch (Store::Battery::get().levelState) {
            case Store::Battery::LevelState::Normal:
                bus.sendUnicast(std::make_unique<app::manager::StartAllowedMessage>(app::manager::StartupType::Regular),
                                service::name::appmgr);
                break;
            case Store::Battery::LevelState::Shutdown:
                if (!lowBatteryShutdownDelay.isActive()) {
                    lowBatteryShutdownDelay.start();
                }
                [[fallthrough]];
            case Store::Battery::LevelState::CriticalNotCharging:
                bus.sendUnicast(
                    std::make_unique<app::manager::StartAllowedMessage>(app::manager::StartupType::LowBattery),
                    service::name::appmgr);
                break;
            case Store::Battery::LevelState::CriticalCharging:
                bus.sendUnicast(
                    std::make_unique<app::manager::StartAllowedMessage>(app::manager::StartupType::LowBatteryCharging),
                    service::name::appmgr);
                break;
            }
            return sys::MessageNone{};
        });

        deviceManager->RegisterNewDevice(powerManager->getExternalRamDevice());

        cpuSentinel = std::make_shared<sys::CpuSentinel>(
            service::name::system_manager, this, [this](bsp::CpuFrequencyMHz newFrequency) {
                UpdateResourcesAfterCpuFrequencyChange(newFrequency);
            });
        powerManager->RegisterNewSentinel(cpuSentinel);

        return ReturnCodes::Success;
    }

    MessagePointer SystemManagerCommon::DataReceivedHandler(DataMessage * /*msg*/, ResponseMessage * /*resp*/)
    {
        return std::make_shared<ResponseMessage>();
    }

    void SystemManagerCommon::CloseSystemHandler(CloseReason closeReason)
    {
        LOG_DEBUG("Invoking closing procedure...");

        // In case if other power down request arrive in the meantime
        lowBatteryShutdownDelay.stop();
        freqTimer.stop();
        powerManagerEfficiencyTimer.stop();

        // We are going to remove services in reversed order of creation
        CriticalSection::Enter();
        std::reverse(servicesList.begin(), servicesList.end());
        CriticalSection::Exit();

        preCloseRoutine(closeReason);
    }

    void SystemManagerCommon::CloseServices()
    {
        for (const auto &element : readyForCloseRegister) {
            LOG_INFO("Service: %s did not reported before timeout", element.c_str());
        }
        // All delayed messages will be ignored
        readyForCloseRegister.clear();

        switch (closeReason) {
        case CloseReason::RegularPowerDown:
        case CloseReason::FactoryReset:
        case CloseReason::SystemBrownout:
        case CloseReason::LowBattery:
        case CloseReason::RebootToUsbMscMode:
            DestroyServices(sys::state::regularClose::whitelist);
            set(State::Shutdown);
            break;
        case CloseReason::Reboot:
            DestroyServices(sys::state::regularClose::whitelist);
            set(State::Reboot);
            break;
        case CloseReason::RebootToUpdate:
            DestroyServices(sys::state::update::whitelist);
            set(State::RebootToUpdate);
            break;
        }
    }

    void SystemManagerCommon::RestoreSystemHandler()
    {
        LOG_INFO("Entering restore system state");

        // We are going to remove services in reversed order of creation
        CriticalSection::Enter();
        std::reverse(servicesList.begin(), servicesList.end());
        CriticalSection::Exit();

        DestroyServices(sys::state::restore::whitelist);

        LOG_INFO("entered restore state");
    }

    void SystemManagerCommon::RebootHandler()
    {
        CloseSystemHandler(CloseReason::Reboot);
    }

    void SystemManagerCommon::RebootToUpdateHandler(UpdateReason updateReason)
    {
        CloseSystemHandler(CloseReason::RebootToUpdate);
        this->updateReason = updateReason;
    }

    void SystemManagerCommon::RebootToUsbMscModeHandler(State newState)
    {
        CloseSystemHandler(CloseReason::RebootToUsbMscMode);
        set(newState);
    }

    void SystemManagerCommon::FreqUpdateTick()
    {
        if (!cpuStatisticsTimerInit) {
            cpuStatisticsTimerInit = true;
            freqTimer.restart(constants::timerPeriodInterval);
        }

        auto ret = powerManager->UpdateCpuFrequency();
        cpuStatistics->TrackChange(ret);
    }

    void SystemManagerCommon::UpdateResourcesAfterCpuFrequencyChange(bsp::CpuFrequencyMHz newFrequency)
    {
        if (newFrequency <= bsp::CpuFrequencyMHz::Level_1) {
            purefs::subsystem::disk_mgr()->pm_control(purefs::blkdev::pm_state::suspend);
        }
        else {
            purefs::subsystem::disk_mgr()->pm_control(purefs::blkdev::pm_state::active);
        }
    }

    std::vector<std::shared_ptr<Service>> SystemManagerCommon::servicesList;
    std::vector<std::shared_ptr<app::ApplicationCommon>> SystemManagerCommon::applicationsList;
    cpp_freertos::MutexStandard SystemManagerCommon::serviceDestroyMutex;
    cpp_freertos::MutexStandard SystemManagerCommon::appDestroyMutex;
} // namespace sys
