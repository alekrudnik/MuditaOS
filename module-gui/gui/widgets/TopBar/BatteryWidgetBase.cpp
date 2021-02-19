// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "BatteryWidgetBase.hpp"
#include "BoxLayout.hpp"

namespace gui
{
    BatteryWidgetBase::BatteryWidgetBase(Item *parent, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
        : StatusBarWidgetBase(parent, x, y, w, h)
    {
        setBorderColor(gui::ColorNoColor);
    }

    void BatteryWidgetBase::update(const Store::Battery &batteryContext)
    {
        switch (batteryContext.state) {
        case Store::Battery::State::Discharging:
            [[fallthrough]];
        case Store::Battery::State::PluggedNotCharging:
            showBatteryLevel(batteryContext.level);
            break;
        case Store::Battery::State::Charging:
            showBatteryCharging();
            break;
        case Store::Battery::State::ChargingDone:
            showBatteryChargingDone();
            break;
        }
    }
} // namespace gui
