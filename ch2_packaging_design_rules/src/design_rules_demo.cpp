// design_rules_demo.cpp                                              -*-C++-*-
//
// Demonstrates all key design rules from Chapter 2:
//   - Naming conventions (§2.4)
//   - Package structure
//   - No 'using namespace' in headers
//   - Utility struct pattern
//   - Dependency structure
//   - Logical/physical coherence (§2.3)
//   - Hierarchical testability (§2.14)

#ifndef INCLUDED_PKG_DATE
#include <pkg/pkg_date.h>
#endif

#ifndef INCLUDED_PKG_DATEUTIL
#include <pkg/pkg_dateutil.h>
#endif

#include <iostream>
#include <string>
#include <vector>

// ============================================================
// SECTION 1: Naming Conventions Demo
// ============================================================

static void demoNamingConventions()
{
    std::cout << "--- Naming Conventions (Lakos §2.4) ---\n\n";

    std::cout << "Package: pkg\n";
    std::cout << "  Component: pkg_date\n";
    std::cout << "    Header: pkg/pkg_date.h\n";
    std::cout << "    Impl:   pkg/pkg_date.cpp\n";
    std::cout << "    Test:   pkg/pkg_date.t.cpp\n";
    std::cout << "    Class:  pkg::Date\n\n";

    std::cout << "  Component: pkg_dateutil\n";
    std::cout << "    Header: pkg/pkg_dateutil.h\n";
    std::cout << "    Impl:   pkg/pkg_dateutil.cpp\n";
    std::cout << "    Struct: pkg::DateUtil (utility struct)\n\n";

    std::cout << "Key insight: From 'pkg::Date', you know:\n";
    std::cout << "  Namespace 'pkg'  → package directory 'pkg/'\n";
    std::cout << "  Class name 'Date' → component 'pkg_date' → files pkg_date.h/.cpp\n";
    std::cout << "  (Lakos §2.4.6: Point of use identifies physical location)\n\n";
}

// ============================================================
// SECTION 2: pkg::Date usage (value type)
// ============================================================

static void demoPkgDate()
{
    std::cout << "--- pkg::Date Usage ---\n\n";

    // Construction and basic access
    pkg::Date birthday(1990, 7, 4);
    std::cout << "Birthday:     " << birthday << "\n";
    std::cout << "Year:         " << birthday.year() << "\n";
    std::cout << "Month:        " << birthday.month() << "\n";
    std::cout << "Day:          " << birthday.day() << "\n\n";

    // Value semantics (copy)
    pkg::Date copy = birthday;
    copy.addDays(365);
    std::cout << "One year later: " << copy << "\n";
    std::cout << "Original unchanged: " << birthday << "\n\n";

    // Comparison
    std::cout << "birthday < copy: " << (birthday < copy ? "true" : "false") << "\n";
    std::cout << "birthday == copy: " << (birthday == copy ? "true" : "false") << "\n\n";
}

// ============================================================
// SECTION 3: Utility struct pattern (pkg::DateUtil)
// ============================================================

static void demoPkgDateUtil()
{
    std::cout << "--- Utility Struct Pattern (pkg::DateUtil) ---\n\n";

    std::cout << "The 'utility struct' pattern:\n";
    std::cout << "  - A struct with ONLY static methods, no data members\n";
    std::cout << "  - Serves as a namespace for related stateless operations\n";
    std::cout << "  - Deletes its constructor (not instantiable)\n";
    std::cout << "  - Named with 'Util' suffix by convention\n\n";

    pkg::Date today(2024, 3, 15);  // A Friday
    std::cout << "Date: " << today << "\n";
    std::cout << "Day of week: " << pkg::DateUtil::dayOfWeekName(today) << "\n";
    std::cout << "Is weekend: " << (pkg::DateUtil::isWeekend(today) ? "yes" : "no") << "\n";
    std::cout << "Days in this month: " << pkg::DateUtil::daysInMonth(2024, 3) << "\n";
    std::cout << "Is 2024 a leap year: " << (pkg::DateUtil::isLeapYear(2024) ? "yes" : "no") << "\n\n";

    pkg::Date start(2024, 1, 1);
    pkg::Date end(2024, 12, 31);
    std::cout << "Days between 2024-01-01 and 2024-12-31: "
              << pkg::DateUtil::daysBetween(start, end) << "\n\n";

    pkg::Date friday(2024, 3, 15);  // Friday
    pkg::Date nextMonday = pkg::DateUtil::nextWeekday(friday, 1);  // 1=Monday
    std::cout << "Next Monday after " << friday << ": " << nextMonday << "\n\n";
}

// ============================================================
// SECTION 4: Design Rules Illustrated
// ============================================================

