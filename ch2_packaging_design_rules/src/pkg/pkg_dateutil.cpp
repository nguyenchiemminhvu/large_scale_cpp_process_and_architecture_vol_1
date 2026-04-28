// pkg_dateutil.cpp                                                   -*-C++-*-
//
// Component Property 1: pkg_dateutil.h MUST be the first #include.

#ifndef INCLUDED_PKG_DATEUTIL
#include <pkg/pkg_dateutil.h>
#endif

#include <array>
#include <string>

namespace pkg {

namespace {
// Internal helper: check if year is a leap year (anonymous namespace = internal linkage)
bool leapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
}  // close anonymous namespace

bool DateUtil::isLeapYear(int year)
{
    return leapYear(year);
}

int DateUtil::daysInMonth(int year, int month)
{
    static const std::array<int, 13> days = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (month == 2 && leapYear(year)) return 29;
    return days[static_cast<std::size_t>(month)];
}

int DateUtil::dayOfWeek(const Date& date)
{
    // Zeller's congruence (simplified for Gregorian calendar)
    int m = date.month();
    int y = date.year();
    int d = date.day();

    if (m < 3) {
        m += 12;
        --y;
    }
    int k = y % 100;
    int j = y / 100;

    int h = (d + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    // Zeller gives: 0=Sat, 1=Sun, 2=Mon, ..., 6=Fri
    // Convert to: 0=Sun, 1=Mon, ..., 6=Sat
    return (h + 6) % 7;
}

std::string DateUtil::dayOfWeekName(const Date& date)
{
    static const std::array<std::string, 7> names = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };
    return names[static_cast<std::size_t>(dayOfWeek(date))];
}

int DateUtil::daysBetween(const Date& start, const Date& end)
{
    return end.toDayNumber() - start.toDayNumber();
}

Date DateUtil::nextWeekday(const Date& date, int weekday)
{
    int current = dayOfWeek(date);
    int daysToAdd = (weekday - current + 7) % 7;

    Date result = date;
    result.addDays(daysToAdd);
    return result;
}

bool DateUtil::isWeekend(const Date& date)
{
    int dow = dayOfWeek(date);
    return dow == 0 || dow == 6;  // 0=Sunday, 6=Saturday
}

}  // close namespace pkg
