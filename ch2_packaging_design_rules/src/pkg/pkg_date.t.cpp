// pkg_date.t.cpp                                                     -*-C++-*-
//
// TEST DRIVER for the pkg_date component.
//
// DEMONSTRATES:
//   - Hierarchical Testability Requirement (Lakos §2.14)
//   - Test driver naming convention: <component>.t.cpp
//   - Uniform invocation interface (returns 0 on success)
//   - Dependencies: ONLY pkg_date (+ standard library)
//
// BUILD NOTE: This test driver links ONLY against the pkg library
// (which contains pkg_date.cpp). No other application-level code needed.

#ifndef INCLUDED_PKG_DATE
#include <pkg/pkg_date.h>
#endif

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>

// ============================================================
// TEST CASES
// ============================================================

static int testCase1_DefaultConstruction()
{
    pkg::Date d;
    assert(d.year()  == 1);
    assert(d.month() == 1);
    assert(d.day()   == 1);
    assert(d.toString() == "0001-01-01");
    std::cout << "  [PASS] Test 1: Default construction\n";
    return 0;
}

static int testCase2_ValueConstruction()
{
    pkg::Date d(2024, 6, 15);
    assert(d.year()  == 2024);
    assert(d.month() == 6);
    assert(d.day()   == 15);
    assert(d.toString() == "2024-06-15");
    std::cout << "  [PASS] Test 2: Value construction\n";
    return 0;
}

static int testCase3_InvalidDateThrows()
{
    bool caught = false;
    try {
        pkg::Date d(2024, 13, 1);  // month 13 is invalid
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        pkg::Date d(2024, 2, 30);  // Feb 30 is invalid
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    std::cout << "  [PASS] Test 3: Invalid date throws\n";
    return 0;
}

static int testCase4_LeapYear()
{
    // Feb 29 in leap year
    pkg::Date leapDay(2024, 2, 29);
    assert(leapDay.day() == 29);

    // Feb 29 in non-leap year
    bool caught = false;
    try {
        pkg::Date d(2023, 2, 29);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    // Feb 29 in century year (not a leap year unless divisible by 400)
    caught = false;
    try {
        pkg::Date d(1900, 2, 29);  // 1900 is NOT a leap year
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    // But 2000 IS a leap year (divisible by 400)
    pkg::Date y2k(2000, 2, 29);
    assert(y2k.day() == 29);

    std::cout << "  [PASS] Test 4: Leap year\n";
    return 0;
}

static int testCase5_Comparison()
{
    pkg::Date d1(2024, 1, 1);
    pkg::Date d2(2024, 1, 2);
    pkg::Date d3(2024, 1, 1);

    assert(d1 == d3);
    assert(d1 != d2);
    assert(d1 <  d2);
    assert(d1 <= d2);
    assert(d2 >  d1);
    assert(d2 >= d1);
    assert(d1 <= d3);
    assert(d1 >= d3);

    std::cout << "  [PASS] Test 5: Comparison operators\n";
    return 0;
}

static int testCase6_AddDays()
{
    pkg::Date d(2024, 1, 30);
    d.addDays(3);
    assert(d.year()  == 2024);
    assert(d.month() == 2);
    assert(d.day()   == 2);

    // Cross year boundary
    pkg::Date yearEnd(2023, 12, 31);
    yearEnd.addDays(1);
    assert(yearEnd.year()  == 2024);
    assert(yearEnd.month() == 1);
    assert(yearEnd.day()   == 1);

    std::cout << "  [PASS] Test 6: Add days\n";
    return 0;
}

static int testCase7_StreamOutput()
{
    pkg::Date d(2024, 3, 5);
    std::ostringstream oss;
    oss << d;
    assert(oss.str() == "2024-03-05");

    std::cout << "  [PASS] Test 7: Stream output\n";
    return 0;
}

// ============================================================
// Main — Hierarchical Testability: uniform invocation interface
// ============================================================

int main()
{
    std::cout << "=== pkg_date Component Test Driver ===\n\n";

    int failures = 0;
    failures += testCase1_DefaultConstruction();
    failures += testCase2_ValueConstruction();
    failures += testCase3_InvalidDateThrows();
    failures += testCase4_LeapYear();
    failures += testCase5_Comparison();
    failures += testCase6_AddDays();
    failures += testCase7_StreamOutput();

    if (failures == 0) {
        std::cout << "\n[ALL TESTS PASSED]\n";
        std::cout << "\nHierarchical Testability demonstrated:\n";
        std::cout << "  - This test only links pkg_date (+ std library)\n";
        std::cout << "  - No application code needed to test this component\n";
        std::cout << "  - Return code 0 = all pass (uniform interface)\n";
    } else {
        std::cout << "\n[" << failures << " TEST(S) FAILED]\n";
    }

    return failures;
}
