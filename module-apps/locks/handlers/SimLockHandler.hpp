// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <locks/widgets/Lock.hpp>
#include <locks/data/SimLockMessages.hpp>
#include <locks/data/LockData.hpp>

#include <module-sys/Service/Service.hpp>
#include <service-appmgr/service-appmgr/data/SimActionsParams.hpp>

namespace locks
{
    class SimLockHandler
    {
      private:
        sys::Service *owner;
        Lock lock;

        SimInputTypeAction simInputTypeAction = SimInputTypeAction::UnlockWithPin;
        Store::GSM::SIM sim       = Store::GSM::SIM::NONE;
        app::manager::actions::SimLockParams storedUnlockSimData;
        unsigned int storedErrorCode     = 0;
        bool simUnlockBlockOnLockedPhone = false;
        std::vector<unsigned int> storedFirstInput;
        std::vector<unsigned int> storedSecondInput;

        void getCellularSimValues(const app::manager::actions::SimLockParams &data);
        void clearStoredInputs();
        void setSimInputTypeAction(SimInputTypeAction _simInputTypeAction);

        void simInputRequiredAction();
        void simErrorAction(unsigned int errorCode);
        void simUnlockAction();
        void simInfoAction();

        sys::MessagePointer unlockSimWithPin(const std::vector<unsigned int> &pinInputData);
        sys::MessagePointer processLockWithNewInput(const std::vector<unsigned int> &inputData);
        sys::MessagePointer resolveNewInputAction(const std::vector<unsigned int> &firstInputData,
                                                  const std::vector<unsigned int> &secondInputData);
        sys::MessagePointer unlockSimWithPuk(const std::vector<unsigned int> &pukInputData,
                                             const std::vector<unsigned int> &newPinInputData);
        sys::MessagePointer changeSimPin(const std::vector<unsigned int> &oldPinInputData,
                                         const std::vector<unsigned int> &newPinInputData);
        sys::MessagePointer enableSimPin(const std::vector<unsigned int> &pinInputData);
        sys::MessagePointer disableSimPin(const std::vector<unsigned int> &pinInputData);

      public:
        explicit SimLockHandler(sys::Service *owner);

        void setSimUnlockBlockOnLockedPhone();
        sys::MessagePointer releaseSimUnlockBlockOnLockedPhone();

        sys::MessagePointer verifySimLockInput(const std::vector<unsigned int> &inputData);

        sys::MessagePointer handleSimPinRequest(const app::manager::actions::SimLockParams &data);
        sys::MessagePointer handleSimPukRequest(const app::manager::actions::SimLockParams &data);
        sys::MessagePointer handleSimPinChangeRequest();
        sys::MessagePointer handleSimEnableRequest();
        sys::MessagePointer handleSimDisableRequest();
        sys::MessagePointer handleSimBlockedRequest();
        sys::MessagePointer handleCMEErrorRequest(unsigned int errorCode);

        sys::MessagePointer handleSimUnlockedMessage(const app::manager::actions::SimStateParams &data);
        sys::MessagePointer handleSimChangedMessage();
        sys::MessagePointer handleSimAvailabilityMessage();

        void getSettingsSimSelect(const std::string &settingsSim);
    };

} // namespace locks
