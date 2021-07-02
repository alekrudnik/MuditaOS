// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ATCommon.hpp"
#include <at/Cmd.hpp>
#include "time/ScopedTime.hpp"
#include <functional>
#include <log.hpp>
#include <string>
#include <Utils.hpp>
#include "ATStream.hpp"
#include <at/ATFactory.hpp>

using namespace at;
using namespace std::chrono_literals;

const std::string Channel::OK         = "OK";
const std::string Channel::ERROR      = "ERROR";
const std::string Channel::NO_CARRIER = "NO CARRIER";
const std::string Channel::BUSY       = "BUSY";
const std::string Channel::NO_ANSWER  = "NO ANSWER";
const std::string Channel::CME_ERROR  = "+CME ERROR:";
const std::string Channel::CMS_ERROR  = "+CMS ERROR:";
// const std::string Channel::CONNECT = "CONNECT";
// const std::string Channel::RING = "RING";
// const std::string Channel::NO_DIALTONE = "NO DIALTONE";

void Channel::cmdLog(std::string cmd, const Result &result, std::chrono::milliseconds timeout)
{
    cmd.erase(std::remove(cmd.begin(), cmd.end(), '\r'), cmd.end());
    cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
    switch (result.code) {
    case Result::Code::TIMEOUT: {
        LOG_ERROR("[AT]: >%s<, timeout %s - please check the value with Quectel_EC25&EC21_AT_Commands_Manual_V1.3.pdf",
                  cmd.c_str(),
                  utils::to_string(timeout.count()).c_str());
    } break;
    case Result::Code::ERROR: {
        LOG_ERROR("[AT]: >%s<, >%s<", cmd.c_str(), !result.response.empty() ? result.response.back().c_str() : "");
    } break;
    default:
        LOG_INFO("[AT]: >%s<, >%s<", cmd.c_str(), !result.response.empty() ? result.response.back().c_str() : "");
        break;
    }
    for (const auto &s : result.response) {
        LOG_INFO("[AT] > %s", s.c_str());
    }
}

std::string Channel::formatCommand(const std::string &cmd) const
{
    bool isTerminatorValid = std::find(validTerm.begin(), validTerm.end(), cmd.back()) != validTerm.end();
    if (isTerminatorValid) {
        return cmd;
    }
    return cmd + cmdSeparator;
}

Result Channel::cmd(const std::string &cmd, std::chrono::milliseconds timeout, size_t rxCount)
{
    Result result;
    ATStream atStream(rxCount);

    awaitingResponseFlag.set();

    cmdInit();
    std::string cmdFixed = formatCommand(cmd);
    cmdSend(cmdFixed);

    auto startTime = std::chrono::steady_clock::now();
    auto endTime   = startTime + timeout;

    while (true) {
        if (std::chrono::steady_clock::now() > endTime) {
            result.code = Result::Code::TIMEOUT;
            break;
        }

        if (size_t bytesRead = cmdReceive(receiveBuffer.get(), 0ms); bytesRead > 0) {
            auto cellularResult = bsp::cellular::CellularResult{receiveBuffer.get(), bytesRead};

            if (result = checkResult(cellularResult.getResultCode()); result.code != at::Result::Code::OK) {
                break;
            }

            atStream.write(cellularResult.getDataAsString());

            if (atStream.isReady()) {
                result = atStream.getResult();
                break;
            }
        }
    }

    awaitingResponseFlag.clear();

    cmdPost();
    cmdLog(cmdFixed, result, timeout);

    return result;
}

auto Channel::cmd(const at::Cmd &at) -> Result
{
    auto time = utils::time::Scoped("Time to run at command" + at.getCmd());
    return cmd(at.getCmd(), at.getTimeout());
}

auto Channel::cmd(const at::AT &at) -> Result
{
    auto cmd  = at::factory(at);
    auto time = utils::time::Scoped("Time to run at command" + cmd.getCmd());
    return this->cmd(cmd);
}

Result Channel::checkResult(bsp::cellular::CellularResultCode cellularResult)
{
    Result result;

    switch (cellularResult) {
    case bsp::cellular::CellularResultCode::ReceivingNotStarted:
        result.code = Result::Code::RECEIVING_NOT_STARTED;
        break;
    case bsp::cellular::CellularResultCode::TransmittingNotStarted:
        result.code = Result::Code::TRANSMISSION_NOT_STARTED;
        break;
    case bsp::cellular::CellularResultCode::CMUXFrameError:
        result.code = Result::Code::CMUX_FRAME_ERROR;
        break;
    default:
        result.code = Result::Code::OK;
        break;
    }

    return result;
}
