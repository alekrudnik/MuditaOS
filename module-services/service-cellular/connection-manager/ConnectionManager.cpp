// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <module-services/service-cellular/service-cellular/connection-manager/ConnectionManager.hpp>
#include <module-services/service-cellular/service-cellular/CellularMessage.hpp>
#include <module-cellular/at/Cmd.hpp>

auto ConnectionManager::onPhoneModeChange(sys::phone_modes::PhoneMode mode) -> bool
{
    if (mode == sys::phone_modes::PhoneMode::Offline) {
        return handleModeChangeToCommonOffline();
    }
    return handleModeChangeToConnected();
}

void ConnectionManager::onTimerTick()
{
    if (connectionInterval.count() == 0 || isFlightMode) {
        return;
    }

    minutesOfflineElapsed++;
    if (minutesOfflineElapsed.count() >= connectionInterval.count()) {
        minutesOfflineElapsed = static_cast<std::chrono::minutes>(0);
        ;
        onlinePeriod = true;
        cellular->connectToNetwork();
        return;
    }
    if (onlinePeriod) {
        minutesOnlineElapsed++;
        if (minutesOnlineElapsed.count() >= connectedPeriod.count()) {
            minutesOnlineElapsed = static_cast<std::chrono::minutes>(0);
            onlinePeriod         = false;
            cellular->disconnectFromNetwork();
        }
    }
}

void ConnectionManager::setInterval(const std::chrono::minutes interval)
{
    connectionInterval   = interval;
    minutesOnlineElapsed = static_cast<std::chrono::minutes>(0);
    ;
    minutesOfflineElapsed = static_cast<std::chrono::minutes>(0);
    ;
}

void ConnectionManager::setFlightMode(const bool mode)
{
    isFlightMode = mode;
}

auto ConnectionManager::isMessagesOnlyMode() -> bool
{
    return !isFlightMode && connectionInterval.count() != 0;
}

auto ConnectionManager::handleModeChangeToCommonOffline() -> bool
{
    if (cellular->isConnectedToNetwork()) {
        cellular->hangUpOngoingCall();
        cellular->disconnectFromNetwork();
        cellular->clearNetworkIndicator();
    }

    minutesOfflineElapsed = static_cast<std::chrono::minutes>(0);
    ;
    minutesOnlineElapsed = static_cast<std::chrono::minutes>(0);
    ;

    if (isMessagesOnlyMode()) {
        handleModeChangeToMessageOnlyMode();
        return true;
    }
    else {
        handleModeChangeToOfflineMode();
        return true;
    }
}

void ConnectionManager::handleModeChangeToOfflineMode()
{
    if (cellular->isConnectionTimerActive()) {
        cellular->stopConnectionTimer();
        onlinePeriod = false;
    }
}

void ConnectionManager::handleModeChangeToMessageOnlyMode()
{
    if (!cellular->isConnectionTimerActive()) {
        cellular->startConnectionTimer();
    }
}

auto ConnectionManager::handleModeChangeToConnected() -> bool
{
    if (cellular->isConnectionTimerActive()) {
        cellular->stopConnectionTimer();
        onlinePeriod = false;
    }
    if (!cellular->isConnectedToNetwork()) {
        cellular->connectToNetwork();
    }
    return true;
}
