// pkg_date.h                                                         -*-C++-*-
//
// Component: pkg_date
// Package:   pkg
//
// PURPOSE: Provides a simple date value type for demonstrating
//          Lakos naming and packaging conventions.
//
// CLASSES:
//   pkg::Date - a value type representing a calendar date (year/month/day)
//
// NAMING CONVENTION DEMONSTRATION (Lakos §2.4):
//   - Package name:    pkg          (short, all-lowercase)
//   - Component name:  pkg_date     (package prefix + underscore + local name)
//   - Header name:     pkg_date.h   (same as component name + .h)
//   - Implementation:  pkg_date.cpp (same as component name + .cpp)
//   - Test driver:     pkg_date.t.cpp
//   - Namespace:       pkg          (same as package name)
//   - Class name:      Date         (capitalized, no pkg_ prefix — namespace handles it)
//
// KEY INSIGHT: Using the class as pkg::Date, a developer immediately knows:
//   - Namespace → "pkg" → package "pkg" → UOR containing "pkg"
//   - Header     → "pkg_date.h"
//   - Component  → "pkg_date"
// All three names are derivable from the point of use. (Lakos §2.4.6)
//
// DESIGN RULES DEMONSTRATED:
//   DR-2.1: Component files share root name "pkg_date"
//   DR-2.2: Filename "pkg_date.h" is (assumed) enterprise-wide unique
//   DR-2.3: This component resides within package "pkg"
//   DR-2.4: Component name "pkg_date" begins with package name "pkg_"
//
// INCLUDE GUARD: INCLUDED_PKG_DATE

#ifndef INCLUDED_PKG_DATE
#define INCLUDED_PKG_DATE

#include <iosfwd>    // std::ostream (forward declaration — not full include)
#include <stdexcept> // std::out_of_range
#include <string>    // std::string

namespace pkg {

/// A simple date value type (year/month/day).
///
/// This class demonstrates:
///   1. Lakos naming conventions for library types
///   2. Value type design (copyable, comparable, streamable)
///   3. "Manifestly primitive" interface design (§3.2)
///      - Only operations that are inherently part of a date
///      - No business logic (e.g., "isBusinessDay" does NOT belong here)
///
/// Invariants:
///   - year  >= 1 && year  <= 9999
///   - month >= 1 && month <= 12
///   - day   >= 1 && day   <= (valid days for the given month/year)
class Date {
    int d_year;   // d_ prefix: data member (Lakos convention)
    int d_month;
    int d_day;

    // PRIVATE CLASS METHOD
    static bool isValidDate(int year, int month, int day);
    static bool isLeapYear(int year);
    static int  daysInMonth(int year, int month);

  public:
    // CREATORS

    /// Creates a date with the given year, month, and day.
    /// Throws std::out_of_range if the date is invalid.
    Date(int year, int month, int day);

    /// Creates a default date (year=1, month=1, day=1).
    Date();

    // MANIPULATORS

    /// Sets this date to the given year/month/day.
    /// Throws std::out_of_range if the date is invalid.
    void setYearMonthDay(int year, int month, int day);

    /// Advances this date by one day (handles month/year rollover).
    void addDays(int days);

    // ACCESSORS

    int year()  const;
    int month() const;
    int day()   const;

    /// Returns a string representation: "YYYY-MM-DD"
    std::string toString() const;

    /// Returns the number of days since a reference epoch (year 1, Jan 1).
    /// Used for date arithmetic.
    int toDayNumber() const;
};

// FREE OPERATORS
//
// DESIGN RULE (Lakos §2.4.8): A free operator may be declared in a
// component's header ONLY if at least one parameter type is defined
// in that same component. Both sides of these operators use pkg::Date.

bool operator==(const Date& lhs, const Date& rhs);
bool operator!=(const Date& lhs, const Date& rhs);
bool operator< (const Date& lhs, const Date& rhs);
bool operator<=(const Date& lhs, const Date& rhs);
bool operator> (const Date& lhs, const Date& rhs);
bool operator>=(const Date& lhs, const Date& rhs);

/// Outputs date in "YYYY-MM-DD" format.
/// The ostream parameter uses the HETEROGENEOUS free operator rule:
/// since one parameter (rhs) is pkg::Date defined here, this is OK.
std::ostream& operator<<(std::ostream& stream, const Date& date);

}  // close namespace pkg

#endif  // INCLUDED_PKG_DATE
