// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "BellPxAudioCodec.hpp"
#include "board.h"
#include <log/log.hpp>
#include <cmath>

#include "board/BoardDefinitions.hpp"
#include "board/rt1051/common/audio.hpp"

using audio::codec::Configuration;

namespace
{
    constexpr auto maxBellVolume = 15.0f;
    constexpr auto minBellVolume = 0.0f;
} // namespace

namespace audio
{
    sai_edma_handle_t BellPxAudioCodec::txHandle = {};
    sai_edma_handle_t BellPxAudioCodec::rxHandle = {};

    BellPxAudioCodec::BellPxAudioCodec(const Configuration &format)
        : SAIAudioDevice(BELL_AUDIOCODEC_SAIx, &rxHandle, &txHandle), saiInFormat{}, saiOutFormat{},
          codecParams{}, codec{},
          formats(audio::AudioFormat::makeMatrix(supportedSampleRates, supportedBitWidths, supportedChannelModes)),
          currentFormat(format)
    {}

    BellPxAudioCodec::~BellPxAudioCodec()
    {
        Stop();
        bsp::audio::deinit();
    }

    AudioDevice::RetCode BellPxAudioCodec::Start()
    {
        if (state == State::Running) {
            return AudioDevice::RetCode::Failure;
        }

        auto sampleRate = CodecParams::ValToSampleRate(currentFormat.sampleRate_Hz);
        if (sampleRate == CodecParams::SampleRate::Invalid) {
            LOG_ERROR("Unsupported sample rate");
            return AudioDevice::RetCode::Failure;
        };

        bsp::audio::init(currentFormat.sampleRate_Hz);

        saiInFormat.bitWidth      = currentFormat.bitWidth;
        saiInFormat.sampleRate_Hz = currentFormat.sampleRate_Hz;

        saiOutFormat.bitWidth      = currentFormat.bitWidth;
        saiOutFormat.sampleRate_Hz = currentFormat.sampleRate_Hz;

        if (currentFormat.flags & static_cast<std::uint32_t>(audio::codec::Flags::InputLeft)) {
            saiInFormat.stereo = kSAI_MonoLeft;
            InStart();
        }
        else if (currentFormat.flags & static_cast<std::uint32_t>(audio::codec::Flags::InputRight)) {
            saiInFormat.stereo = kSAI_MonoRight;
            InStart();
        }
        else if (currentFormat.flags & static_cast<std::uint32_t>(audio::codec::Flags::InputStereo)) {
            saiInFormat.stereo = kSAI_Stereo;
            InStart();
        }

        if (currentFormat.flags & static_cast<std::uint32_t>(audio::codec::Flags::OutputMono)) {
            saiOutFormat.stereo = kSAI_MonoLeft;
            OutStart();
        }
        else if (currentFormat.flags & static_cast<std::uint32_t>(audio::codec::Flags::OutputStereo)) {
            saiOutFormat.stereo = kSAI_Stereo;
            OutStart();
        }

        codecParams.sampleRate = sampleRate;
        /// Set the codec output volume to max possible value. Volume control is implemented
        /// using software scaling instead of hardware gain control.
        codecParams.outVolume = maxBellVolume;
        codecParams.inGain    = currentFormat.inputGain;

        /// Set the initial volume used by the software volume control
        setOutputVolume(currentFormat.outputVolume);

        txEnabled = true;
        initiateTxTransfer();
        codec.Start(codecParams);

        state = State::Running;

        return AudioDevice::RetCode::Success;
    }

    AudioDevice::RetCode BellPxAudioCodec::Stop()
    {
        if (state == State::Stopped) {
            return AudioDevice::RetCode::Failure;
        }

        InStop();
        OutStop();

        codec.Stop();

        state = State::Stopped;
        vTaskDelay(codecSettleTime);

        return AudioDevice::RetCode::Success;
    }

    AudioDevice::RetCode BellPxAudioCodec::setInputGain(float gain)
    {
        currentFormat.inputGain = gain;
        CodecParamsAW8898 params;
        params.inGain = gain;
        params.opCmd  = CodecParams::Cmd::SetInGain;
        codec.Ioctrl(params);
        return AudioDevice::RetCode::Success;
    }

