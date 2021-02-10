// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <log/log.hpp>
#include "ServiceFileIndexer.hpp"
#include "notesIndexer.hpp"
#include "messages/FileChangeMessage.hpp"
#include "Constants.hpp"
#include <fileref.h>
#include <tag.h>

#include <purefs/vfs_subsystem.hpp>

using namespace service;

ServiceFileIndexer::ServiceFileIndexer(const std::string_view name) : sys::Service(std::string(name))
{
    LOG_DEBUG("[%s] Initializing", std::string(name).c_str());
}

sys::MessagePointer ServiceFileIndexer::DataReceivedHandler(sys::DataMessage *msg, sys::ResponseMessage *resp)
{
    auto fcm = dynamic_cast<file_indexer::FileChangeMessage *>(msg);
    if (fcm) {
        switch (fcm->event()) {
        case file_indexer::FileChange::Event::modified: {
            switch (file_indexer::StartupIndexer::getFileType(fcm->newPath())) {
            case file_indexer::mimeType::audio:
                onAudioContentChanged(fcm->newPath());
                break;
            case file_indexer::mimeType::text:
                onTextContentChanged(fcm->newPath());
                break;
            default:
                LOG_INFO("Skip indexing file %s", fcm->newPath().c_str());
                break;
            }
        } break;
        case file_indexer::FileChange::Event::renamed:
            onRenameFile(fcm->oldPath(), fcm->newPath());
            break;
        case file_indexer::FileChange::Event::deleted:
            onDeleteFile(fcm->newPath());
            break;
        }
        return std::make_shared<sys::ResponseMessage>();
    }
    return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
}

// Initialize data notification handler
sys::ReturnCodes ServiceFileIndexer::InitHandler()
{
    mEventMapper = std::make_unique<file_indexer::EventMapper>(shared_from_this());
    mStartupIndexer.start(shared_from_this());
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceFileIndexer::DeinitHandler()
{
    /*
    vfs.registerNotificationHandler(nullptr);
    */
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceFileIndexer::SwitchPowerModeHandler(const sys::ServicePowerMode mode)
{
    LOG_DEBUG("Switch to power Mode %s", c_str(mode));
    return sys::ReturnCodes::Success;
}

// When file is changed update db only
auto ServiceFileIndexer::onDeleteFile(std::string_view path) -> void
{
    LOG_DEBUG("File deleted %s", std::string(path).c_str());
}
// When file is renamed
auto ServiceFileIndexer::onRenameFile(std::string_view oldPath, std::string_view newPath) -> void
{
    LOG_DEBUG("File renamed old: %s, new: %s", std::string(oldPath).c_str(), std::string(newPath).c_str());
}
// On audio file content change
auto ServiceFileIndexer::onAudioContentChanged(std::string_view path) -> void
{
    LOG_DEBUG("Audio content index %s", std::string(path).c_str());
    TagLib::FileRef fref(std::string(path).c_str());
    if (!fref.isNull() && fref.tag()) {
        const auto tag = fref.tag();
        LOG_DEBUG(">>>>> title %s", tag->title().toCString());
        LOG_DEBUG(">>>>> artist %s", tag->artist().toCString());
        LOG_DEBUG(">>>> album %s", tag->album().toCString());
        LOG_DEBUG(">>>>> year %i", tag->year());
        LOG_DEBUG(">>>>> comment %s", tag->comment().toCString());
        LOG_DEBUG(">>>> track %u", tag->track());
        LOG_DEBUG(">>>> genre %s", tag->genre().toCString());
    }
    if (!fref.isNull() && fref.audioProperties()) {
        const auto prop = fref.audioProperties();
        int seconds     = prop->length() % 60;
        int minutes     = (prop->length() - seconds) / 60;
        LOG_DEBUG(">>>>> bitrate %i", prop->bitrate());
        LOG_DEBUG(">>>>> samplerate %i", prop->sampleRate());
        LOG_DEBUG(">>>>> channels %i", prop->channels());
        LOG_DEBUG(">>>>> length %02i:%02i", minutes, seconds);
    }
}
// On text file content change
auto ServiceFileIndexer::onTextContentChanged(std::string_view path) -> void
{
    LOG_DEBUG("Text content index %s", std::string(path).c_str());
    detail::notesIndexer noteInfo(path);
    LOG_DEBUG("Words %zu Lines %zu Chars %zu Size %zu",
              noteInfo.getWords(),
              noteInfo.getLines(),
              noteInfo.getChars(),
              noteInfo.getFileSize());
}
