// my_stack.cpp                                                        -*-C++-*-
//
// Component Property 1 Test:
//   The FIRST #include in a component's .cpp must be its OWN .h file.
//   This guarantees that my_stack.h is self-sufficient (compilable standalone).
//   If my_stack.h were missing an #include, this compilation would catch it.
//
// Component Property 2:
//   This file defines ONLY what is declared in my_stack.h.
//   No "extra" symbols with external linkage are created here.
//
// Since my::Stack<T> is a class template, all definitions are in the header.
// This .cpp file exists to satisfy Component Property 1 (the self-sufficiency
// test) and to provide a translation unit for any non-template utilities.

#ifndef INCLUDED_MY_STACK
#include <my_stack.h>  // Component Property 1: ALWAYS first
#endif

// This file is intentionally minimal for a template component.
// In a non-template component, the function bodies would appear here.

// ============================================================
// DEMONSTRATION of Component Property 2:
//
// The following would be a VIOLATION of Component Property 2:
//
//   // BAD: This function has external linkage but is NOT in my_stack.h
//   // Clients cannot know this exists; it "leaks" from the component.
//   int helperFunctionNotInHeader() { return 42; }
//
// CORRECT approach: if a helper is only used internally, give it
// internal linkage using 'static' or an anonymous namespace:
// ============================================================

namespace {
// Internal helpers can go here — they have internal linkage
// and do NOT violate Component Property 2.
// (Empty here since Stack<T> is fully inline in the header)
}  // close anonymous namespace

// There is nothing else to define here for this template component.
// The my_stack library target still exists so that clients can link
// against it (future non-template additions would go here).
