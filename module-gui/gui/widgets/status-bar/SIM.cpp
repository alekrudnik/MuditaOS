// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SIM.hpp"

namespace gui::status_bar
{
    using namespace Store;

    constexpr auto sim1       = "sim1_status"; // sim 1 indicator
    constexpr auto sim2       = "sim2_status"; // sim 2 indicator
    constexpr auto simunknown = "simunknown";  // sim - unknown sim state indicator (i.e. no initialization was done)
    constexpr auto simfailed  = "simfail";     // sim - notification for sim failure

    SIM::SIM(Item *parent, uint32_t x, uint32_t y) : StatusBarWidgetBase(parent, x, y, 0, 0)
    {
        set(simunknown);
    }

    void SIM::update()
    {
        if (current == Store::GSM::get()->sim) {
            return;
        }
        current = Store::GSM::get()->sim;
        switch (current) {
        case GSM::SIM::SIM1:
            set(sim1);
            break;
        case GSM::SIM::SIM2:
            set(sim2);
            break;
        case GSM::SIM::SIM_FAIL:
            set(simfailed);
            break;
        case GSM::SIM::NONE:
            [[fallthrough]];
        case GSM::SIM::SIM_UNKNOWN:
            set(simunknown);
            break;
        }
    }

    void SIM::show()
    {
        bool isVisible = !(mode == SIMConfiguration::DisplayMode::OnlyInactiveState &&
                           (current == GSM::SIM::SIM1 || current == GSM::SIM::SIM2));
        StatusBarWidgetBase<Image>::setVisible(isVisible);
    }

    void SIM::acceptStatusBarVisitor(StatusBarVisitor &visitor)
    {
        visitor.visit(*this);
    }

    SIMConfiguration::SIMConfiguration(DisplayMode mode) : mode{mode}
    {}

    void SIMConfiguration::visit(gui::status_bar::SIM &widget) const
    {
        widget.mode = getMode();
    }

    SIMConfiguration::DisplayMode SIMConfiguration::getMode() const noexcept
    {
        return mode;
    }

    SIMDevelopersMode::SIMDevelopersMode(Item *parent, uint32_t x, uint32_t y) : SIM(parent, x, y)
    {}

    void SIMDevelopersMode::show()
    {
        StatusBarWidgetBase<Image>::setVisible(true);
    }
}; // namespace gui::status_bar
