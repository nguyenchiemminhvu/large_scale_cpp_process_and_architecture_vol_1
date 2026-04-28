// pkg_date.cpp                                                       -*-C++-*-
//
// Component Property 1: pkg_date.h MUST be the first #include.
// This guarantees self-sufficiency of the header.

#ifndef INCLUDED_PKG_DATE
#include <pkg/pkg_date.h>
#endif

#include <ostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace pkg {

// ============================================================
// PRIVATE HELPER IMPLEMENTATIONS
// ============================================================

bool Date::isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int Date::daysInMonth(int year, int month)
{
    static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month];
}

bool Date::isValidDate(int year, int month, int day)
{
    if (year  < 1 || year  > 9999) return false;
    if (month < 1 || month > 12)   return false;
    if (day   < 1 || day   > daysInMonth(year, month)) return false;
    return true;
}

// ============================================================
// PUBLIC CONSTRUCTORS
// ============================================================

Date::Date()
: d_year(1)
, d_month(1)
, d_day(1)
{
}

Date::Date(int year, int month, int day)
: d_year(year)
, d_month(month)
, d_day(day)
{
    if (!isValidDate(year, month, day)) {
        throw std::out_of_range(
            "pkg::Date: invalid date " + std::to_string(year) + "-" +
            std::to_string(month) + "-" + std::to_string(day));
    }
}

// ============================================================
// MANIPULATORS
// ============================================================

void Date::setYearMonthDay(int year, int month, int day)
{
    if (!isValidDate(year, month, day)) {
        throw std::out_of_range("pkg::Date::setYearMonthDay: invalid date");
    }
    d_year  = year;
    d_month = month;
    d_day   = day;
}

void Date::addDays(int days)
{
    // Simple implementation: convert to day number, add, convert back
    int dayNum = toDayNumber() + days;
    if (dayNum < 1) {
        throw std::out_of_range("pkg::Date::addDays: result before epoch");
    }

    // Convert back from day number to year/month/day
    int year = 1;
    int daysInYear = isLeapYear(year) ? 366 : 365;

    while (dayNum > daysInYear) {
        dayNum -= daysInYear;
        ++year;
        if (year > 9999) {
            throw std::out_of_range("pkg::Date::addDays: result exceeds max year");
        }
        daysInYear = isLeapYear(year) ? 366 : 365;
    }

    int month = 1;
    int dim = daysInMonth(year, month);
    while (dayNum > dim) {
        dayNum -= dim;
        ++month;
        dim = daysInMonth(year, month);
    }

    d_year  = year;
    d_month = month;
    d_day   = dayNum;
}

// ============================================================
// ACCESSORS
// ============================================================

int Date::year()  const { return d_year; }
int Date::month() const { return d_month; }
int Date::day()   const { return d_day; }

std::string Date::toString() const
{
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << d_year  << "-"
        << std::setw(2) << d_month << "-"
        << std::setw(2) << d_day;
    return oss.str();
}

int Date::toDayNumber() const
{
    // Days from year 1, Jan 1
    int days = 0;
    for (int y = 1; y < d_year; ++y) {
        days += isLeapYear(y) ? 366 : 365;
    }
    for (int m = 1; m < d_month; ++m) {
        days += daysInMonth(d_year, m);
    }
    days += d_day;
    return days;
}

// ============================================================
// FREE OPERATORS
// ============================================================

bool operator==(const Date& lhs, const Date& rhs)
{
    return lhs.year()  == rhs.year()  &&
           lhs.month() == rhs.month() &&
           lhs.day()   == rhs.day();
}

bool operator!=(const Date& lhs, const Date& rhs) { return !(lhs == rhs); }

bool operator<(const Date& lhs, const Date& rhs)
{
    return lhs.toDayNumber() < rhs.toDayNumber();
}

bool operator<=(const Date& lhs, const Date& rhs) { return !(rhs < lhs); }
bool operator> (const Date& lhs, const Date& rhs) { return rhs < lhs; }
bool operator>=(const Date& lhs, const Date& rhs) { return !(lhs < rhs); }

std::ostream& operator<<(std::ostream& stream, const Date& date)
{
    return stream << date.toString();
}

}  // close namespace pkg
