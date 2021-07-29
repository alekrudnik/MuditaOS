// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <models/SongsRepository.hpp>

#include <optional>

namespace testing::app::music_player
{
    class MockTagsFetcher : public ::app::music_player::AbstractTagsFetcher
    {
      public:
        MOCK_METHOD(std::optional<audio::Tags>, getFileTags, (const std::string &filePath), (override));
    };
}; // namespace testing::app::music_player
