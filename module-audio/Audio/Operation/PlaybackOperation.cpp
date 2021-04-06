// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PlaybackOperation.hpp"

#include "Audio/decoder/Decoder.hpp"
#include "Audio/Profiles/Profile.hpp"
#include "Audio/StreamFactory.hpp"

#include "Audio/AudioCommon.hpp"

#include <log/log.hpp>

namespace audio
{

    using namespace AudioServiceMessage;
    using namespace utils;

    PlaybackOperation::PlaybackOperation(const char *file, const audio::PlaybackType &playbackType, Callback callback)
        : Operation(callback, playbackType), dec(nullptr)
    {
        // order defines priority
        AddProfile(Profile::Type::PlaybackBluetoothA2DP, playbackType, false);
        AddProfile(Profile::Type::PlaybackHeadphones, playbackType, false);
        AddProfile(Profile::Type::PlaybackLoudspeaker, playbackType, true);

        endOfFileCallback = [this]() {
            state          = State::Idle;
            const auto req = AudioServiceMessage::EndOfFile(operationToken);
            serviceCallback(&req);
            return std::string();
        };

        dec = Decoder::Create(file);
        if (dec == nullptr) {
            throw AudioInitException("Error during initializing decoder", RetCode::FileDoesntExist);
        }
        tags        = dec->fetchTags();
        auto format = dec->getSourceFormat();
        LOG_DEBUG("Source format: %s", format.toString().c_str());

        auto retCode = SwitchToPriorityProfile();
        if (retCode != RetCode::Success) {
            throw AudioInitException("Failed to switch audio profile", retCode);
        }
    }

    audio::RetCode PlaybackOperation::Start(audio::Token token)
    {
        if (state == State::Active || state == State::Paused) {
            return RetCode::InvokedInIncorrectState;
        }

        // create stream
        StreamFactory streamFactory(playbackCapabilities, playbackBufferingSize);
        dataStreamOut = streamFactory.makeStream(*dec.get(), *audioDevice.get());

        // create audio connection
        outputConnection = std::make_unique<StreamConnection>(dec.get(), audioDevice.get(), dataStreamOut.get());

        // decoder worker soft start - must be called after connection setup
        dec->startDecodingWorker(endOfFileCallback);

        // start output device and enable audio connection
        auto ret = audioDevice->Start(currentProfile->GetAudioFormat());
        outputConnection->enable();

        // update state and token
        state          = State::Active;
        operationToken = token;

        return GetDeviceError(ret);
    }

    audio::RetCode PlaybackOperation::Stop()
    {
        state = State::Idle;
        if (!audioDevice) {
            return audio::RetCode::DeviceFailure;
        }

        // stop playback by destroying audio connection
        outputConnection.reset();
        dec->stopDecodingWorker();
        dataStreamOut.reset();

        return GetDeviceError(audioDevice->Stop());
    }

    audio::RetCode PlaybackOperation::Pause()
    {
        if (state == State::Paused || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }
        state = State::Paused;
        outputConnection->disable();
        return audio::RetCode::Success;
    }

    audio::RetCode PlaybackOperation::Resume()
    {
        if (state == State::Active || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }
        state = State::Active;
        outputConnection->enable();
        return audio::RetCode::Success;
    }

    audio::RetCode PlaybackOperation::SetOutputVolume(float vol)
    {
        currentProfile->SetOutputVolume(vol);
        auto ret = audioDevice->OutputVolumeCtrl(vol);
        return GetDeviceError(ret);
    }

    audio::RetCode PlaybackOperation::SetInputGain(float gain)
    {
        currentProfile->SetInputGain(gain);
        auto ret = audioDevice->InputGainCtrl(gain);
        return GetDeviceError(ret);
    }

    Position PlaybackOperation::GetPosition()
    {
        return dec->getCurrentPosition();
    }

    audio::RetCode PlaybackOperation::SendEvent(std::shared_ptr<Event> evt)
    {
        auto isAvailable = evt->getDeviceState() == Event::DeviceState::Connected ? true : false;
        switch (evt->getType()) {
        case EventType::JackState:
            SetProfileAvailability({Profile::Type::PlaybackHeadphones}, isAvailable);
            SwitchToPriorityProfile();
            break;
        case EventType::BlutoothA2DPDeviceState:
            SetProfileAvailability({Profile::Type::PlaybackBluetoothA2DP}, isAvailable);
            SwitchToPriorityProfile();
            break;
        default:
            return RetCode::UnsupportedEvent;
        }

        return RetCode::Success;
    }

    audio::RetCode PlaybackOperation::SwitchProfile(const Profile::Type type)
    {
        auto newProfile = GetProfile(type);
        if (newProfile == nullptr) {
            return RetCode::UnsupportedProfile;
        }

        if (currentProfile && currentProfile->GetType() == newProfile->GetType()) {
            return RetCode::Success;
        }

        /// profile change - (re)create output device; stop audio first by
        /// killing audio connection
        outputConnection.reset();
        dec->stopDecodingWorker();
        audioDevice.reset();
        dataStreamOut.reset();
        audioDevice = CreateDevice(newProfile->GetAudioDeviceType());
        if (audioDevice == nullptr) {
            LOG_ERROR("Error creating AudioDevice");
            return RetCode::Failed;
        }

        // check if audio device supports Decoder's profile
        if (auto format = dec->getSourceFormat(); !audioDevice->isFormatSupportedBySink(format)) {
            LOG_ERROR("Format unsupported by the audio device: %s", format.toString().c_str());
            return RetCode::Failed;
        }

        // adjust new profile with information from file's tags
        newProfile->SetSampleRate(tags->sample_rate);
        newProfile->SetInOutFlags(static_cast<uint32_t>(AudioDevice::Flags::OutputStereo));

        // store profile
        currentProfile = newProfile;

        if (state == State::Active) {
            // playback in progress, restart
            state = State::Idle;
            Start(operationToken);
        }

        return audio::RetCode::Success;
    }

    PlaybackOperation::~PlaybackOperation()
    {
        Stop();
    }

} // namespace audio
