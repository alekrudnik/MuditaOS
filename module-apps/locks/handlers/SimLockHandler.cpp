//// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
//// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SimLockHandler.hpp"

#include <service-appmgr/service-appmgr/Controller.hpp>
#include <locks/widgets/LockHash.hpp>
#include <Utils.hpp>
#include <memory>

#include <module-apps/popups/data/PopupRequestParams.hpp>
#include <module-services/service-cellular/include/service-cellular-api>

namespace locks
{
    constexpr inline auto serviceCellular = "ServiceCellular";

    constexpr unsigned int default_attempts = 4;
    constexpr unsigned int max_input_size   = 8;
    constexpr unsigned int min_input_size   = 4;

    SimLockHandler::SimLockHandler(sys::Service *owner)
        : owner(owner), lock(Lock::LockState::Unlocked, default_attempts)
    {
        lock.setInputSizeBounds(min_input_size, max_input_size);
    }

    void SimLockHandler::clearStoredInputs()
    {
        storedFirstInput.clear();
        storedSecondInput.clear();
    }

    void SimLockHandler::setSimInputTypeAction(SimInputTypeAction _simInputTypeAction)
    {
        if (simInputTypeAction != _simInputTypeAction) {
            simInputTypeAction = _simInputTypeAction;
            lock.lockState     = Lock::LockState::Unlocked;
        }
    }

    void SimLockHandler::simInputRequiredAction()
    {
        app::manager::Controller::sendAction(
            owner,
            app::manager::actions::ShowPopup,
            std::make_unique<gui::SimUnlockInputRequestParams>(gui::popup::ID::SimUnlock, lock, simInputTypeAction));
    }

    void SimLockHandler::simErrorAction(unsigned int errorCode)
    {
        app::manager::Controller::sendAction(owner,
                                             app::manager::actions::ShowPopup,
                                             std::make_unique<gui::SimUnlockInputRequestParams>(
                                                 gui::popup::ID::SimUnlock, lock, simInputTypeAction, errorCode));
    }

    void SimLockHandler::simUnlockAction()
    {
        app::manager::Controller::sendAction(owner,
                                             app::manager::actions::AbortPopup,
                                             std::make_unique<gui::PopupRequestParams>(gui::popup::ID::SimUnlock));
    }

    void SimLockHandler::simInfoAction()
    {
        app::manager::Controller::sendAction(
            owner,
            app::manager::actions::ShowPopup,
            std::make_unique<gui::SimUnlockInputRequestParams>(gui::popup::ID::SimInfo, lock, simInputTypeAction));
    }

    void SimLockHandler::getSettingsSimSelect(const std::string &settingsSim)
    {
        auto selectedSim = magic_enum::enum_cast<Store::GSM::SIM>(settingsSim);
        if (selectedSim.has_value()) {
            // TODO check that
            Store::GSM::get()->selected = selectedSim.value();
            auto arg =
                (selectedSim == Store::GSM::SIM::SIM2) ? cellular::api::SimSlot::SIM2 : cellular::api::SimSlot::SIM1;
            owner->bus.sendUnicast<cellular::msg::request::sim::SetActiveSim>(arg);
        }
        else {
            Store::GSM::get()->selected = Store::GSM::SIM::NONE;
        }
    }

