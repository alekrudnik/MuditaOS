﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#ifndef PUREPHONE_LINUXLPM_H
#define PUREPHONE_LINUXLPM_H

#include "bsp/lpm/bsp_lpm.hpp"

namespace bsp
{

    class LinuxLPM : public LowPowerMode
    {
      public:
        int32_t PowerOff() override final;
        int32_t Reboot() override final;
        void SetCpuFrequency(CpuFrequency freq) final;
        void SwitchOscillatorSource(OscillatorSource source) final;
        void SwitchPll2State(Pll2State state) final;
    };

} // namespace bsp

#endif // PUREPHONE_LINUXLPM_H
