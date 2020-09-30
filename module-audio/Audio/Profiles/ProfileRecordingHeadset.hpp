/*
 *  @file ProfileRecordingHeadset.hpp
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 24.07.19
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#pragma once

#include "Profile.hpp"

namespace audio
{

    class ProfileRecordingHeadset : public Profile
    {
      public:
        ProfileRecordingHeadset(std::function<int32_t()> callback, Gain gain)
            : Profile(
                  "Recording Headset",
                  Type::RecordingHeadset,
                  bsp::AudioDevice::Format{.sampleRate_Hz = 44100,
                                           .bitWidth      = 16,
                                           .flags         = static_cast<uint32_t>(
                                               bsp::AudioDevice::Flags::InputLeft), // microphone use left audio channel
                                           .outputVolume = 0,
                                           .inputGain    = static_cast<float>(gain),
                                           .inputPath    = bsp::AudioDevice::InputPath::Headphones,
                                           .outputPath   = bsp::AudioDevice::OutputPath::None},
                  bsp::AudioDevice::Type::Audiocodec,
                  callback)
        {}
    };

} // namespace audio