    sys::MessagePointer SimLockHandler::handleSimPinRequest(unsigned int attempts)
    {
        setSimInputTypeAction(SimInputTypeAction::UnlockWithPin);
        lock.attemptsLeft = attempts;

        if (simUnlockBlockOnLockedPhone) {
            return sys::msgNotHandled();
        }

        if (lock.isState(Lock::LockState::Unlocked)) {
            lock.lockState = Lock::LockState::InputRequired;
        }
        else if (lock.isState(Lock::LockState::InputRequired)) {
            lock.lockState = Lock::LockState::InputInvalid;
        }

        simInputRequiredAction();
        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimPukRequest(unsigned int attempts)
    {
        setSimInputTypeAction(SimInputTypeAction::UnlockWithPuk);
        lock.attemptsLeft = attempts;

        if (simUnlockBlockOnLockedPhone) {
            return sys::msgNotHandled();
        }

        clearStoredInputs();

        if (lock.isState(Lock::LockState::Unlocked)) {
            lock.lockState = Lock::LockState::InputRequired;
        }
        else if (lock.isState(Lock::LockState::InputRequired) ||
                 lock.isState(Lock::LockState::NewInputConfirmRequired)) {
            lock.lockState = Lock::LockState::InputInvalid;
        }

        simInputRequiredAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimUnlockedMessage()
    {
        lock.lockState = Lock::LockState::Unlocked;

        if (simInputTypeAction == SimInputTypeAction::UnlockWithPuk ||
            simInputTypeAction == SimInputTypeAction::ChangePin) {
            simInfoAction();
        }

        simUnlockAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimChangedMessage()
    {
        lock.lockState = Lock::LockState::Unlocked;
        simInfoAction();
        simUnlockAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimAvailabilityMessage()
    {
        lock.lockState = Lock::LockState::Unlocked;
        simInfoAction();
        simUnlockAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimPinChangeRequest()
    {
        setSimInputTypeAction(SimInputTypeAction::ChangePin);

        lock.lockName  = utils::enumToString(Store::GSM::get()->selected);
        lock.lockState = Lock::LockState::InputRequired;
        simInputRequiredAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimEnableRequest()
    {
        setSimInputTypeAction(SimInputTypeAction::EnablePin);

        lock.lockName  = utils::enumToString(Store::GSM::get()->selected);
        lock.lockState = Lock::LockState::InputRequired;
        simInputRequiredAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimDisableRequest()
    {
        setSimInputTypeAction(SimInputTypeAction::DisablePin);

        lock.lockName  = utils::enumToString(Store::GSM::get()->selected);
        lock.lockState = Lock::LockState::InputRequired;
        simInputRequiredAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleSimBlockedRequest()
    {
        setSimInputTypeAction(SimInputTypeAction::Blocked);

        if (simUnlockBlockOnLockedPhone) {
            return sys::msgNotHandled();
        }

        lock.lockName  = utils::enumToString(Store::GSM::get()->selected);
        lock.lockState = Lock::LockState::Blocked;
        simInputRequiredAction();

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::handleCMEErrorRequest(unsigned int errorCode)
    {
        setSimInputTypeAction(SimInputTypeAction::Error);

        if (simUnlockBlockOnLockedPhone) {
            storedErrorCode = errorCode;
            return sys::msgNotHandled();
        }

        lock.lockName  = utils::enumToString(Store::GSM::get()->selected);
        lock.lockState = Lock::LockState::ErrorOccurred;
        simErrorAction(errorCode);

        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::processLockWithNewInput(const std::vector<unsigned int> &inputData)
    {
        if (lock.isState(Lock::LockState::InputRequired) || (lock.isState(Lock::LockState::InputInvalid))) {

            storedFirstInput = inputData;
            lock.lockState   = Lock::LockState::NewInputRequired;
            simInputRequiredAction();
        }
        else if (lock.isState(Lock::LockState::NewInputRequired) || lock.isState(Lock::LockState::NewInputInvalid)) {

            storedSecondInput = inputData;
            lock.lockState    = Lock::LockState::NewInputConfirmRequired;
            simInputRequiredAction();
        }
        else if (lock.isState(Lock::LockState::NewInputConfirmRequired)) {

            if (storedSecondInput == inputData) {
                return resolveNewInputAction(storedFirstInput, inputData);
            }
            else {
                lock.lockState = Lock::LockState::NewInputInvalid;
                simInputRequiredAction();
            }
        }
        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::verifySimLockInput(const std::vector<unsigned int> &inputData)
    {
        switch (simInputTypeAction) {
        case SimInputTypeAction::UnlockWithPin:
            return unlockSimWithPin(inputData);
        case SimInputTypeAction::UnlockWithPuk:
            return processLockWithNewInput(inputData);
        case SimInputTypeAction::ChangePin:
            return processLockWithNewInput(inputData);
        case SimInputTypeAction::EnablePin:
            return enableSimPin(inputData);
        case SimInputTypeAction::DisablePin:
            return disableSimPin(inputData);
        default:
            return sys::msgNotHandled();
        }
    }

    void SimLockHandler::setSimUnlockBlockOnLockedPhone()
    {
        simUnlockBlockOnLockedPhone = true;
    }

    sys::MessagePointer SimLockHandler::releaseSimUnlockBlockOnLockedPhone()
    {
        if (simUnlockBlockOnLockedPhone) {
            simUnlockBlockOnLockedPhone = false;
            if (simInputTypeAction == SimInputTypeAction::UnlockWithPin) {
                return handleSimPinRequest(lock.getAttemptsLeft());
            }
            else if (simInputTypeAction == SimInputTypeAction::UnlockWithPuk) {
                return handleSimPukRequest(lock.getAttemptsLeft());
            }
            else if (simInputTypeAction == SimInputTypeAction::Blocked) {
                return handleSimBlockedRequest();
            }
            else if (simInputTypeAction == SimInputTypeAction::Error) {
                return handleCMEErrorRequest(storedErrorCode);
            }
        }
        return sys::msgNotHandled();
    }

    sys::MessagePointer SimLockHandler::resolveNewInputAction(const std::vector<unsigned int> &firstInputData,
                                                              const std::vector<unsigned int> &secondInputData)
    {
        if (simInputTypeAction == SimInputTypeAction::UnlockWithPuk) {
            return unlockSimWithPuk(firstInputData, secondInputData);
        }
        else if (simInputTypeAction == SimInputTypeAction::ChangePin) {
            return changeSimPin(firstInputData, secondInputData);
        }
        return sys::msgNotHandled();
    }

    sys::MessagePointer SimLockHandler::unlockSimWithPin(const std::vector<unsigned int> &pinInputData)
    {
        owner->bus.sendUnicast(std::make_shared<cellular::msg::request::sim::PinUnlock>(pinInputData), serviceCellular);
        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::unlockSimWithPuk(const std::vector<unsigned int> &pukInputData,
                                                         const std::vector<unsigned int> &newPinInputData)
    {
        owner->bus.sendUnicast(
            std::make_shared<cellular::msg::request::sim::UnblockWithPuk>(pukInputData, newPinInputData),
            serviceCellular);
        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::changeSimPin(const std::vector<unsigned int> &oldPinInputData,
                                                     const std::vector<unsigned int> &newPinInputData)
    {
        owner->bus.sendUnicast(
            std::make_shared<cellular::msg::request::sim::ChangePin>(oldPinInputData, newPinInputData),
            serviceCellular);
        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::enableSimPin(const std::vector<unsigned int> &pinInputData)
    {
        owner->bus.sendUnicast(std::make_shared<cellular::msg::request::sim::SetPinLock>(
                                   cellular::api::SimLockState::Enabled, pinInputData),
                               serviceCellular);
        return sys::msgHandled();
    }

    sys::MessagePointer SimLockHandler::disableSimPin(const std::vector<unsigned int> &pinInputData)
    {
        owner->bus.sendUnicast(std::make_shared<cellular::msg::request::sim::SetPinLock>(
                                   cellular::api::SimLockState::Disabled, pinInputData),
                               serviceCellular);
        return sys::msgHandled();
    }
} // namespace locks
