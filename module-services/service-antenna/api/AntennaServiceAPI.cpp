﻿// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-antenna/AntennaServiceAPI.hpp>

#include <service-antenna/AntennaMessage.hpp>
#include <service-antenna/ServiceAntenna.hpp>

#include <MessageType.hpp>
#include <Service/Common.hpp>
#include <Service/Message.hpp>

#include <memory>
#include <utility>

namespace sys
{
    class Service;
} // namespace sys

namespace AntennaServiceAPI
{

    void CSQChange(sys::Service *serv)
    {
        auto msg = std::make_shared<sys::DataMessage>(MessageType::AntennaCSQChange);
        serv->bus.sendMulticast(msg, sys::BusChannel::AntennaNotifications);
    }
    bool LockRequest(sys::Service *serv, antenna::lockState request)
    {
        auto msg = std::make_shared<AntennaLockRequestMessage>(MessageType::AntennaLockService, request);
        auto ret = serv->bus.sendUnicast(msg, ServiceAntenna::serviceName, 5000);
        if (ret.first == sys::ReturnCodes::Success) {

            return true;
        }

        return false;
    }
    bool GetLockState(sys::Service *serv, antenna::lockState &response)
    {
        auto msg = std::make_shared<AntennaLockRequestMessage>(MessageType::AntennaGetLockState);
        auto ret = serv->bus.sendUnicast(msg, ServiceAntenna::serviceName, 5000);
        if (ret.first == sys::ReturnCodes::Success) {
            auto responseMsg = dynamic_cast<AntennaLockRequestResponse *>(ret.second.get());
            if (responseMsg != nullptr) {
                response = responseMsg->data;
                return true;
            }
        }
        return false;
    }
} // namespace AntennaServiceAPI
