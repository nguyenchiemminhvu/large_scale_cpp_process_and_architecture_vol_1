# Chapter 2: Packaging and Design Rules

**Book Pages**: 269–494 | *Large-Scale C++, Volume I* by John Lakos

---

## Overview

Chapter 2 defines the **packaging hierarchy** that sits above the individual component: packages and package groups. It also establishes the formal design rules that govern naming, dependencies, and physical organization.

The central insight: **individual components are too fine-grained to manage at enterprise scale**. We need higher-level aggregates with their own naming conventions and dependency rules.

---

## 2.1 The Big Picture

The enterprise software ecosystem divides into:

```
┌─────────────────────────────────────────┐
│         Enterprise Software Ecosystem   │
├───────────────┬─────────────────────────┤
│ Application   │    Library Software     │
│  Software     │  ┌──────────────────┐   │
│               │  │  Package Groups  │   │
│               │  │  ┌────────────┐  │   │
│               │  │  │  Packages  │  │   │
│               │  │  │ ┌────────┐ │  │   │
│               │  │  │ │Comps   │ │  │   │
│               │  │  │ └────────┘ │  │   │
│               │  │  └────────────┘  │   │
│               │  └──────────────────┘   │
│               │  Third-Party / Legacy   │
└───────────────┴─────────────────────────┘
```

---

## 2.2 Physical Aggregation

### Three Levels of Aggregation

| Level | Name | Typical Size | Description |
|---|---|---|---|
| **Level I** | Component | ~100–1000 LOC | A single `.h`/`.cpp` pair |
| **Level II** | Package | ~10–20 components | A directory of related components |
| **Level III** | Package Group (UOR) | ~10–30 packages | The deployable unit |

### Unit of Release (UOR)

A **Unit of Release (UOR)** is the outermost physical aggregate that is deployed atomically. Clients either take the entire UOR or none of it. This is typically a static library (`lib<name>.a`) or shared library (`lib<name>.so`).

### Balance Principle

Peer entities within an aggregate should have **comparable complexity**. An unbalanced hierarchy (e.g., a package group with one package containing 80% of the components) is a design smell. It indicates that either:
1. The grouping is wrong and needs to be reorganized, or
2. Some components should be demoted to a lower-level package

### No Cyclic Dependencies

**Design Imperative**: Allowed dependencies among any physical aggregates (at any level) must be **acyclic**.

This is not negotiable. Cyclic dependencies:
- Cannot be tested independently
- Create circular build dependencies
- Make component-level reuse impossible
- Grow in scope over time (one cycle always becomes many)

---

## 2.3 Logical/Physical Coherence

**Key principle**: The logical and physical designs must be coherent — all logical entities advertised by a physical module must be *implemented* within that module.

### Incoherence Examples (Both Bad)

```
Scenario A: Cyclic but coherent (BAD — cycles prevent scaling)
  buyside  ←→  sellside   [bidirectional dependency]

Scenario B: Acyclic but incoherent (BAD — logical doesn't match physical)
  buyside_lib: implements parts of both 'buyside' AND 'sellside'
  sellside_lib: implements the rest of both

Scenario C: Acyclic AND coherent (GOOD)
  shared_lib:     common abstractions used by both
  buyside_lib:    implements buyside (uses shared_lib)
  sellside_lib:   implements sellside (uses shared_lib)
```

---

## 2.4 Logical and Physical Name Cohesion

### The Naming Hierarchy

Every architecturally significant entity has **three names**:
1. The **logical entity name** (e.g., `Date`)
2. The **component/header name** (e.g., `bdlt_date.h`)
3. The **UOR/package group name** (e.g., `bdlt`)

These three names must be **cohesive** — a developer reading `bdlt::Date` should immediately know:
- The class is `Date` in namespace `bdlt`
- It's declared in `bdlt_date.h`
- It's implemented in the `bdlt` package group

### Design Rules for Naming

```
Package group "bdlt" (Bloomberg Date/Time Library):
  Package "bdlt":
    Component bdlt_date → bdlt_date.h + bdlt_date.cpp
      Declares class bdlt::Date

  Package "bdlb":
    Component bdlb_string → bdlb_string.h + bdlb_string.cpp
      Declares struct bdlb::StringUtil
```

| Design Rule | Description |
|---|---|
| DR-2.1 | Component files must share the same root name |
| DR-2.2 | Every library component filename must be enterprise-wide unique |
| DR-2.3 | Every component must reside within a package |
| DR-2.4 | Component name = `<package_name>_<local_name>` (all lowercase) |
| DR-2.5 | Package name = `<group_name>` + 1-3 chars (e.g., `bdlt` → `bdlt_date`) |

### Why Opaque Package Prefixes Are Fine

An important insight: package prefixes like `bdlt` need NOT be descriptive. They serve as:
- Globally unique namespace tags (avoids symbol collisions)
- Visual cues linking an entity to its physical location
- Mnemonics that grow meaningful through familiarity

`bdlt` initially means nothing; after months of use, it becomes immediately recognizable as "Bloomberg Date/Time."

---

## 2.4.12 No `using` Directives in Headers

**Design Rule**: Neither `using` directives nor `using` declarations are permitted outside function scope within a component.

