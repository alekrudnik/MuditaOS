// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-appmgr/data/MmiActionsParams.hpp>

using namespace app::manager::actions;

void MMICustomResultParams::addMessage(const MMIResultMessage &message)
{
    messages.emplace_back(message);
}

auto MMICustomResultParams::getMessage() const -> std::vector<MMIResultMessage>
{
    return messages;
}

auto MMICustomResultParams::getMessageType() const noexcept -> MMIType
{
    return type;
}

void MMICustomResultParams::accept(Visitor &v, std::string &displayMessage)
{
    v.visit(*this, displayMessage);
};

void MMINoneSpecifiedResult::accept(Visitor &v, std::string &displayMessage)
{
    v.visit(*this, displayMessage);
};

auto MMICallForwardingResult::getData() const -> std::tuple<std::string, std::string, std::string, std::string>
{
    return std::make_tuple(voice, fax, sync, async);
}

void MMICallForwardingResult::accept(Visitor &v, std::string &displayMessage)
{
    v.visit(*this, displayMessage);
};

MMIParams::MMIParams(std::string mmiData) : mmiData{std::move(mmiData)}
{}

std::string MMIParams::getData() const
{
    return mmiData;
}

MMIResultParams::MMIResultParams(MMIResult result, std::shared_ptr<MMICustomResultParams> customResult)
    : result(result), customResult(std::move(customResult))
{}


MMIResultParams::MMIResult MMIResultParams::getData() const noexcept
{
    return result;
}

auto MMIResultParams::getCustomData() const noexcept -> std::shared_ptr<MMICustomResultParams>
{
    return customResult;
}
