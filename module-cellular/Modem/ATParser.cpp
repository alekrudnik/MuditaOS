﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ATParser.hpp"
#include <service-fota/FotaServiceAPI.hpp>
#include "bsp/cellular/bsp_cellular.hpp"
#include <service-cellular/CellularMessage.hpp>
#include "ticks.hpp"
#include <Utils.hpp>
#include <utility>
#include <vector>

ATParser::ATParser(bsp::Cellular *cellular) : Channel{new uint8_t[at::defaultReceiveBufferSize]}, cellular(cellular)
{
    responseBuffer = xMessageBufferCreate(at::defaultMessageBufferSize);
}

/// plz see 12.7 summary of urc in documentation
std::vector<ATParser::Urc> ATParser::ParseURC()
{
    std::vector<ATParser::Urc> resp;
    size_t maxPos = 0, pos = 0;

    cpp_freertos::LockGuard lock(mutex);

    std::vector<std::pair<std::string, ATParser::Urc>> vals = {
        {"RDY", ATParser::Urc::MeInitializationSuccessful},
        {"+CFUN: 1", ATParser::Urc::FullFuncionalityAvailable},
    };

    for (const auto &el : vals) {
        pos = urcBuffer.find(el.first);
        if (pos != std::string::npos) {
            resp.push_back(el.second);
            maxPos = std::max(pos + el.first.length(), maxPos);
            LOG_DEBUG("%s", ("[URC]: " + el.first).c_str());
        }
    }

    if (urcBuffer.find("+QIND: \"FOTA\"") != std::string::npos) {
        LOG_DEBUG("%s", urcBuffer.c_str());
        resp.push_back(ATParser::Urc::Fota);
        return resp;
    }

    // manage string buffer
    if (maxPos == 0) {}
    else if (urcBuffer.size() >= maxPos) {
        urcBuffer.erase();
    }
    else {
        urcBuffer = urcBuffer.substr(maxPos);
    }

    return resp;
}

at::Result ATParser::ProcessNewData(sys::Service *service, bsp::cellular::CellularResult *cellularResult)
{
    at::Result result;

    {
        cpp_freertos::LockGuard lock(mutex);
        urcBuffer.append(cellularResult->getDataAsString());
    }

    auto ret = ParseURC();

    if (awaitingResponseFlag.state()) {
        if (!xMessageBufferSend(responseBuffer,
                                (void *)cellularResult->getSerialized().get(),
                                cellularResult->getSerializedSize(),
                                pdMS_TO_TICKS(at::defaultBufferTimeoutMs.count()))) {
            LOG_DEBUG("[AT] Message buffer full!");
            result.code = at::Result::Code::FULL_MSG_BUFFER;
        }
    }
    else if (!ret.empty()) {
        if (ret.size() == 1 && ret[0] == ATParser::Urc::Fota) {
            std::string fotaData(urcBuffer);
            LOG_DEBUG("parsing FOTA:\"%s\"", fotaData.c_str());
            FotaService::API::sendRawProgress(service, fotaData);
            urcBuffer.erase();
        }
        else {
            urcs.insert(std::end(urcs), std::begin(ret), std::end(ret));
        }
        // GSM modem is considered as operational when it outputs URCs specified below:
        // 1) RDY
        // 2) +CFUN: 1
        if (urcs.size() == 2) {
            cpp_freertos::LockGuard lock(mutex);
            auto msg = std::make_shared<CellularPowerUpProcedureCompleteNotification>();
            service->bus.sendMulticast(msg, sys::BusChannel::ServiceCellularNotifications);
            urcBuffer.erase();
            urcs.clear();
        }
    }
    else {
        result.code = at::Result::Code::DATA_NOT_USED;
    }

    return result;
}

void ATParser::cmd_init()
{
    cpp_freertos::LockGuard lock(mutex);
    urcBuffer.erase();
}

void ATParser::cmd_send(std::string cmd)
{
    cellular->write(const_cast<char *>(cmd.c_str()), cmd.size());
}

size_t ATParser::cmd_receive(std::uint8_t *buffer, std::chrono::milliseconds timeout = std::chrono::milliseconds{0})
{
    return xMessageBufferReceive(responseBuffer, buffer, 256, pdMS_TO_TICKS(timeout.count()));
}

void ATParser::cmd_post()
{
    cpp_freertos::LockGuard lock(mutex);
    urcBuffer.erase();
    xMessageBufferReset(responseBuffer);
}
