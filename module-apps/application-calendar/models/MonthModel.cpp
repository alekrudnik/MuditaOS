// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MonthModel.hpp"
#include <time/time_locale.hpp>
#include <time/time_conversion.hpp>

date::year_month_day MonthModel::getYearMonthDayFromTimePoint(utils::time::TimePoint timePoint) const
{
    return date::year_month_day{date::floor<date::days>(timePoint)};
}

uint32_t MonthModel::getEventDurationInDays(const EventsRecord &record) const
{
    auto eventDuration = utils::time::Duration(record.date_till, record.date_from);
    auto days = eventDuration.getRoundedDays();
    return days.count();
}

uint32_t MonthModel::getDayIndex(utils::time::TimePoint date) const
{
    date::year_month_day recordDate = utils::time::CalendarConversion::TimePointToYearMonthDay(date);
    return (static_cast<unsigned>(recordDate.day()) - 1);
}

MonthModel::MonthModel(date::year_month_day yearMonthDay)
{
    this->yearMonth                            = YearMonth(yearMonthDay);
    this->month                                = yearMonthDay.month();
    this->year                                 = yearMonthDay.year();
    date::year_month_day_last yearMonthDayLast = this->year / this->month / date::last;
    this->lastDay                              = static_cast<unsigned>(yearMonthDayLast.day());
    date::year_month_day yearMonthDayFirst     = this->year / this->month / 1;
    this->firstWeekDayNumb                     = date::weekday{yearMonthDayFirst}.c_encoding();
}

void MonthModel::markEventsInDays(const std::vector<EventsRecord> &records, std::array<bool, 31> &isDayEmpty)
{
    for (auto &rec : records) {
        auto eventYearMonthFrom = YearMonth(getYearMonthDayFromTimePoint(rec.date_from));
        auto eventYearMonthTill = YearMonth(getYearMonthDayFromTimePoint(rec.date_till));

        if (eventYearMonthFrom < this->yearMonth && this->yearMonth < eventYearMonthTill) {
            for (uint32_t i = 0; i < utils::time::max_month_day; i++) {
                isDayEmpty[i] = false;
            }
            return;
        }

        int durationInDays = getEventDurationInDays(rec);

        if (this->yearMonth == eventYearMonthFrom) {
            for (int i = getDayIndex(rec.date_from); i < utils::time::max_month_day && durationInDays >= 0; i++, durationInDays--) {
                isDayEmpty[i] = false;
            }
        }

        if (this->yearMonth == eventYearMonthTill) {
            for (int i = getDayIndex(rec.date_till); i >= 0 && durationInDays >= 0; i--, durationInDays--) {
                isDayEmpty[i] = false;
            }
        }
    }
}

date::year MonthModel::getYear()
{
    return year;
}

date::month MonthModel::getMonth()
{
    return month;
}

uint32_t MonthModel::getLastDay()
{
    return this->lastDay;
}

uint32_t MonthModel::getFirstWeekOffset()
{
    if (this->firstWeekDayNumb == 0) {
        return 6;
    }
    else {
        return this->firstWeekDayNumb - 1;
    }
}

std::vector<std::string> MonthModel::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::string MonthModel::getMonthText()
{
    std::string monthStr = utils::time::Locale::get_month(month);
    return monthStr;
}

std::string MonthModel::getMonthYearText()
{
    return utils::time::Locale::get_month(month) + " " + utils::to_string(year);
}
