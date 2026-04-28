// linkage_demo.cpp                                                    -*-C++-*-
//
// Component Property 1: linkage_demo.h is the FIRST include.
// This verifies that linkage_demo.h is self-sufficient.

#ifndef INCLUDED_LINKAGE_DEMO
#include <linkage_demo.h>
#endif

#include <string>

namespace linkage {

// ============================================================
// DEFINITIONS of external-linkage entities declared in .h
// ============================================================

// Definition of the global extern variable declared in the header.
// There must be EXACTLY ONE definition across the entire program.
int g_externalCounter = 0;

// Definition of the static class member (also external linkage, one instance)
int LinkageExample::s_instanceCount = 0;

std::string externalLinkageFunction()
{
    return "I have external linkage — visible across all translation units";
}

LinkageExample::LinkageExample(int value)
: d_value(value)
{
    ++s_instanceCount;
}

int LinkageExample::getValue() const
{
    return d_value;
}


// ============================================================
// INTERNAL LINKAGE EXAMPLES
//
// These entities are defined in this .cpp but NOT declared in
// linkage_demo.h. They have INTERNAL linkage (via 'static' or
// anonymous namespace). They are completely invisible to other
// translation units — even at link time.
//
// This is Component Property 2: the .cpp does NOT leak these
// symbols outward. They stay private to this translation unit.
// ============================================================

namespace {

// Anonymous namespace gives internal linkage.
// This is the PREFERRED modern C++ way (vs. 'static' functions).

int s_privateCounter = 0;  // internal linkage: not visible outside this TU

std::string formatInternal(int value)
{
    return "internal:" + std::to_string(value);
}

}  // close anonymous namespace


// An old-style 'static' function also gives internal linkage.
// Prefer anonymous namespace for types and variables.
static int oldStyleInternalHelper(int x)
{
    return x * 2;
}


// ============================================================
// Using internal helpers in the implementation:
// These are called from external-linkage functions defined here,
// but the helpers themselves are NOT visible externally.
// ============================================================

// (The internal helpers above are used conceptually — this shows
//  that the .cpp can freely use them without exposing them.)
void useInternalHelpers()
{
    ++s_privateCounter;
    (void)formatInternal(s_privateCounter);
    (void)oldStyleInternalHelper(s_privateCounter);
}

}  // close namespace linkage
