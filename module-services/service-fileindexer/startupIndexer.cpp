// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "startupIndexer.hpp"
#include "messages/FileChangeMessage.hpp"
#include <filesystem>
#include <ff_stdio_listdir_recursive.h>
#include <vfs.hpp>
#include <Service/Bus.hpp>
#include "Constants.hpp"

namespace service::detail
{

    namespace fs = std::filesystem;
    namespace
    {
        // File extensions indexing allow list
        const std::vector<std::string_view> allowed_exts{
            ".txt",
            ".wav",
            ".mp3",
            ".flac",
        };
    } // namespace

    auto startupIndexer::fileShouldBeIndexed(std::string_view path) -> bool
    {
        for (const auto &ext : allowed_exts) {
            if (fs::path(path).extension() == ext) {
                return true;
            }
        }
        return false;
    }

    // Collect startup files when service starts
    auto startupIndexer::collectStartupFiles() -> void
    {
        using namespace std::string_literals;
        auto searcher_cb = [](void *ctx, const char *path, bool isDir) {
            auto _this = reinterpret_cast<startupIndexer *>(ctx);
            if (!isDir) {
                for (const auto &ext : allowed_exts) {
                    if (fs::path(path).extension() == ext) {
                        _this->mMsgs.emplace_back(std::make_shared<msg::FileChangeMessage>(
                            path, msg::FileChangeMessage::evt_t::modified, ""s));
                        LOG_DEBUG("Initial indexing file added %s", path);
                    }
                }
            }
        };

        ff_stdio_listdir_recursive(purefs::dir::user_disk.c_str(), searcher_cb, this);
        ff_stdio_listdir_recursive(purefs::dir::os_current.c_str(), searcher_cb, this);
    }
    // Setup timers for notification
    auto startupIndexer::setupTimers(std::shared_ptr<sys::Service> svc, std::string_view svc_name) -> void
    {
        if (!mIdxTimer) {
            mIdxTimer = std::make_unique<sys::Timer>("file_indexing", svc.get(), timer_indexing_time);
            mIdxTimer->connect([this, svc](sys::Timer &) {
                if (!mMsgs.empty()) {
                    sys::Bus::SendUnicast(mMsgs.front(), std::string(service::name::file_indexer), svc.get());
                    mMsgs.pop_front();
                }
                else {
                    mIdxTimer->stop();
                }
            });
            mIdxTimer->start();
        }
    }
} // namespace service::detail