// include_guard_demo.cpp                                              -*-C++-*-
//
// Component Property 1: include_guard_demo.h is the FIRST include.

#ifndef INCLUDED_INCLUDE_GUARD_DEMO
#include <include_guard_demo.h>
#endif

namespace guard_demo {

std::string describeIncludeGuards()
{
    return
        "Include guards prevent a header from being processed more than once\n"
        "within a single translation unit. Without them, re-including a header\n"
        "that defines types would cause 'redefinition' compiler errors.\n"
        "\n"
        "The Lakos convention uses INCLUDED_<NAME> as the guard macro,\n"
        "where <NAME> is the component name in ALL_UPPERCASE.";
}

std::vector<std::string> bestPractices()
{
    return {
        "Use #ifndef INCLUDED_<COMPONENT> pattern for include guards",
        "Make the guard macro name match the component filename exactly",
        "Put the #endif comment: // INCLUDED_<COMPONENT>",
        "Use external guards in .cpp files for build-time optimization",
        "Never use 'using namespace' in a header outside function scope",
        "The .h must be the first #include in its own .cpp (CP1 test)"
    };
}

}  // close namespace guard_demo
