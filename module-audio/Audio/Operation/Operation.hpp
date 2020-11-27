// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <memory>
#include <optional>
#include <functional>

#include <Audio/AudioCommon.hpp>
#include <Audio/Stream.hpp>
#include <Audio/encoder/Encoder.hpp>
#include <Audio/Profiles/Profile.hpp>

#include <service-bluetooth/ServiceBluetoothCommon.hpp>

namespace audio
{
    class Operation
    {
      public:
        Operation(const audio::PlaybackType &playbackType = audio::PlaybackType::None) : playbackType{playbackType}
        {}

        enum class State
        {
            Idle,
            Active,
            Paused
        };

        enum class Type
        {
            Idle,
            Playback,
            Recorder,
            Router
        };

        [[nodiscard]] static inline auto c_str(Type type) -> const char *
        {
            switch (type) {
            case Type::Idle:
                return "Idle";
            case Type::Playback:
                return "Playback";
            case Type::Recorder:
                return "Recorder";
            case Type::Router:
                return "Router ";
            }
            return "";
        }

        virtual ~Operation() = default;

        static std::unique_ptr<Operation> Create(
            Type t,
            const char *fileName                  = "",
            const audio::PlaybackType &operations = audio::PlaybackType::None,
            std::function<uint32_t(const std::string &path, const uint32_t &defaultValue)> dbCallback = nullptr);

        virtual audio::RetCode Start(audio::AsyncCallback callback, audio::Token token) = 0;
        virtual audio::RetCode Stop()                                                   = 0;
        virtual audio::RetCode Pause()                                                  = 0;
        virtual audio::RetCode Resume()                                                 = 0;
        virtual audio::RetCode SendEvent(std::shared_ptr<Event> evt)                    = 0;
        virtual audio::RetCode SetOutputVolume(float vol)                               = 0;
        virtual audio::RetCode SetInputGain(float gain)                                 = 0;

        virtual Position GetPosition() = 0;
        virtual void SetBluetoothStreamData(std::shared_ptr<BluetoothStreamData> data);

        Volume GetOutputVolume() const
        {
            return (currentProfile != nullptr) ? currentProfile->GetOutputVolume() : Volume{};
        }

        Gain GetInputGain() const
        {
            return currentProfile->GetInputGain();
        }

        State GetState() const
        {
            return state;
        }

        const std::shared_ptr<Profile> GetProfile() const
        {
            return currentProfile;
        }

        audio::PlaybackType GetPlaybackType() const noexcept
        {
            return playbackType;
        }

        const audio::Token &GetToken() const noexcept
        {
            return operationToken;
        }

        Type GetOperationType() const noexcept
        {
            return opType;
        }

        std::string GetFilePath() const noexcept
        {
            return filePath;
        }

        audio::RetCode SwitchToPriorityProfile();

        void SetDataStreams(Stream *dStreamOut, Stream *dStreamIn)
        {
            dataStreamOut = dStreamOut;
            dataStreamIn  = dStreamIn;
        }

      protected:
        struct SupportedProfile
        {
            SupportedProfile(std::shared_ptr<Profile> profile, bool isAvailable)
                : profile(std::move(profile)), isAvailable(isAvailable)
            {}

            std::shared_ptr<Profile> profile;
            bool isAvailable;
        };

        Stream *dataStreamOut = nullptr;
        Stream *dataStreamIn  = nullptr;

        std::shared_ptr<Profile> currentProfile;
        std::unique_ptr<bsp::AudioDevice> audioDevice;

        std::vector<SupportedProfile> supportedProfiles;
        void SetProfileAvailability(std::vector<Profile::Type> profiles, bool available);

        State state = State::Idle;
        audio::AsyncCallback eventCallback;

        audio::Token operationToken;
        Type opType = Type::Idle;
        std::string filePath;

        audio::PlaybackType playbackType = audio::PlaybackType::None;

        virtual audio::RetCode SwitchProfile(const Profile::Type type) = 0;

        std::shared_ptr<Profile> GetProfile(const Profile::Type type);

        std::function<int32_t(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer)>
            audioCallback = nullptr;
    };

} // namespace audio
