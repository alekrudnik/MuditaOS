// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "WindowsPopupQueue.hpp"
#include "WindowsPopupFilter.hpp"

namespace app
{
    std::optional<gui::popup::Request> WindowsPopupQueue::popRequest(const gui::popup::Filter &filter)
    {
        for (const auto &val : requests) {
            if (filter.is_ok(val.getPopupParams())) {
                printf("pop depth %d\n", requests.size() - 1);
                return {std::move(requests.extract(val).value())};
            }
        }
        return {std::nullopt};
    }

    bool WindowsPopupQueue::pushRequest(gui::popup::Request &&r)
    {
        const auto &[_, success] = requests.emplace(std::move(r));
        if (success) {
            printf("push depth %d\n", requests.size());
        }
        return success;
    }
} // namespace app
