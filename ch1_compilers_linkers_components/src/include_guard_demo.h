// include_guard_demo.h                                                -*-C++-*-
//
// Demonstrates the proper include guard pattern from Lakos Chapter 1.
//
// INCLUDE GUARD FORMAT (Lakos convention):
//   #ifndef INCLUDED_<UPPERCASE_COMPONENT_NAME>
//   #define INCLUDED_<UPPERCASE_COMPONENT_NAME>
//   ... header content ...
//   #endif  // INCLUDED_<UPPERCASE_COMPONENT_NAME>
//
// Why this naming convention?
//   1. "INCLUDED_" prefix avoids collisions with other macros.
//   2. Using the component name ensures uniqueness across the codebase.
//   3. Consistent naming enables automated tooling to verify guards.
//
// Book reference: §1.5 Include Directives and Include Guards

#ifndef INCLUDED_INCLUDE_GUARD_DEMO
#define INCLUDED_INCLUDE_GUARD_DEMO

// ============================================================
// REDUNDANT (EXTERNAL) INCLUDE GUARDS
//
// In .cpp files (and sometimes in other headers), Lakos recommends
// adding redundant external guards BEFORE the #include directive:
//
//   #ifndef INCLUDED_SOME_HEADER
//   #include <some_header.h>
//   #endif
//
// WHY: At very large scale (thousands of headers), the preprocessor
// opens and closes files even when the include guard would prevent
// re-processing. External guards prevent even the file-open cost.
//
// EXAMPLE: Instead of just:
//   #include <vector>
//
// You would write (in performance-critical build contexts):
//   #ifndef INCLUDED_BSLSTL_VECTOR
//   #include <bslstl_vector.h>
//   #endif
//
// For standard library headers, the external guards look like:
//   #ifndef INCLUDED_VECTOR
//   #include <vector>
//   #define INCLUDED_VECTOR
//   #endif
//
// In normal projects, #pragma once or standard guards are fine.
// The external guard technique is a large-scale build optimization.
// ============================================================

#include <string>
#include <vector>

namespace guard_demo {

/// A simple type to demonstrate that multiple inclusions of this header
/// do not cause duplicate definition errors.
struct Point {
    double x;
    double y;
};

/// Returns a string describing the include guard technique.
std::string describeIncludeGuards();

/// Returns a vector of best practices for include guards.
std::vector<std::string> bestPractices();

}  // close namespace guard_demo

#endif  // INCLUDED_INCLUDE_GUARD_DEMO
