#include "MonthModel.hpp"
#include <time/time_locale.hpp>

MonthModel::MonthModel(date::year_month_day yearMonthDay)
{
    this->month                                = yearMonthDay.month();
    this->year                                 = yearMonthDay.year();
    date::year_month_day_last yearMonthDayLast = this->year / this->month / date::last;
    this->lastDay                              = static_cast<unsigned>(yearMonthDayLast.day());
    date::year_month_day yearMonthDayFirst     = this->year / this->month / 1;
    this->firstWeekDayNumb                     = date::weekday{yearMonthDayFirst}.c_encoding();
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

YearMonthDay MonthModel::parseDateFromDB(TimePoint dateDB)
{
    return TimePointToYearMonthDay(dateDB);
}

TimePoint MonthModel::parseDateToDB(YearMonthDay date)
{
    return TimePointFromYearMonthDay(date);
}

std::string MonthModel::getMonthText()
{
    unsigned int monthUInt = static_cast<unsigned>(month);
    std::string monthStr   = utils::time::Locale::get_month(utils::time::Locale::Month(monthUInt - 1));
    return monthStr;
}

std::string MonthModel::getMonthYearText()
{
    int yearUInt        = static_cast<decltype(yearUInt)>(year);
    std::string yearStr = std::to_string(yearUInt);
    std::string monthStr;
    unsigned int monthUInt = static_cast<unsigned>(month);
    monthStr               = utils::time::Locale::get_month(utils::time::Locale::Month(monthUInt - 1));

    return monthStr + " " + yearStr;
}
