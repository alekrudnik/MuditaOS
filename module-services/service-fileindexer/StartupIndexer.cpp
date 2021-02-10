// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "StartupIndexer.hpp"
#include "messages/FileChangeMessage.hpp"
#include <Timers/TimerFactory.hpp>
#include <filesystem>
//#include <ff_stdio_listdir_recursive.h>
#include <purefs/filesystem_paths.hpp>
#include "Constants.hpp"

namespace service::file_indexer
{

    namespace fs = std::filesystem;
    namespace
    {
        using namespace std::string_literals;
        // File extensions indexing allow list
        const std::vector<std::pair<std::string_view, mimeType>> allowed_exts{
            {".txt", mimeType::text}, {".wav", mimeType::audio}, {".mp3", mimeType::audio}, {".flac", mimeType::audio}};

        // List of initial dirs for scan
        const std::vector<std::string> start_dirs{purefs::dir::getUserDiskPath(), purefs::dir::getCurrentOSPath()};
    } // namespace

    auto StartupIndexer::getFileType(std::string_view path) -> mimeType
    {
        for (const auto &ext : allowed_exts) {
            if (fs::path(path).extension() == ext.first) {
                return ext.second;
            }
        }
        return mimeType::_none_;
    }

    // Collect startup files when service starts
    auto StartupIndexer::collectStartupFiles() -> void
    {
        for (const auto &path : start_dirs)
            for (auto &p : fs::recursive_directory_iterator(path))
                //        using namespace std::string_literals;
                //        auto searcher_cb = [](void *ctx, const char *path, bool isDir) {
                //            auto _this = reinterpret_cast<StartupIndexer *>(ctx);
                if (!fs::is_directory(p)) {
                    for (const auto &ext : allowed_exts) {
                        if (fs::path(p).extension() == ext.first) {
                            mMsgs.emplace_back(std::make_shared<FileChangeMessage>(
                                FileChange::Event::modified, p.path().string(), ""s));
                            LOG_DEBUG("Initial indexing file added %s", p.path().c_str());
                        }
                    }
                }
        //        };
        //        (void)searcher_cb;
        //        for (const auto &path : start_dirs) {
        //            ff_stdio_listdir_recursive(path.c_str(), searcher_cb, this);
        //        }
    }
    // Setup timers for notification
    auto StartupIndexer::setupTimers(std::shared_ptr<sys::Service> svc) -> void
    {
        if (!mIdxTimer.isValid()) {
            mIdxTimer = sys::TimerFactory::createPeriodicTimer(
                svc.get(), "file_indexing", std::chrono::milliseconds{timer_indexing_time}, [this, svc](sys::Timer &) {
                    if (!mMsgs.empty()) {
                        svc->bus.sendUnicast(mMsgs.front(), svc->GetName());
                        mMsgs.pop_front();
                    }
                    else {
                        mIdxTimer.stop();
                    }
                });
            mIdxTimer.start();
        }
    }
} // namespace service::file_indexer
