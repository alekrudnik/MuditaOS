// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "SongContext.hpp"
#include <widgets/SongItem.hpp>
#include <InternalModel.hpp>
#include <ListItemProvider.hpp>
#include <apps-common/Application.hpp>

namespace app::music_player
{
    class SongsListItemProvider : public app::InternalModel<gui::SongItem *>, public gui::ListItemProvider
    {
      public:
        virtual ~SongsListItemProvider() noexcept = default;

        virtual void createData(std::function<bool(const std::string &fileName)>) = 0;
        virtual size_t getCurrentIndex() const                                    = 0;
    };

    class SongsModelInterface : public SongsListItemProvider
    {
      public:
        virtual ~SongsModelInterface() noexcept = default;

        virtual bool isSongPlaying() const noexcept                              = 0;
        virtual void setCurrentSongState(SongState songState) noexcept           = 0;
        virtual std::optional<audio::Token> getCurrentFileToken() const noexcept = 0;
        virtual SongContext getCurrentSongContext() const noexcept               = 0;
        virtual void setCurrentSongContext(SongContext context)                  = 0;
        virtual void clearCurrentSongContext()                                   = 0;
    };
} // namespace app::music_player