```cpp
// VIOLATION — do NOT do this in a header:
using namespace std;       // Pollutes every TU that includes this header
using std::vector;         // Same problem — exported into every includer

// OK — within function scope only:
void foo() {
    using std::cout;       // Only affects this function's scope
    cout << "hello\n";
}
```

**Why**: At large scale, a `using namespace X` in a header effectively merges namespace `X` into every file that includes this header (transitively). This makes it **impossible to determine which namespace a name comes from** by reading any single source file.

---

## 2.5 Component Source Code Organization

### Standard Layout of a Component

```
package/
├── pkg_mytype.h        ← Interface declaration
├── pkg_mytype.cpp      ← Implementation
└── pkg_mytype.t.cpp    ← Test driver (optional, not shipped)
```

### Standard File Header Structure

```cpp
// pkg_mytype.h                                                     -*-C++-*-
//
// PURPOSE: ...
// CLASSES:
//   pkg::MyType - ...
// DESCRIPTION: ...

#ifndef INCLUDED_PKG_MYTYPE
#define INCLUDED_PKG_MYTYPE

// Standard library includes first
#include <string>

// Then package-group internal includes
// #include <other_pkg_component.h>

namespace pkg {

class MyType { /* ... */ };

}  // close namespace pkg

#endif  // INCLUDED_PKG_MYTYPE
```

---

## 2.8–2.10 Packages

### Package Structure

A **package** is a directory containing a set of components with:
- A common (short) prefix in all component names
- A shared namespace (the package name is the namespace)
- A declared set of allowed dependencies (what other packages it may use)
- A package-level documentation file

```
pkg/                    ← Package directory
├── package/            ← (some organizations use this)
│   ├── package.mem     ← manifest: lists components
│   └── package.dep     ← allowed dependencies
├── pkg_date.h
├── pkg_date.cpp
├── pkg_date.t.cpp
├── pkg_dateutil.h
└── pkg_dateutil.cpp
```

### Package Group Structure

A **package group** contains a set of packages that are released together as a UOR:

```
bdlt/                   ← Package group (UOR)
├── group/
│   ├── bdlt.mem        ← lists packages
│   └── bdlt.dep        ← allowed external dependencies
├── bdlt/               ← Package "bdlt"
│   ├── bdlt_date.h
│   └── bdlt_date.cpp
├── bdlb/               ← Package "bdlb"
│   ├── bdlb_string.h
│   └── bdlb_string.cpp
└── ...
```

---

## 2.14 The Hierarchical Testability Requirement

This section is foundational to the entire methodology. It defines what it means for software to be **testable in a hierarchical, fine-grained manner**.

### Key Requirements

1. **Every component must be independently unit-testable** using only its allowed dependencies.
2. **Test drivers** must not introduce new dependencies beyond what the component itself depends on.
3. **Test drivers** are named `<component>.t.cpp` and live in the same directory.
4. **Test driver invocation** must be uniform: run the executable, check exit code (0 = pass).

### Why Hierarchical Testability Requires Acyclic Dependencies

If component `A` depends on `B` and `B` depends on `A` (cycle), then:
- You cannot test `A` without `B`
- You cannot test `B` without `A`
- Fine-grained unit testing becomes impossible
- You are forced into integration testing at the very first level

This is why **no cyclic dependencies** is a design *imperative*, not just a preference.

### Test Driver Example

```
Test Level 1: test math_util → no dependencies to link
Test Level 2: test date       → link only math_util
Test Level 3: test calendar   → link math_util + date
```

Each test links only the components it needs — nothing more. This enables:
- Fast, focused tests
- Clear failure attribution
- Parallel test execution
- Incremental testing (only re-test what changed)

---

## 2.15 From Development to Deployment

### Unique Names

**Design Rule**: Every `.h` file and `.o` file must have a globally unique name throughout the enterprise.

This allows:
- Flexible deployment (install all headers in `/usr/include` without collisions)
- Tool automation (grep for a header name finds exactly one component)
- Archive organization (library archives with unique `.o` files are easier to use)

### Flexible Deployment

Package groups can be deployed as:
- Static libraries (`.a`) — fast link, larger executables
- Shared libraries (`.so`) — slower link, smaller executables, runtime binding
- Source (component-level) — maximum flexibility for embedded/constrained environments

The physical design must support all three deployment modes.

---

## 2.16 Metadata

For package groups and packages, explicit metadata must be maintained:

| Metadata Type | Contents |
|---|---|
| **Membership metadata** | Which components belong to this package |
| **Dependency metadata** | Which external packages/groups this is allowed to depend on |
| **Build requirements** | Compiler flags, platform requirements |
| **Enterprise policy** | Compliance, licensing, ownership |

This metadata is what enables automated **dependency verification** — checking that a developer hasn't introduced a dependency that violates the allowed dependency graph.

---

## Code Examples

| Directory | Demonstrates |
|---|---|
| [src/pkg/](src/pkg/) | A complete example package `pkg` with proper naming |
| [src/pkg/pkg_date.h](src/pkg/pkg_date.h) | Cohesive naming: class `pkg::Date`, file `pkg_date.h` |
| [src/pkg/pkg_dateutil.h](src/pkg/pkg_dateutil.h) | Utility struct pattern for free functions |
| [src/design_rules_demo.cpp](src/design_rules_demo.cpp) | Demonstrates all naming and design rules |
