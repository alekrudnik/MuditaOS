// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "NewEditAlarmModel.hpp"
#include "application-alarm-clock/widgets/AlarmTimeItem.hpp"
#include "application-alarm-clock/widgets/AlarmOptionsItem.hpp"
#include "application-alarm-clock/widgets/AlarmClockStyle.hpp"
#include <ListView.hpp>

namespace app::alarmClock
{
    NewEditAlarmModel::NewEditAlarmModel(app::Application *app,
                                         std::shared_ptr<AbstractAlarmsRepository> alarmsRepository,
                                         bool mode24H)
        : application(app), alarmsRepository{std::move(alarmsRepository)}, mode24H(mode24H)
    {}

    unsigned int NewEditAlarmModel::requestRecordsCount()
    {
        return internalData.size();
    }

    unsigned int NewEditAlarmModel::getMinimalItemHeight() const
    {
        return style::alarmClock::window::item::options::height;
    }

    void NewEditAlarmModel::requestRecords(uint32_t offset, uint32_t limit)
    {
        setupModel(offset, limit);
        list->onProviderDataUpdate();
    }

    gui::ListItem *NewEditAlarmModel::getItem(gui::Order order)
    {
        return getRecord(order);
    }

    void NewEditAlarmModel::createData()
    {
        auto app = application;
        assert(app != nullptr);

        internalData.push_back(new gui::AlarmTimeItem(
            mode24H,
            [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));
        soundItem = new gui::AlarmOptionsItem(
            application,
            AlarmOptionItemName::Sound,
            [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); });
        internalData.push_back(soundItem);

        internalData.push_back(new gui::AlarmOptionsItem(
            application,
            AlarmOptionItemName::Snooze,
            [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

        repeatOption = new gui::AlarmOptionsItem(
            application,
            AlarmOptionItemName::Repeat,
            [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); });
        internalData.push_back(repeatOption);

        for (auto &item : internalData) {
            item->deleteByList = false;
        }
    }

    void NewEditAlarmModel::loadData(std::shared_ptr<AlarmsRecord> record)
    {
        list->clear();
        eraseInternalData();

        createData();

        for (auto &item : internalData) {
            if (item->onLoadCallback) {
                item->onLoadCallback(record);
            }
        }

        list->rebuildList();
    }

    void NewEditAlarmModel::loadRepeat(std::shared_ptr<AlarmsRecord> record)
    {
        if (repeatOption->onLoadCallback) {
            repeatOption->onLoadCallback(std::move(record));
        }
    }

    void NewEditAlarmModel::saveData(std::shared_ptr<AlarmsRecord> alarm, AlarmAction action)
    {
        for (auto &item : internalData) {
            if (item->onSaveCallback) {
                item->onSaveCallback(alarm);
            }
        }

        if (action == AlarmAction::Edit) {
            alarmsRepository->update(*alarm,
                                     [this](bool) { application->switchWindow(gui::name::window::main_window); });
        }
        else {
            alarmsRepository->add(*alarm, [this](bool) { application->returnToPreviousWindow(); });
        }
        list->clear();
        eraseInternalData();
    }

    void NewEditAlarmModel::updateAudioToken(audio::Token audioToken)
    {
        soundItem->updateAudioToken(audioToken);
    }
} // namespace app::alarmClock
