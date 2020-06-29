#include "CalendarModel.hpp"
#include "ListView.hpp"
#include "gui/widgets/Label.hpp"
#include "../widgets/CalendarItem.hpp"
#include "../widgets/CalendarItem.cpp"

CalendarModel::CalendarModel(app::Application *app)
{}

CalendarModel::~CalendarModel()
{}

void CalendarModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    /// TODO: Connect to database
    list->onProviderDataUpdate();   // listView refresh()
}

unsigned int CalendarModel::getMinimalItemHeight()
{
    return 90; // item height in ListView
}

gui::ListItem *CalendarModel::getItem(gui::Order order)
{
    /// TODO: Get items from database
    ///       Fix bad ListView items display
    ///       Fix navigation

    // ----- temporary --------
    static int items = getItemCount()+1;
    if(items == 0){
        items = getItemCount();
        return nullptr;
    }
    // ------------------------
    gui::CalendarItem *item = new gui::CalendarItem(this);
    //LOG_DEBUG("Counts ..");
    items--;
    return item;
}