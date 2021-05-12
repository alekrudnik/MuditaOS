// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SimCard.hpp"
#include "utils.hpp"
#include "messages.hpp"
#include <service-cellular/api/request/sim.hpp>
#include <service-cellular/api/notification/notification.hpp>

#include <Service/Service.hpp>

#include <common_data/EventStore.hpp>
#include <bsp/cellular/bsp_cellular.hpp>
#include <modem/ATCommon.hpp>
#include <at/ATFactory.hpp>
#include <at/UrcFactory.hpp>
#include <at/UrcCpin.hpp>

namespace cellular::service
{
    namespace
    {
        constexpr const char *pinString[] = {"SC", "P2"};

        sim::Result convertErrorFromATResult(const at::Result atres)
        {
            if (std::holds_alternative<at::EquipmentErrorCode>(atres.errorCode)) {

                auto err = static_cast<int>(std::get<at::EquipmentErrorCode>(atres.errorCode));
                if ((err > static_cast<int>(sim::Result::AT_ERROR_Begin)) &&
                    (err < static_cast<int>(sim::Result::AT_ERROR_End))) {
                    return static_cast<sim::Result>(err);
                }
            }
            return sim::Result::Unknown;
        }

    } // namespace

    void SimCard::registerMessages()
    {
        using namespace ::cellular::msg;
        owner->connect(typeid(request::sim::SetActiveSim), [&](sys::Message *request) -> sys::MessagePointer {
            auto msg = static_cast<request::sim::SetActiveSim *>(request);
            /* That's EVIL !! */
            Store::GSM::get()->selected = static_cast<Store::GSM::SIM>(msg->sim);
            bsp::cellular::sim::simSelect();
            bsp::cellular::sim::hotSwapTrigger();
            //!!!!            sim = msg->sim;
            return std::make_shared<msg::Response>(true);
        });
        owner->connect(typeid(request::sim::GetLockState), [&](sys::Message *) -> sys::MessagePointer {
            return std::make_shared<request::sim::GetLockState::Response>(isPinLocked());
        });
        owner->connect(typeid(request::sim::ChangePin), [&](sys::Message *request) -> sys::MessagePointer {
            auto msg          = static_cast<request::sim::ChangePin *>(request);
            const auto oldPin = cellular::utils::pinToString(msg->oldPin);
            const auto pin    = cellular::utils::pinToString(msg->pin);
            return std::make_shared<request::sim::ChangePin::Response>(processPinResult(changePin(oldPin, pin)));
        });
        owner->connect(typeid(request::sim::UnblockWithPuk), [&](sys::Message *request) -> sys::MessagePointer {
            auto msg       = static_cast<request::sim::UnblockWithPuk *>(request);
            const auto puk = cellular::utils::pinToString(msg->puk);
            const auto pin = cellular::utils::pinToString(msg->pin);
            return std::make_shared<request::sim::UnblockWithPuk::Response>(processPinResult(supplyPuk(puk, pin)));
        });
        owner->connect(typeid(request::sim::SetPinLock), [&](sys::Message *request) -> sys::MessagePointer {
            auto msg       = static_cast<request::sim::SetPinLock *>(request);
            const auto pin = cellular::utils::pinToString(msg->pin);
            return std::make_shared<request::sim::SetPinLock::Response>(
                processPinResult(setPinLock(msg->lock == cellular::api::SimLockState::Locked, pin)), msg->lock);
        });
        owner->connect(typeid(request::sim::PinUnlock), [&](sys::Message *request) -> sys::MessagePointer {
            auto msg       = static_cast<request::sim::PinUnlock *>(request);
            const auto pin = cellular::utils::pinToString(msg->pin);
            return std::make_shared<request::sim::PinUnlock::Response>(processPinResult(supplyPin(pin)));
        });
        owner->connect(typeid(internal::msg::SimStateChanged), [&](sys::Message *request) -> sys::MessagePointer {
            auto msg = static_cast<internal::msg::SimStateChanged *>(request);
            handleSimState(msg->state);
            return sys::MessageNone{};
        });
    }

    bool SimCard::ready() const
    {
        return channel;
    }

    void SimCard::selectSim(api::SimSlot slot)
    {
        sim = slot;
    }

    void SimCard::resetChannel(at::Channel *channel)
    {
        this->channel = channel;
    }

