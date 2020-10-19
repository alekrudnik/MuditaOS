/*
 *  @file RecorderOperation.cpp
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 23.07.19
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#include "RecorderOperation.hpp"
#include "Audio/encoder/Encoder.hpp"
#include "bsp/audio/bsp_audio.hpp"
#include "Audio/Profiles/Profile.hpp"
#include "Audio/Profiles/ProfileRecordingHeadset.hpp"
#include "Audio/Profiles/ProfileRecordingOnBoardMic.hpp"
#include "Audio/AudioCommon.hpp"

#include "log/log.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace audio
{

#define PERF_STATS_ON 0

    using namespace bsp;

    RecorderOperation::RecorderOperation(
        const char *file, std::function<uint32_t(const std::string &path, const uint32_t &defaultValue)> dbCallback)
    {

        audioCallback = [this](const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer) -> int32_t {

#if PERF_STATS_ON == 1
            auto tstamp = xTaskGetTickCount();
#endif
            auto ret = enc->Encode(framesPerBuffer, reinterpret_cast<int16_t *>(const_cast<void *>(inputBuffer)));
#if PERF_STATS_ON == 1
            LOG_DEBUG("Enc:%dms", xTaskGetTickCount() - tstamp);
            // LOG_DEBUG("Watermark:%lu",uxTaskGetStackHighWaterMark2(NULL));  M.P: left here on purpose, it's handy
            // during sf tests on hardware
#endif
            if (ret == 0) {
                state = State::Idle;
                eventCallback({PlaybackEventType::FileSystemNoSpace, audio::Token::MakeBadToken()});
            }
            return ret;
        };

        constexpr audio::Gain defaultRecordingOnBoardMicGain = 200;
        constexpr audio::Gain defaultRecordingHeadsetGain    = 100;

        const auto dbRecordingOnBoardMicGainPath =
            audio::dbPath(audio::Setting::Gain, audio::PlaybackType::None, audio::Profile::Type::RecordingBuiltInMic);
        const auto recordingOnBoardMicGain = dbCallback(dbRecordingOnBoardMicGainPath, defaultRecordingOnBoardMicGain);

        const auto dbRecordingHeadsetGainPath =
            audio::dbPath(audio::Setting::Gain, audio::PlaybackType::None, audio::Profile::Type::RecordingHeadset);
        const auto recordingHeadsetGain = dbCallback(dbRecordingHeadsetGainPath, defaultRecordingHeadsetGain);

        availableProfiles.push_back(
            Profile::Create(Profile::Type::RecordingBuiltInMic, nullptr, 0, recordingOnBoardMicGain));
        availableProfiles.push_back(Profile::Create(Profile::Type::RecordingHeadset, nullptr, 0, recordingHeadsetGain));

        currentProfile = availableProfiles[0];

        uint32_t channels = 0;
        if ((currentProfile->GetInOutFlags() & static_cast<uint32_t>(AudioDevice::Flags::InputLeft)) ||
            (currentProfile->GetInOutFlags() & static_cast<uint32_t>(AudioDevice::Flags::InputRight))) {
            channels = 1;
        }
        else if (currentProfile->GetInOutFlags() & static_cast<uint32_t>(AudioDevice::Flags::InputStereo)) {
            channels = 2;
        }

        enc = Encoder::Create(file, Encoder::Format{.chanNr = channels, .sampleRate = currentProfile->GetSampleRate()});

        if (enc == nullptr) {
            LOG_ERROR("Error during initializing encoder");
            return;
        }

        audioDevice = AudioDevice::Create(currentProfile->GetAudioDeviceType(), audioCallback).value_or(nullptr);
        if (audioDevice == nullptr) {
            LOG_ERROR("Error creating AudioDevice");
            return;
        }

        isInitialized = true;
    }

    audio::RetCode RecorderOperation::Start(audio::AsyncCallback callback, audio::Token token)
    {

        if (state == State::Paused || state == State::Active) {
            return RetCode::InvokedInIncorrectState;
        }
        operationToken = token;
        eventCallback = callback;
        state         = State::Active;

        if (audioDevice->IsFormatSupported(currentProfile->GetAudioFormat())) {
            auto ret = audioDevice->Start(currentProfile->GetAudioFormat());
            return GetDeviceError(ret);
        }
        else {
            return RetCode::InvalidFormat;
        }
    }

    audio::RetCode RecorderOperation::Stop()
    {

        if (state == State::Paused || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }

        state = State::Idle;
        return GetDeviceError(audioDevice->Stop());
    }

    audio::RetCode RecorderOperation::Pause()
    {

        if (state == State::Paused || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }

        state = State::Paused;
        return GetDeviceError(audioDevice->Stop());
    }

    audio::RetCode RecorderOperation::Resume()
    {

        if (state == State::Active || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }

        state = State::Active;
        auto ret = audioDevice->Start(currentProfile->GetAudioFormat());
        return GetDeviceError(ret);
    }

    audio::RetCode RecorderOperation::SendEvent(std::shared_ptr<Event> evt)
    {
        switch (evt->getType()) {
        case EventType::HeadphonesPlugin:
            SwitchProfile(Profile::Type::RecordingHeadset);
            break;
        case EventType::HeadphonesUnplug:
            // TODO: Switch to recording bt profile if present
            SwitchProfile(Profile::Type::RecordingBuiltInMic);
            break;
        case EventType::BTHeadsetOn:
            SwitchProfile(Profile::Type::RecordingBTHeadset);
            break;
        case EventType::BTHeadsetOff:
            // TODO: Switch to recording headphones profile if present
            SwitchProfile(Profile::Type::RecordingBuiltInMic);
            break;
        default:
            return RetCode::UnsupportedEvent;
        }

        return RetCode::Success;
    }

    audio::RetCode RecorderOperation::SwitchProfile(const Profile::Type type)
    {

        auto ret = GetProfile(type);
        if (ret) {
            currentProfile = ret.value();
        }
        else {
            return RetCode::UnsupportedProfile;
        }

        audioDevice = AudioDevice::Create(currentProfile->GetAudioDeviceType(), audioCallback).value_or(nullptr);

        switch (state) {
        case State::Idle:
        case State::Paused:
            break;

        case State::Active:
            state = State::Idle;
            Start(eventCallback, operationToken);
            break;
        }

        return audio::RetCode::Success;
    }

    audio::RetCode RecorderOperation::SetOutputVolume(float vol)
    {
        currentProfile->SetOutputVolume(vol);
        auto ret = audioDevice->OutputVolumeCtrl(vol);
        return GetDeviceError(ret);
    }

    audio::RetCode RecorderOperation::SetInputGain(float gain)
    {
        currentProfile->SetInputGain(gain);
        auto ret = audioDevice->InputGainCtrl(gain);
        return GetDeviceError(ret);
    }

    Position RecorderOperation::GetPosition()
    {
        return enc->getCurrentPosition();
    }

    std::string RecorderOperation::GetFilename() const noexcept
    {
        return enc->GetFilePath();
    }
} // namespace audio
