#include "CalendarData.hpp"
#include "dateCommon.hpp"

WeekDaysRepeatData::WeekDaysRepeatData()
{
    for (uint32_t i = 0; i < style::window::calendar::week_days_number; i++) {
        weekDays[i] = false;
    }
}

[[nodiscard]] auto WeekDaysRepeatData::getData(const uint32_t &weekDay) const -> bool
{
    if (weekDay >= style::window::calendar::week_days_number) {
        LOG_ERROR("Weekday out of range (%d)", static_cast<int>(weekDay));
        return false;
    }
    return weekDays[weekDay];
}

void WeekDaysRepeatData::setData(const uint32_t &weekDay)
{
    if (weekDay >= style::window::calendar::week_days_number) {
        LOG_ERROR("Weekday out of range (%d)", static_cast<int>(weekDay));
        return;
    }
    if (weekDays[weekDay]) {
        weekDays[weekDay] = false;
    }
    else {
        weekDays[weekDay] = true;
    }
}