    void BellPxAudioCodec::InStart()
    {
        sai_transceiver_t saiConfig;
        auto audioCfg = bsp::audio::AudioConfig::get();

        SAI_TransferRxCreateHandleEDMA(BELL_AUDIOCODEC_SAIx,
                                       &rxHandle,
                                       rxAudioCodecCallback,
                                       this,
                                       reinterpret_cast<edma_handle_t *>(audioCfg->rxDMAHandle->GetHandle()));
        /* I2S mode configurations */
        SAI_GetClassicI2SConfig(
            &saiConfig, static_cast<sai_word_width_t>(saiInFormat.bitWidth), saiInFormat.stereo, 0U);
        saiConfig.syncMode    = kSAI_ModeSync;
        saiConfig.masterSlave = kSAI_Slave;
        SAI_TransferRxSetConfigEDMA(BELL_AUDIOCODEC_SAIx, &txHandle, &saiConfig);
        const auto channelNumbers = saiInFormat.stereo == kSAI_Stereo ? 2U : 1U;
        /* set bit clock divider */
        SAI_RxSetBitClockRate(BELL_AUDIOCODEC_SAIx,
                              audioCfg->mclkSourceClockHz,
                              saiInFormat.sampleRate_Hz,
                              saiInFormat.bitWidth,
                              channelNumbers);
    }

    void BellPxAudioCodec::OutStart()
    {
        sai_transceiver_t saiConfig;
        auto audioCfg = bsp::audio::AudioConfig::get();

        SAI_TransferTxCreateHandleEDMA(BELL_AUDIOCODEC_SAIx,
                                       &txHandle,
                                       txAudioCodecCallback,
                                       this,
                                       reinterpret_cast<edma_handle_t *>(audioCfg->txDMAHandle->GetHandle()));
        /* I2S mode configurations */
        SAI_GetClassicI2SConfig(
            &saiConfig, static_cast<sai_word_width_t>(saiOutFormat.bitWidth), saiOutFormat.stereo, 0U);
        SAI_TransferTxSetConfigEDMA(BELL_AUDIOCODEC_SAIx, &txHandle, &saiConfig);
        const auto channelNumbers = saiOutFormat.stereo == kSAI_Stereo ? 2U : 1U;
        /* set bit clock divider */
        SAI_TxSetBitClockRate(BELL_AUDIOCODEC_SAIx,
                              audioCfg->mclkSourceClockHz,
                              saiOutFormat.sampleRate_Hz,
                              saiOutFormat.bitWidth,
                              channelNumbers);
    }

    void BellPxAudioCodec::OutStop()
    {
        SAI_TxDisableInterrupts(BELL_AUDIOCODEC_SAIx, kSAI_FIFOErrorInterruptEnable);
        if (txHandle.dmaHandle) {
            SAI_TransferTerminateSendEDMA(BELL_AUDIOCODEC_SAIx, &txHandle);
        }
        memset(&txHandle, 0, sizeof(txHandle));
    }

    void BellPxAudioCodec::InStop()
    {
        SAI_RxDisableInterrupts(BELL_AUDIOCODEC_SAIx, kSAI_FIFOErrorInterruptEnable);
        if (rxHandle.dmaHandle) {
            SAI_TransferAbortReceiveEDMA(BELL_AUDIOCODEC_SAIx, &rxHandle);
        }
        memset(&rxHandle, 0, sizeof(rxHandle));
    }

    auto BellPxAudioCodec::getSupportedFormats() -> std::vector<AudioFormat>
    {
        return formats;
    }

    auto BellPxAudioCodec::getTraits() const -> Traits
    {
        return Traits{.usesDMA = true};
    }

    auto BellPxAudioCodec::getSourceFormat() -> audio::AudioFormat
    {
        if (currentFormat.flags == 0) {
            return audio::nullFormat;
        }

        auto isMono = (currentFormat.flags & static_cast<unsigned int>(audio::codec::Flags::InputStereo)) == 0;
        return audio::AudioFormat{currentFormat.sampleRate_Hz, currentFormat.bitWidth, isMono ? 1U : 2U};
    }

    AudioDevice::RetCode BellPxAudioCodec::setOutputVolume(float vol)
    {
        /// Calculated for a curve with a dynamic range of 52dB
        /// For more info check: https://www.dr-lex.be/info-stuff/volumecontrols.html
        constexpr auto a = 2.512e-3f;
        constexpr auto b = 5.986721f;

        vol          = std::clamp(vol, minBellVolume, maxBellVolume);
        volumeFactor = std::clamp(a * std::exp(b * (vol / maxBellVolume)), 0.f, 1.f);
        return AudioDevice::RetCode::Success;
    }

    void rxAudioCodecCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
    {
        auto self = static_cast<BellPxAudioCodec *>(userData);
        self->onDataReceive();
    }

    void txAudioCodecCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
    {
        auto self = static_cast<BellPxAudioCodec *>(userData);
        self->onDataSend();
    }

} // namespace audio
