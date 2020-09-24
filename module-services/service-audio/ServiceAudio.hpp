/*
 *  @file ServiceAudio.hpp
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 29.07.19
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#ifndef PUREPHONE_SERVICEAUDIO_HPP
#define PUREPHONE_SERVICEAUDIO_HPP

#include "Service/Service.hpp"
#include <functional>
#include "Audio/Audio.hpp"
#include "MessageType.hpp"

#include <service-db/api/DBServiceAPI.hpp>
#include <queries/settings/QuerySettingsGet_v2.hpp>
#include <queries/settings/QuerySettingsAddOrIgnore_v2.hpp>
#include <queries/settings/QuerySettingsUpdate_v2.hpp>
#include <service-db/messages/QueryMessage.hpp>
#include <Utils.hpp>

class ServiceAudio : public sys::Service
{

  public:
    ServiceAudio();

    ~ServiceAudio();

    sys::Message_t DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr) override;

    // Invoked when timer ticked
    void TickHandler(uint32_t id) override;

    // Invoked during initialization
    sys::ReturnCodes InitHandler() override;

    sys::ReturnCodes DeinitHandler() override;

    sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final;

    static const char *serviceName;

  private:
    audio::Audio audio;
    std::function<uint32_t(audio::AudioEvents event)> audioCallback = nullptr;

    template <typename T> void addOrIgnoreEntry(const std::string &profilePath, const T &defaultValue)
    {
        auto [code, msg] = DBServiceAPI::GetQueryWithReply(
            this,
            db::Interface::Name::Settings_v2,
            std::make_unique<db::query::settings::AddOrIgnoreQuery>(
                SettingsRecord_v2{SettingsTableRow_v2{{.ID = DB_ID_NONE},
                                                      .path  = profilePath,
                                                      .value = std::to_string(static_cast<uint32_t>(defaultValue))}}),
            audio::audioOperationTimeout);

        if (code == sys::ReturnCodes::Success && msg != nullptr) {
            auto queryResponse = dynamic_cast<db::QueryResponse *>(msg.get());
            assert(queryResponse != nullptr);

            auto settingsResultResponse = queryResponse->getResult();
            assert(dynamic_cast<db::query::settings::AddOrIgnoreResult *>(settingsResultResponse.get()) != nullptr);
        }
    }

    template <typename T>[[nodiscard]] T fetchAudioSettingFromDb(const std::string &profilePath, const T &defaultValue)
    {
        auto [code, msg] =
            DBServiceAPI::GetQueryWithReply(this,
                                            db::Interface::Name::Settings_v2,
                                            std::make_unique<db::query::settings::SettingsQuery>(profilePath),
                                            audio::audioOperationTimeout);

        if (code == sys::ReturnCodes::Success && msg != nullptr) {
            auto queryResponse = dynamic_cast<db::QueryResponse *>(msg.get());
            assert(queryResponse != nullptr);

            auto settingsResultResponse = queryResponse->getResult();
            auto settingsResult = dynamic_cast<db::query::settings::SettingsResult *>(settingsResultResponse.get());
            assert(settingsResult != nullptr);

            return settingsResult->getResult().getValue<T>({});
        }
        return defaultValue;
    }

    void setCurrentSetting(const audio::Setting &setting, const uint32_t &value);
    void setCurrentVolume(const uint32_t &value);
    void setSetting(const audio::Setting &setting,
                    const uint32_t value,
                    const audio::Profile::Type &profileType,
                    const audio::PlaybackType &playbackType);
    [[nodiscard]] uint32_t getSetting(const audio::Setting &setting,
                                      const audio::Profile::Type &profileType,
                                      const audio::PlaybackType &playbackType);
    std::optional<uint32_t> getCurrentSetting(const audio::Setting &setting);
    void updateDbValue(const std::string &path, const audio::Setting &setting, const uint32_t &value);
    void updateDbValue(const audio::Operation *currentOperation, const audio::Setting &setting, const uint32_t &value);
};

#endif // PUREPHONE_SERVICEAUDIO_HPP
