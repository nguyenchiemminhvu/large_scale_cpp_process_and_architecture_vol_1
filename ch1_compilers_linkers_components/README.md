# Chapter 1: Compilers, Linkers, and Components

**Book Pages**: 123–268 | *Large-Scale C++, Volume I* by John Lakos

---

## Overview

Chapter 1 is the foundation of the entire methodology. It covers the C++ build process in depth — from source files through object files to executables — and uses this understanding to derive the formal definition of a **component**: the atomic unit of physical design.

Understanding what the compiler and linker actually do is essential because physical design decisions are ultimately constrained by these tools.

---

## 1.1 Knowledge Is Power: The Devil Is in the Details

Lakos opens with a deceptively simple "Hello, World!" program and uses it to illuminate what actually happens when C++ code is compiled. Most developers have an incomplete mental model of the build process; this chapter fills in the gaps.

### 1.1.1 "Hello World!" — What Actually Happens

```cpp
// hello.cpp
#include <iostream>
int main() {
    std::cout << "Hello, World!\n";
    return 0;
}
```

Steps:
1. **Preprocessor**: Expands `#include <iostream>`, expands macros → produces a single *translation unit*
2. **Compiler**: Translates the translation unit to an *object file* (`.o`)
3. **Linker**: Resolves symbols (like `std::cout`) from library archives → produces an executable

### 1.1.3 The Role of Header Files

Header files do NOT contain machine code. They contain **declarations** — promises to the compiler that something exists somewhere. The linker resolves these promises at link time.

---

## 1.2 Compiling and Linking C++

### 1.2.1–1.2.2 The Build Process

```
source.cpp ─→ [preprocessor] ─→ translation unit ─→ [compiler] ─→ source.o
                                                                       ↓
                                                                  [linker]
                                                                       ↓
                                              other.o ─────────────→ executable
                                              libfoo.a ──────────────↗
```

### Object File Sections

An object file (`.o`) contains:
| Section | Contents |
|---|---|
| `.text` | Machine code (function bodies) |
| `.data` | Initialized global/static data |
| `.bss` | Uninitialized global/static data |
| Symbol table | Names of defined and referenced symbols |

### 1.2.4 Library Archives (`.a` files)

A static library is an **archive** of object files. The linker searches it for symbols that are undefined in the objects already processed. **Key insight**: the linker pulls in only the `.o` files it needs — but it pulls in entire `.o` files, not individual functions.

This is why component granularity matters: if you put 50 unrelated functions in one `.cpp`, using any one of them forces the linker to include all 50.

### 1.2.7–1.2.8 Shared Libraries

