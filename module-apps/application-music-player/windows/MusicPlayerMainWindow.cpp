#include "MusicPlayerMainWindow.hpp"
#include "application-music-player/ApplicationMusicPlayer.hpp"

#include <Style.hpp>
#include <i18/i18.hpp>
#include <log/log.hpp>
#include <service-audio/api/AudioServiceAPI.hpp>

namespace gui
{

    MusicPlayerMainWindow::MusicPlayerMainWindow(app::Application *app)
        : AppWindow(app, gui::name::window::main_window), songsModel{std::make_shared<SongsModel>(this->application)}
    {
        buildInterface();
    }

    void MusicPlayerMainWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    auto MusicPlayerMainWindow::setCurrentVolume(const std::function<void(const audio::Volume &)> &successCallback,
                                                 const std::function<void(const audio::Volume &)> &errCallback) -> bool
    {
        audio::Volume volume;
        const auto ret = application->getCurrentVolume(volume);
        if (ret == audio::RetCode::Success) {
            if (successCallback != nullptr) {
                successCallback(volume);
            }
        }
        else {
            if (errCallback != nullptr) {
                errCallback(volume);
            }
        }
        return ret == audio::RetCode::Success;
    }

    void MusicPlayerMainWindow::buildInterface()
    {
        AppWindow::buildInterface();

        setTitle(utils::localize.get("app_music_player_all_songs"));

        bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("app_music_player_play"));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));

        topBar->setActive(TopBar::Elements::TIME, true);

        songsList = new gui::ListView(this,
                                      musicPlayerStyle::mainWindow::x,
                                      musicPlayerStyle::mainWindow::y,
                                      musicPlayerStyle::mainWindow::w,
                                      musicPlayerStyle::mainWindow::h,
                                      songsModel);

        auto successCallback = [this](const audio::Volume &volume) {
            auto volumeText = audio::GetVolumeText(volume);
            soundLabel      = new gui::Label(this,
                                        musicPlayerStyle::volumeLabel::x,
                                        musicPlayerStyle::volumeLabel::y,
                                        musicPlayerStyle::volumeLabel::w,
                                        musicPlayerStyle::volumeLabel::h,
                                        volumeText);
        };
        auto errCallback = [this](const audio::Volume &volume) {
            soundLabel = new gui::Label(this,
                                        musicPlayerStyle::volumeLabel::x,
                                        musicPlayerStyle::volumeLabel::y,
                                        musicPlayerStyle::volumeLabel::w,
                                        musicPlayerStyle::volumeLabel::h,
                                        musicPlayerStyle::volumeLabel::defaultVolumeLabelText);
        };
        setCurrentVolume(successCallback, errCallback);

        soundLabel->setMargins(gui::Margins(musicPlayerStyle::volumeLabel::leftMargin,
                                            musicPlayerStyle::volumeLabel::topMargin,
                                            musicPlayerStyle::volumeLabel::rightMargin,
                                            musicPlayerStyle::volumeLabel::bottomMargin));
        soundLabel->setFilled(false);
        soundLabel->setPenFocusWidth(style::window::default_border_focus_w);
        soundLabel->setPenWidth(style::window::default_border_no_focus_w);
        soundLabel->setFont(style::window::font::medium);
        soundLabel->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Top));

        setFocusItem(songsList);
    }

    void MusicPlayerMainWindow::destroyInterface()
    {
        erase();
    }

    void MusicPlayerMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        auto app = dynamic_cast<app::ApplicationMusicPlayer *>(application);
        assert(app);

        songsModel->createData(app->getMusicFilesList());
    }

    bool MusicPlayerMainWindow::onDatabaseMessage(sys::Message *msgl)
    {
        return false;
    }

    bool MusicPlayerMainWindow::onInput(const InputEvent &inputEvent)
    {
        auto ret           = AppWindow::onInput(inputEvent);
        const auto keyCode = inputEvent.keyCode;
        if (keyCode == KeyCode::KEY_VOLUP || keyCode == KeyCode::KEY_VOLDN || keyCode == KeyCode::KEY_ENTER) {
            auto successCallback = [this](const audio::Volume &volume) {
                auto volumeText = audio::GetVolumeText(volume);
                soundLabel->setText(volumeText);
            };
            return setCurrentVolume(successCallback, nullptr);
        }
        return ret;
    }

} /* namespace gui */