static void demoDesignRules()
{
    std::cout << "--- Design Rules Illustrated (Lakos Chapter 2) ---\n\n";

    std::cout << "RULE: 'using namespace' is FORBIDDEN in headers\n";
    std::cout << "  BAD:  using namespace pkg;  // in header\n";
    std::cout << "  GOOD: Always use fully qualified names: pkg::Date\n";
    std::cout << "  WHY:  At scale, 'using' in headers poisons every TU that\n";
    std::cout << "        includes the header, making names ambiguous.\n\n";

    std::cout << "RULE: Component name = package_prefix + '_' + local_name\n";
    std::cout << "  OK:  pkg_date.h (prefix 'pkg' + '_date')\n";
    std::cout << "  BAD: date.h     (no package prefix — not unique enough)\n";
    std::cout << "  BAD: pkg-date.h (hyphen not allowed)\n\n";

    std::cout << "RULE: Free operators anchored to component that defines their types\n";
    std::cout << "  OK:  operator==(Date, Date) in pkg_date.h  (Date defined here)\n";
    std::cout << "  OK:  operator<<(ostream, Date) in pkg_date.h (Date defined here)\n";
    std::cout << "  BAD: operator==(Square, Rectangle) in a 3rd unrelated component\n\n";

    std::cout << "RULE: Utility struct for multi-type operations\n";
    std::cout << "  If an operation spans types from two different components,\n";
    std::cout << "  create a third 'Util' component rather than picking one arbitrarily.\n";
    std::cout << "  Example: pkg_squarerectangleutil.h contains SquareRectangleUtil::areEqual()\n\n";

    std::cout << "RULE: No cyclic physical dependencies\n";

    // Demonstrate the dependency order:
    std::cout << "  Dependency graph (this demo):\n";
    std::cout << "    design_rules_demo (this file)\n";
    std::cout << "      ↓ uses\n";
    std::cout << "    pkg_dateutil  (Level 2 in package 'pkg')\n";
    std::cout << "      ↓ uses\n";
    std::cout << "    pkg_date      (Level 1 in package 'pkg')\n";
    std::cout << "      ↓ uses\n";
    std::cout << "    std library only\n\n";
}

// ============================================================
// SECTION 5: Package hierarchy visualization
// ============================================================

static void demoPackageHierarchy()
{
    std::cout << "--- Package Hierarchy (Lakos §2.2, §2.8-2.9) ---\n\n";

    std::cout << "Three levels of physical aggregation:\n\n";

    std::cout << "Level 3 (UOR / Package Group):\n";
    std::cout << "  mylib/           ← deployed as libmylib.a\n";
    std::cout << "    group/\n";
    std::cout << "      mylib.mem    ← lists packages: pkg, pkgu\n";
    std::cout << "      mylib.dep    ← allowed external deps\n\n";

    std::cout << "Level 2 (Package):\n";
    std::cout << "  mylib/pkg/       ← package directory\n";
    std::cout << "    package/\n";
    std::cout << "      pkg.mem      ← lists: pkg_date, pkg_dateutil\n";
    std::cout << "      pkg.dep      ← allowed deps within group\n\n";

    std::cout << "Level 1 (Component):\n";
    std::cout << "  mylib/pkg/pkg_date.h     ← interface\n";
    std::cout << "  mylib/pkg/pkg_date.cpp   ← implementation\n";
    std::cout << "  mylib/pkg/pkg_date.t.cpp ← test driver\n\n";

    std::cout << "Naming flow:\n";
    std::cout << "  Group 'mylib' → Package 'pkg' → Component 'pkg_date'\n";
    std::cout << "  Type: 'pkg::Date'\n";
    std::cout << "  All three names are derivable from the point of use!\n\n";
}

// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "=== Chapter 2: Packaging and Design Rules Demo ===\n\n";

    demoNamingConventions();
    demoPkgDate();
    demoPkgDateUtil();
    demoDesignRules();
    demoPackageHierarchy();

    std::cout << "=== Summary ===\n\n";
    std::cout << "Chapter 2 establishes the physical packaging hierarchy:\n";
    std::cout << "  Component < Package < Package Group (UOR)\n\n";
    std::cout << "Key design rules:\n";
    std::cout << "  1. Component name = package_prefix + '_' + local_name\n";
    std::cout << "  2. No 'using namespace' in headers (outside functions)\n";
    std::cout << "  3. Free operators only in the component that owns the types\n";
    std::cout << "  4. Utility structs for multi-type operations\n";
    std::cout << "  5. All dependencies must be acyclic at every level\n";
    std::cout << "  6. Every component must be independently unit-testable\n";

    return 0;
}
