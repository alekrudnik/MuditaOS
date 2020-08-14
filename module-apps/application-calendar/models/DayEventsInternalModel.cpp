#include "DayEventsInternalModel.hpp"
#include "application-calendar/widgets/DayEventsItem.hpp"
#include <ListView.hpp>
#include <Utils.hpp>
#include <algorithm>

DayEventsInternalModel::DayEventsInternalModel(app::Application *app) : application(app)
{}

unsigned int DayEventsInternalModel::requestRecordsCount()
{
    return internalData.size();
}

unsigned int DayEventsInternalModel::getMinimalItemHeight() const
{
    return style::window::calendar::item::dayEvents::height;
}

void DayEventsInternalModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

gui::ListItem *DayEventsInternalModel::getItem(gui::Order order)
{
    return getRecord(order);
}

void DayEventsInternalModel::loadData(std::unique_ptr<std::vector<EventsRecord>> records)
{
    auto app = application;
    assert(app != nullptr);

    list->clear();
    eraseInternalData();

    std::sort(records->begin(), records->end(), [](const EventsRecord &first, const EventsRecord &second) {
        return first.date_from < second.date_from;
    });

    for (auto &record : *records) {
        auto item = new gui::DayEventsItem();
        item->setEvent(std::make_shared<EventsRecord>(record));
        item->activatedCallback = [=](gui::Item &item) {
            LOG_INFO("Switch to event detail window");
            app->switchWindow(style::window::calendar::name::details_window);
            return true;
        };
        internalData.push_back(item);
    }

    for (auto &item : internalData) {
        item->deleteByList = false;
    }

    list->rebuildList();
}