    void SimCard::handleSimState(at::SimState state)
    {
        switch (state) {
        case at::SimState::Ready:
            Store::GSM::get()->sim = Store::GSM::get()->selected;
            if (onSimReady)
                onSimReady(true);
            break;
        case at::SimState::NotReady:
            Store::GSM::get()->sim = Store::GSM::SIM::SIM_FAIL;
            if (onSimReady)
                onSimReady(false);
            break;
        case at::SimState::SimPin:
            [[fallthrough]];
        case at::SimState::SimPin2: {
            if (auto pc = getAttemptsCounters(state == at::SimState::SimPin ? sim::Pin::PIN1 : sim::Pin::PIN2); pc) {
                if (pc.value().PukCounter != 0) {
                    if (onNeedPin)
                        onNeedPin(pc.value().PinCounter);
                    break;
                }
            }
            if (onSimBlocked)
                onSimBlocked();
            break;
        }
        case at::SimState::SimPuk:
            [[fallthrough]];
        case at::SimState::SimPuk2: {
            if (auto pc = getAttemptsCounters(state == at::SimState::SimPuk ? sim::Pin::PIN1 : sim::Pin::PIN2); pc) {
                if (pc.value().PukCounter != 0) {
                    if (onNeedPuk)
                        onNeedPuk(pc.value().PukCounter);
                    break;
                }
            }
            if (onSimBlocked)
                onSimBlocked();
            break;
        }
        case at::SimState::Locked:
            Store::GSM::get()->sim = Store::GSM::SIM::SIM_FAIL;
            if (onSimBlocked)
                onSimBlocked();
            break;
        case at::SimState::PhNetPin:
            [[fallthrough]];
        case at::SimState::PhNetPuk:
            [[fallthrough]];
        case at::SimState::PhNetSPin:
            [[fallthrough]];
        case at::SimState::PhNetSPuk:
            [[fallthrough]];
        case at::SimState::PhSpPin:
            [[fallthrough]];
        case at::SimState::PhSpPuk:
            [[fallthrough]];
        case at::SimState::PhCorpPin:
            [[fallthrough]];
        case at::SimState::PhCorpPuk:
            [[fallthrough]];
        case at::SimState::Unknown:
            LOG_ERROR("SimState not supported");
            Store::GSM::get()->sim = Store::GSM::SIM::SIM_UNKNOWN;
            break;
        }
        if (onSimEvent)
            onSimEvent();
    }

    std::optional<at::response::qpinc::AttemptsCounters> SimCard::getAttemptsCounters(sim::Pin pin) const
    {
        if (!ready()) {
            return std::nullopt;
        }

        auto resp = channel->cmd(at::factory(at::AT::QPINC) + "\"" + pinString[static_cast<int>(pin)] + "\"");
        at::response::qpinc::AttemptsCounters ret;
        if (at::response::parseQPINC(resp, ret)) {
            return ret;
        }

        return std::nullopt;
    }

    bool SimCard::processPinResult(sim::Result result)
    {
        if (result == sim::Result::IncorrectPassword) {
            if (auto state = simState(); state) {
                handleSimState(*state);
            }
        }
        else if (result != sim::Result::OK) {
            if (onUnhandledCME)
                onUnhandledCME(static_cast<unsigned int>(result));
        }
        return result == sim::Result::OK;
    }

    sim::Result SimCard::supplyPin(const std::string &pin) const
    {
        return sendCommand(sim::LockType::PIN, at::factory(at::AT::CPIN) + "\"" + pin + "\"");
    }

    sim::Result SimCard::changePin(const std::string &oldPin, const std::string &newPin) const
    {
        return sendCommand(sim::LockType::PIN,
                           at::factory(at::AT::CPWD) + "\"SC\", \"" + oldPin + "\",\"" + newPin + "\"");
    }

    sim::Result SimCard::supplyPuk(const std::string &puk, const std::string &pin) const
    {
        return sendCommand(sim::LockType::PUK, at::factory(at::AT::CPIN) + "\"" + puk + "\"" + ",\"" + pin + "\"");
    }

    sim::Result SimCard::setPinLock(bool lock, const std::string &pin) const
    {
        return sendCommand(sim::LockType::PIN,
                           at::factory(at::AT::CLCK) + "\"SC\"," + (lock ? "1" : "0") + ",\"" + pin + "\"");
    }

    bool SimCard::isPinLocked() const
    {
        auto resp = channel->cmd(at::factory(at::AT::CLCK) + "\"SC\",2\r");
        int val   = 0;
        if (at::response::parseCLCK(resp, val)) {
            return val != 0;
        }
        return true;
    }

    std::optional<at::SimState> SimCard::simState() const
    {
        auto resp = channel->cmd(at::factory(at::AT::GET_CPIN));
        if (resp.code == at::Result::Code::OK) {
            if (resp.response.size()) {
                for (auto el : resp.response) {
                    auto urc = at::urc::UrcFactory::Create(el);
                    if (auto cpin = dynamic_cast<at::urc::Cpin *>(urc.get())) {
                        return cpin->getState();
                    }
                }
            }
        }
        return at::SimState::Unknown;
    }

    sim::Result SimCard::sendCommand(sim::LockType check, const at::Cmd &cmd) const
    {
        if (auto pc = getAttemptsCounters(); pc) {
            switch (check) {
            case sim::LockType::PIN:
                if (pc.value().PinCounter == 0)
                    return sim::Result::Locked;
                break;
            case sim::LockType::PUK:
                if (pc.value().PukCounter == 0)
                    return sim::Result::Locked;
                break;
            }
        }
        else {
            return sim::Result::Unknown;
        }

        if (auto resp = channel->cmd(cmd); resp.code != at::Result::Code::OK) {
            return convertErrorFromATResult(resp);
        }

        return sim::Result::OK;
    }

} // namespace cellular::service
