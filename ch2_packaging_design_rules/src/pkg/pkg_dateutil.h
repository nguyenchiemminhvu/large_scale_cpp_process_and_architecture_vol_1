// pkg_dateutil.h                                                     -*-C++-*-
//
// Component: pkg_dateutil
// Package:   pkg
//
// PURPOSE: Provides utility functions for pkg::Date.
//
// CLASSES:
//   pkg::DateUtil - utility struct with static date operations
//
// DESIGN PATTERN: UTILITY STRUCT (Lakos §3.2)
//
// CONCEPT: A "utility struct" (or "utility class") is a struct with only
// static methods and no data members. It serves as a namespace for related
// free functions that operate on one or more types.
//
// WHY A STRUCT INSTEAD OF FREE FUNCTIONS?
//   1. Avoids polluting the namespace with individual function names
//   2. Enables ADL-free lookup (no accidental name collisions)
//   3. Makes it clear these are related stateless operations
//   4. Provides a clear "single component" anchor for the operations
//
// NAMING CONVENTION:
//   The "Util" suffix signals: "I'm a utility struct with static methods."
//   Common patterns:
//     pkg_dateutil → pkg::DateUtil     (utilities for Date)
//     pkg_mathutil → pkg::MathUtil     (math utilities)
//     pkg_strutil  → pkg::StringUtil   (string utilities)
//
// DEPENDENCY NOTE:
//   This component depends on pkg_date (it uses pkg::Date).
//   This creates the dependency: pkg_dateutil → pkg_date
//   Both are in package "pkg", so this is an intra-package dependency.
//
// INCLUDE GUARD: INCLUDED_PKG_DATEUTIL

#ifndef INCLUDED_PKG_DATEUTIL
#define INCLUDED_PKG_DATEUTIL

#ifndef INCLUDED_PKG_DATE
#include <pkg/pkg_date.h>
#endif

#include <string>

namespace pkg {

/// Utility struct providing operations on pkg::Date values.
///
/// All methods are static. This struct has no data members.
/// Instantiating it is pointless; it serves purely as a namespace.
struct DateUtil {
    // CLASS METHODS

    /// Returns true if 'year' is a leap year (Gregorian calendar).
    static bool isLeapYear(int year);

    /// Returns the number of days in 'month' of 'year'.
    static int daysInMonth(int year, int month);

    /// Returns the day of the week as an integer (0=Sunday, 6=Saturday).
    static int dayOfWeek(const Date& date);

    /// Returns the name of the day of the week ("Monday", "Tuesday", etc.)
    static std::string dayOfWeekName(const Date& date);

    /// Returns the number of days between two dates (end - start).
    /// Negative if end is before start.
    static int daysBetween(const Date& start, const Date& end);

    /// Returns the next occurrence of the given weekday (0=Sun..6=Sat)
    /// on or after 'date'. Returns 'date' if it is already that weekday.
    static Date nextWeekday(const Date& date, int weekday);

    /// Returns true if 'date' falls on a weekend (Saturday or Sunday).
    static bool isWeekend(const Date& date);

  private:
    DateUtil() = delete;  // utility struct: not instantiable
};

}  // close namespace pkg

#endif  // INCLUDED_PKG_DATEUTIL
