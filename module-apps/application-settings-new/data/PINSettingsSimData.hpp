// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <string>
#include <SwitchData.hpp>

namespace gui
{
    class PINSettingsSimData : public SwitchData
    {
      public:
        explicit PINSettingsSimData(std::string sim) : sim(std::move(sim))
        {}
        [[nodiscard]] auto getSim() const -> const std::string &
        {
            return sim;
        }

      private:
        const std::string sim;
    };
} // namespace gui
