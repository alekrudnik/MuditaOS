// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <map>
#include <bitset>
#include <bsp/audio/bsp_audio.hpp>
#include <Utils.hpp>

#include "Profiles/Profile.hpp"

namespace audio
{
    class AudioMux;
}

namespace audio
{
    inline constexpr Volume defaultVolumeStep = 1;
    inline constexpr Gain defaultGainStep     = 10;
    inline constexpr Volume defaultVolume     = 5;
    inline constexpr Gain defaultGain         = 5;

    inline constexpr Volume maxVolume = 10;
    inline constexpr Volume minVolume = 0;

    inline constexpr Gain maxGain = 100;
    inline constexpr Gain minGain = 0;

    inline constexpr auto audioOperationTimeout = 1000U;

    inline constexpr auto audioDbPrefix = "audio/";

    enum class Setting
    {
        Volume,
        Gain,
        EnableVibration,
        EnableSound
    };

    enum class PlaybackType
    {
        None,
        Multimedia,
        Notifications,
        KeypadSound,
        CallRingtone,
        TextMessageRingtone,
        Last = TextMessageRingtone,
    };

    const static std::map<PlaybackType, uint8_t> PlaybackTypePriority = {
        {PlaybackType::CallRingtone, 2},
        {PlaybackType::TextMessageRingtone, 3},
        {PlaybackType::Notifications, 3},
        {PlaybackType::Multimedia, 4},
        {PlaybackType::KeypadSound, 5},
        {PlaybackType::None, static_cast<uint8_t>(PlaybackType::Last)},
    };

    [[nodiscard]] const std::string str(const PlaybackType &playbackType) noexcept;

    [[nodiscard]] const std::string str(const Setting &setting) noexcept;

    [[nodiscard]] const std::string dbPath(const Setting &setting,
                                           const PlaybackType &playbackType,
                                           const Profile::Type &profileType);

    enum class EventType
    {
        // HW state change notifications
        JackState,               //!< jack input plugged / unplugged event
        BlutoothHSPDeviceState,  //!< BT device connected / disconnected event (Headset Profile)
        BlutoothA2DPDeviceState, //!< BT device connected / disconnected event (Advanced Audio Distribution Profile)

        // call control
        CallMute,
        CallUnmute,
        CallLoudspeakerOn,
        CallLoudspeakerOff,
    };

    constexpr auto hwStateUpdateMaxEvent = magic_enum::enum_index(EventType::BlutoothA2DPDeviceState);

    class Event
    {
      public:
        enum class DeviceState
        {
            Connected,
            Disconnected
        };

        explicit Event(EventType eType, DeviceState deviceState = DeviceState::Connected)
            : eventType(eType), deviceState(deviceState)
        {}

        virtual ~Event() = default;

        EventType getType() const noexcept
        {
            return eventType;
        }

        DeviceState getDeviceState() const noexcept
        {
            return deviceState;
        }

      private:
        const EventType eventType;
        const DeviceState deviceState;
    };

    class AudioSinkState
    {
      public:
        void UpdateState(std::shared_ptr<Event> stateChangeEvent)
        {
            auto hwUpdateEventIdx = magic_enum::enum_integer(stateChangeEvent->getType());
            if (hwUpdateEventIdx <= hwStateUpdateMaxEvent) {
                audioSinkState.set(hwUpdateEventIdx,
                                   stateChangeEvent->getDeviceState() == Event::DeviceState::Connected ? true : false);
            }
        }

        std::vector<std::shared_ptr<Event>> getUpdateEvents() const
        {
            std::vector<std::shared_ptr<Event>> updateEvents;
            for (size_t i = 0; i < hwStateUpdateMaxEvent; i++) {
                auto isConnected =
                    audioSinkState.test(i) ? Event::DeviceState::Connected : Event::DeviceState::Disconnected;
                auto updateEvt = magic_enum::enum_cast<EventType>(i);
                updateEvents.emplace_back(std::make_unique<Event>(updateEvt.value(), isConnected));
            }
            return updateEvents;
        }

        bool isConnected(EventType deviceUpdateEvent) const
        {
            return audioSinkState.test(magic_enum::enum_integer(deviceUpdateEvent));
        }

        void setConnected(EventType deviceUpdateEvent, bool isConnected)
        {
            audioSinkState.set(magic_enum::enum_integer(deviceUpdateEvent), isConnected);
        }

      private:
        std::bitset<magic_enum::enum_count<EventType>()> audioSinkState;
    };

    enum class RetCode
    {
        Success = 0,
        InvokedInIncorrectState,
        UnsupportedProfile,
        UnsupportedEvent,
        InvalidFormat,
        OperationCreateFailed,
        FileDoesntExist,
        FailedToAllocateMemory,
        OperationNotSet,
        ProfileNotSet,
        DeviceFailure,
        TokenNotFound,
        Failed
    };

    class Token
    {
        using TokenType = int16_t;

      public:
        explicit Token(TokenType initValue = tokenUninitialized) : t(initValue)
        {}

        bool operator==(const Token &other) const noexcept
        {
            return other.t == t;
        }

        bool operator!=(const Token &other) const noexcept
        {
            return !(other.t == t);
        }

        /**
         * Valid token is one connected with existing sequence of operations
         * @return True if valid, false otherwise
         */
        bool IsValid() const
        {
            return t > tokenUninitialized;
        }
        /**
         * Bad token cannot be used anymore
         * @return True if token is flagged bad
         */
        bool IsBad() const
        {
            return t == tokenBad;
        }
        /**
         * Uninitialized token can be used but it is not connected to any sequence of operations
         * @return True if token is flagged uninitialized
         */
        bool IsUninitialized() const
        {
            return t == tokenUninitialized;
        }
        /**
         * Helper - returns bad Token
         * @return Unusable bad Token
         */
        static inline Token MakeBadToken()
        {
            return Token(tokenBad);
        }

      private:
        static constexpr auto maxToken = std::numeric_limits<TokenType>::max();
        Token IncrementToken()
        {
            t = (t == maxToken) ? 0 : t + 1;
            return *this;
        }

        constexpr static TokenType tokenUninitialized = -1;
        constexpr static TokenType tokenBad           = -2;

        TokenType t;
        friend class ::audio::AudioMux;
    };

    class Handle
    {
      public:
        Handle(const RetCode &retCode = RetCode::Failed, const Token &token = Token())
            : lastRetCode(retCode), token(token)
        {}
        auto GetLastRetCode() -> RetCode
        {
            return lastRetCode;
        }
        auto GetToken() const -> const Token &
        {
            return token;
        }

      private:
        RetCode lastRetCode;
        Token token;
    };

    enum class PlaybackEventType
    {
        Empty,
        EndOfFile,
        FileSystemNoSpace
    };

    struct PlaybackEvent
    {
        PlaybackEventType event = PlaybackEventType::Empty;
        audio::Token token      = audio::Token::MakeBadToken();
    };

    typedef std::function<int32_t(PlaybackEvent e)> AsyncCallback;
    typedef std::function<uint32_t(const std::string &path, const uint32_t &defaultValue)> DbCallback;

    RetCode GetDeviceError(bsp::AudioDevice::RetCode retCode);
    const std::string str(RetCode retcode);
    [[nodiscard]] auto GetVolumeText(const audio::Volume &volume) -> const std::string;
} // namespace audio

namespace audio::notifications
{
    const std::vector<audio::PlaybackType> typesToMute = {audio::PlaybackType::Notifications,
                                                          audio::PlaybackType::CallRingtone,
                                                          audio::PlaybackType::TextMessageRingtone};
} // namespace audio::notifications
