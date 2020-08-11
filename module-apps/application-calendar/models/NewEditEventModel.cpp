#include "NewEditEventModel.hpp"
#include "application-calendar/widgets/TextWithLabelItem.hpp"
#include "application-calendar/widgets/EventTimeItem.hpp"
#include "application-calendar/widgets/SeveralOptionsItem.hpp"
#include "application-calendar/widgets/CheckBoxWithLabelAndModel.hpp"
#include <ListView.hpp>
#include <Utils.hpp>
#include <BottomBar.hpp>

NewEditEventModel::NewEditEventModel(app::Application *app, bool mode24H) : application(app), mode24H(mode24H)
{}

int NewEditEventModel::getItemCount() const
{
    return internalData.size();
}

unsigned int NewEditEventModel::getMinimalItemHeight() const
{
    return style::window::calendar::item::checkBox::height;
}

void NewEditEventModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

gui::ListItem *NewEditEventModel::getItem(gui::Order order)
{
    return getRecord(order);
}

void NewEditEventModel::createData(bool allDayEvent)
{
    auto app = application;
    assert(app != nullptr);

    internalData.push_back(new gui::TextWithLabelItem(
        utils::localize.get("app_calendar_new_edit_event_name"),
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); },
        [app]() { app->getCurrentWindow()->selectSpecialCharacter(); }));

    internalData.push_back(new gui::CheckBoxWithLabelAndModel(
        application, utils::localize.get("app_calendar_new_edit_event_allday"), true, this));

    if (!allDayEvent) {
        internalData.push_back(new gui::EventTimeItem(
            utils::localize.get("app_calendar_new_edit_event_start"),
            mode24H,
            [app](const UTF8 &text) {
                app->getCurrentWindow()->bottomBarTemporaryMode(text, gui::BottomBar::Side::LEFT, false);
            },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

        internalData.push_back(new gui::EventTimeItem(
            utils::localize.get("app_calendar_new_edit_event_end"),
            mode24H,
            [app](const UTF8 &text) {
                app->getCurrentWindow()->bottomBarTemporaryMode(text, gui::BottomBar::Side::LEFT, false);
            },
            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));
    }

    internalData.push_back(new gui::SeveralOptionsItem(
        application,
        utils::localize.get("app_calendar_event_detail_reminder"),
        [app](const UTF8 &text) {
            app->getCurrentWindow()->bottomBarTemporaryMode(text, gui::BottomBar::Side::LEFT, false);
        },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    internalData.push_back(new gui::SeveralOptionsItem(
        application,
        utils::localize.get("app_calendar_event_detail_repeat"),
        [app](const UTF8 &text) {
            app->getCurrentWindow()->bottomBarTemporaryMode(text, gui::BottomBar::Side::LEFT, false);
        },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    for (auto &item : internalData) {
        item->deleteByList = false;
    }
}

void NewEditEventModel::loadData(bool allDayEvent)
{
    list->clear();
    eraseInternalData();

    createData(allDayEvent);

    for (auto &item : internalData) {
        if (item->onLoadCallback) {
            item->onLoadCallback();
        }
    }

    requestRecords(0, internalData.size());
}

void NewEditEventModel::loadDataWithoutTimeItem()
{
    list->clear();
    internalData.erase(internalData.begin() + 2, internalData.begin() + 4);
    requestRecords(0, internalData.size());
}

void NewEditEventModel::reloadDataWithTimeItem()
{
    auto app = application;
    assert(app != nullptr);

    list->clear();
    internalData.insert(internalData.begin() + 2,
                        new gui::EventTimeItem(
                            utils::localize.get("app_calendar_new_edit_event_start"),
                            mode24H,
                            [app](const UTF8 &text) {
                                app->getCurrentWindow()->bottomBarTemporaryMode(
                                    text, gui::BottomBar::Side::LEFT, false);
                            },
                            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    internalData.insert(internalData.begin() + 3,
                        new gui::EventTimeItem(
                            utils::localize.get("app_calendar_new_edit_event_end"),
                            mode24H,
                            [app](const UTF8 &text) {
                                app->getCurrentWindow()->bottomBarTemporaryMode(
                                    text, gui::BottomBar::Side::LEFT, false);
                            },
                            [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); }));

    for (auto &item : internalData) {
        item->deleteByList = false;
    }

    requestRecords(0, internalData.size());
}

void NewEditEventModel::saveData()
{
    for (auto &item : internalData) {
        if (item->onSaveCallback) {
            item->onSaveCallback();
        }
    }
}
