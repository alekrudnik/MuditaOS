// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/widgets/BellSideListItem.hpp>
#include <apps-common/widgets/spinners/Spinners.hpp>

namespace gui
{
    class Spinner;
    class Label;

    class MeditationTimerListItem : public BellSideListItem
    {
        UIntegerSpinner *spinner{};
        Label *bottomDescription{};

        void createSpinner();
        void createBottomDescription();
        void registerCallbacks();

        void onValueChanged(std::uint32_t currentValue);

      public:
        MeditationTimerListItem();

        [[nodiscard]] int getSpinnerValue() const noexcept;
        void setSpinnerValue(int value);
    };
} // namespace gui