Shared libraries (`.so`, `.dll`) defer symbol resolution to load time. They have different trade-offs:
- **Smaller executable** (code shared among processes)
- **ABI stability required** (changing a shared library's binary interface breaks all users)
- **Dynamic linking overhead**

---

## 1.3 Declarations, Definitions, and Linkage

### Declarations vs. Definitions

```cpp
// Declaration: tells the compiler a symbol exists (no storage allocated)
extern int x;           // declared, not defined
class Foo;              // declared (forward declaration)
void bar(int);          // declared

// Definition: allocates storage / provides implementation
int x = 42;             // defined
class Foo { int d_x; }; // defined
void bar(int n) { ... } // defined
```

**Golden rule**: A declaration may appear multiple times in a program; a definition may appear only once (the One Definition Rule, ODR).

### Linkage Types

| Type | Visibility | Example |
|---|---|---|
| **External linkage** | Visible across translation units | `void foo();` (in header), class members |
| **Internal linkage** | Visible only within one `.cpp` | `static void foo()`, `namespace { void foo(); }` |
| **No linkage** | Local only | Local variables, local types |

### 1.3.8 Bindage (External vs. Internal)

Lakos uses **bindage** to describe how a name in a header file is bound to its definition:
- **External bindage**: The name is declared in the header and defined in the `.cpp` — standard for functions and class member functions.
- **Internal bindage** (antipattern in headers): Defining a function body in a header with internal linkage means each translation unit that includes it gets its **own copy**, which wastes code space and can cause subtle bugs with non-local statics.

---

## 1.4 Header Files

A header file's primary purpose: **declare the interface of a component** so that clients can use it without seeing the implementation.

### What Should Be in a Header File

| Should include | Should NOT include |
|---|---|
| Class definitions | Function bodies (unless `inline` or template) |
| `inline` function definitions | `using namespace` directives |
| Template definitions (full body required) | Global variable definitions |
| Forward declarations | `#include` of headers not needed by the interface |
| `typedef` / `using` aliases | Platform-specific code (usually) |

### 1.4.x Private Header Files (Antipattern)

Some methodologies use "private" header files (`.h` files not installed publicly) to share implementation details between `.cpp` files within a component. Lakos discourages this for most cases — it complicates the build and obscures dependencies.

---

## 1.5 Include Directives and Include Guards

### Include Guards (Internal)

Every header file must have an include guard to prevent multiple inclusions within a single translation unit:

```cpp
// my_widget.h
#ifndef INCLUDED_MY_WIDGET
#define INCLUDED_MY_WIDGET

// ... header content ...

#endif  // INCLUDED_MY_WIDGET
```

**Naming convention**: `INCLUDED_<COMPONENT_NAME>` in all uppercase, matching the component's filename.

### External Include Guards (Performance Optimization)

In very large codebases, even parsing a `#ifndef` guard takes time. Bloomberg's style uses **redundant external guards** in `.cpp` files:

```cpp
// my_widget.cpp
#ifndef INCLUDED_MY_WIDGET
#include <my_widget.h>
#endif
```

This prevents the preprocessor from even opening a header file that's already been included. At very large scale (thousands of headers), this can significantly reduce build times.

---

## 1.6 From `.h`/`.cpp` Pairs to Components

A **component** is a `.h`/`.cpp` pair satisfying four properties:

### Component Property 1: The `.h` Is Self-Sufficient
The header file must be compilable on its own:

```cpp
// BAD: foo.h includes a type 'Bar' without including bar.h
// Clients who include foo.h FIRST will get a compiler error.

// GOOD: foo.h includes everything it needs
#ifndef INCLUDED_FOO
#define INCLUDED_FOO
#include <bar.h>  // Bar is used in foo.h's interface

class Foo {
    Bar d_bar;  // Bar must be fully defined here
};
#endif
```

**Design Rule**: The component's `.h` file must be the first `#include` in its own `.cpp` file. This tests Property 1 at every compilation.

### Component Property 2: No Free Symbols Leak Out

The `.cpp` file must not **export symbols** that are not declared in the `.h` file. In other words, the `.cpp` must not define things at external linkage that are not in the header.

**Why**: If a `.cpp` defines a symbol `foo` without declaring it in the header, another `.cpp` might accidentally define the same `foo`, causing a linker error — or worse, silently picking up the wrong one.

### Component Property 3: Clients Include Only the Header

Clients of a component use **only** the header, never the `.cpp`. This means:
- All types used in the interface must be fully declared in the header
- Forward declarations in the header are fine for pointer/reference members

### Component Property 4: Dependency Is `.h`-Only

The actual dependency graph of components is determined by `#include` relationships between **headers** only. The `.cpp` files' `#include` directives may add to a component's implementation dependencies but do NOT affect the component's **interface dependencies** as seen by clients.

**Corollary**: If `my_foo.h` includes `my_bar.h`, then `my_foo` depends on `my_bar`. If `my_foo.cpp` includes `my_baz.h` but `my_foo.h` does not, then clients of `my_foo` do NOT depend on `my_baz`.

---

## 1.7 Notation and Terminology

Lakos defines precise notation for component relationships:

| Relationship | Symbol | Meaning |
|---|---|---|
| **Is-A** | `→` (filled) | Public inheritance |
| **Uses-In-Interface** | `→` (open) | Used in `.h` by value, pointer, or reference |
| **Uses-In-Implementation** | `···→` | Used in `.cpp` only |
| **Uses-In-Name-Only** | `- - →` | Only a forward declaration needed |

### Is-A vs. Uses-In-Interface

```cpp
// Uses-In-Interface: Foo "uses" Bar in its interface
class Foo {
    Bar d_bar;           // Bar used by value → strong dependency
    Bar* d_barPtr;       // Bar used by pointer → only needs forward decl
    void f(const Bar&);  // Bar used by ref → only needs forward decl
};

// Is-A: Derived "is a" Base (public inheritance)
class Derived : public Base { };
```

---

## 1.8 The Depends-On Relation

A component `Y` **Depends-On** component `X` if:
- `Y`'s `.h` file includes `X`'s `.h` file (direct header dependency), **OR**
- `Y`'s `.cpp` file includes `X`'s `.h` file (implementation dependency)

Dependencies are **transitive**: if `C` Depends-On `B` and `B` Depends-On `A`, then `C` Depends-On `A`.

---

## 1.9 Implied Dependency

If `A` Depends-On `B` and `B`'s header includes `C`'s header, then any client of `A` **transitively includes** `C`'s header — even if `A`'s author never intended this.

**Problem**: Implied dependencies create invisible coupling. Removing `B`'s dependency on `C` can break `A`'s clients who were accidentally relying on `C` being included.

**Solution**: Each component should only rely on what it directly `#include`s — never on transitive includes.

---

## 1.10 Level Numbers

Every component can be assigned a **level number** that describes its position in the dependency hierarchy:

- **Level 1**: Depends on nothing in the codebase (only on external/platform libraries)
- **Level N**: Depends on at least one Level N-1 component, and nothing above level N-1

```
Level 1: math_util, string_util
Level 2: date         (uses math_util)
Level 3: calendar     (uses date, string_util)
Level 4: calendar_cache (uses calendar)
```

**Key property**: If every component has a valid level number, the dependency graph is acyclic (a DAG). If you cannot assign level numbers, you have cycles.

---

## 1.11 Extracting Actual Dependencies

### Component Property 4 (Formal Statement)

The **actual** dependency of a component is completely determined by which header files its header `#include`s, plus which additional headers its `.cpp` `#include`s. There is no "hidden" dependency — everything is visible in `#include` directives.

This allows automated tools to extract the exact dependency graph from source code without needing to run the code.

---

## 1.12 Summary of Component Rules (Quick Reference)

```
┌────────────────────────────────────────────────────────────────┐
│                    COMPONENT RULES                              │
├────────────────────────────────────────────────────────────────┤
│ CP1: .h is self-sufficient (compilable standalone)             │
│ CP2: .cpp does not export symbols absent from .h               │
│ CP3: Clients #include only the .h, never the .cpp              │
│ CP4: Depends-On is determined by #include chains               │
│                                                                 │
│ Design Rules:                                                   │
│  - .h must be the FIRST #include in its own .cpp               │
│  - Every component must have include guards                     │
│  - No using directives/declarations in headers (outside funcs) │
│  - Dependencies must be acyclic                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## Code Examples

| File | Demonstrates |
|---|---|
| [src/my_stack.h](src/my_stack.h) | Component Property 1: self-sufficient header |
| [src/my_stack.cpp](src/my_stack.cpp) | Component Property 2: no leaking symbols |
| [src/my_stack.t.cpp](src/my_stack.t.cpp) | Test driver pattern (hierarchical testability) |
| [src/linkage_demo.h](src/linkage_demo.h) | External vs internal linkage |
| [src/linkage_demo.cpp](src/linkage_demo.cpp) | Bindage examples |
| [src/include_guard_demo.h](src/include_guard_demo.h) | Proper include guard pattern |
| [src/component_demo.cpp](src/component_demo.cpp) | Full demo showing all 4 component properties |
