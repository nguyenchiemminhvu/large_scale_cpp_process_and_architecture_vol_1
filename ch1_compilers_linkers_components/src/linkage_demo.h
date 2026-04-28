// linkage_demo.h                                                      -*-C++-*-
//
// Component: linkage_demo
//
// PURPOSE: Demonstrates the difference between external linkage,
//          internal linkage, and bindage as described in Lakos Chapter 1.
//
// CONCEPTS:
//   - External linkage: symbol visible across translation units
//   - Internal linkage: symbol visible only within one .cpp
//   - Bindage: how a header-declared name is bound to its definition
//   - ODR (One Definition Rule)
//
// Book reference: §1.3 Declarations, Definitions, and Linkage

#ifndef INCLUDED_LINKAGE_DEMO
#define INCLUDED_LINKAGE_DEMO

#include <string>

namespace linkage {

// ============================================================
// EXTERNAL LINKAGE EXAMPLES
// These are declared here in the header and defined in the .cpp.
// They have external linkage: visible and usable from ANY translation
// unit that includes this header.
// ============================================================

/// Returns the linkage type description for a variable declared extern.
/// Defined in linkage_demo.cpp (external bindage).
std::string externalLinkageFunction();

/// A global variable with external linkage.
/// DECLARED here (extern), DEFINED in linkage_demo.cpp.
/// Using 'extern' in the header prevents multiple definition errors.
extern int g_externalCounter;

// ============================================================
// INLINE FUNCTIONS (Special External Linkage Case)
//
// inline functions are defined in the header but have EXTERNAL linkage.
// Each translation unit gets its own "copy" of the code, but the linker
// merges duplicate definitions (benign ODR violation by rule).
//
// This is DIFFERENT from non-inline functions defined in headers,
// which violate the ODR (each translation unit gets a separate definition
// and the linker sees duplicates → link error).
// ============================================================

/// An inline function: defined here, external linkage.
/// Multiple TUs including this header all call the same logical function.
inline int inlineAdd(int a, int b)
{
    return a + b;
}

// ============================================================
// WHAT NOT TO DO: Defining non-inline functions in headers
//
// The following would cause "multiple definition" linker errors
// if more than one .cpp includes this header:
//
//   int badFunction() { return 42; }  // WRONG — ODR violation
//
// Rule: Non-inline, non-template function bodies belong in .cpp files.
// ============================================================


// ============================================================
// CLASS WITH MIXED LINKAGE
// ============================================================

/// Demonstrates class member linkage rules.
class LinkageExample {
    int d_value;   // private data member (no linkage — it's a class member)

  public:
    explicit LinkageExample(int value);

    /// Returns the stored value.
    /// Declared here (external linkage declaration), defined in .cpp.
    int getValue() const;

    /// An inline member function — defined here, external linkage.
    /// The definition is the same across all TUs (ODR guarantees this).
    void increment() { ++d_value; }

    /// Static member: ONE instance shared across all TUs.
    /// Must be defined in exactly ONE .cpp file.
    static int s_instanceCount;
};

}  // close namespace linkage

#endif  // INCLUDED_LINKAGE_DEMO
