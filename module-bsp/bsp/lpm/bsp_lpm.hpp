// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <optional>
#include <memory>
#include <bsp/common.hpp>

namespace bsp {

    class LowPowerMode
    {
      public:
        enum class OscillatorSource
        {
            External,
            Internal
        };

        LowPowerMode()          = default;
        virtual ~LowPowerMode() = default;

        static std::optional<std::unique_ptr<LowPowerMode>> Create();

        virtual int32_t PowerOff() = 0;
        virtual int32_t Reboot() = 0;

        virtual void SetCpuFrequency(CpuFrequencyHz freq) = 0;
        [[nodiscard]] CpuFrequencyHz GetCurrentFrequencyLevel() const noexcept;
        [[nodiscard]] virtual uint32_t GetCpuFrequency() const noexcept = 0;

        virtual void SwitchOscillatorSource(OscillatorSource source) = 0;

    protected:
        CpuFrequencyHz currentFrequency = CpuFrequencyHz::Level_6;
    };
} // namespace bsp

