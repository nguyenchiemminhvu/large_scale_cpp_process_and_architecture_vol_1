# Large-Scale C++: Process and Architecture
### A Workshop Based on John Lakos's *Large-Scale C++, Volume I* (Pearson, 2020)

---

## Slide 1 — Title

# Large-Scale C++
## Process and Architecture

> *How to write C++ software that can grow, be tested, and be reused at scale.*

**Based on**: *Large-Scale C++, Volume I* by John Lakos  
**Audience**: C++ developers who want to understand physical design  
**Goal**: Learn the Bloomberg methodology for building maintainable, hierarchically reusable C++

---

## Slide 2 — The Real Problem

### What Goes Wrong in Large C++ Codebases?

- Change one header file → entire codebase rebuilds (10+ minutes)
- Want to write a unit test → must link in 30 unrelated libraries
- Try to reuse a "utility" → it drags along half the application
- Two components form a cycle → neither can be tested alone
- Refactoring feels like surgery → touching anything breaks everything

**These are not compiler problems. They are physical design problems.**

> Most developers learn logical design (classes, patterns, algorithms).  
> Almost nobody teaches **physical design** (files, headers, dependencies).

---

## Slide 3 — What Is Physical Design?

### Two Kinds of Design

| Logical Design | Physical Design |
|---|---|
| What classes should exist? | Which `.h`/`.cpp` files should exist? |
| What are their relationships? | What do those files `#include`? |
| Inheritance, composition, patterns | Dependency graphs, link-time structure |
| Expresses *semantics* | Governs *compilation, testing, deployment* |

**Physical design determines:**
1. How long it takes to compile
2. Whether components can be tested independently
3. How much code you must link just to use one function
4. Whether you can reuse a library without dragging along everything else

> *"Physical design is not just about files and directories. It is about controlling the envelope of dependencies."* — Lakos

---

## Slide 4 — Application vs. Library Software

### Different Goals, Different Rules

| | Application Software | Library Software |
|---|---|---|
| **Consumers** | End users | Other developers |
| **Lifecycle** | Ship and patch | Long-term evolution |
| **Test surface** | System/integration tests | Fine-grained unit tests |
| **Deployment** | Monolithic binary | Independently linkable UOR |
| **Coupling tolerance** | High (internal is fine) | Low (must be reusable) |
| **Discipline required** | Moderate | High |

**Key insight:**  
Even "application" code contains layers that ARE library software.  
Your date utilities, event system, string helpers — these are libraries.  
Design them like libraries or pay the debt later.

---

## Slide 5 — Software Capital

### Well-Designed Software Is an Economic Asset

```
Software Capital:
  - Components are independently reusable ✓
  - Dependencies are minimal and acyclic ✓
  - Interfaces are stable ✓
  → Can be reused in the NEXT project, and the one after that
  → Grows in value as more things depend on it

Software Debt:
  - Everything depends on everything else ✗
  - Cannot change one thing without breaking ten others ✗
  - Unit testing requires the entire system linked ✗
  → Must be rewritten every project
  → Grows in cost as it ages
```

> The entire Lakos methodology is about accumulating **software capital**,  
> not software debt.

---

## Slide 6 — Collaborative vs. Reusable vs. Hierarchically Reusable

### Three Tiers of Software Reuse

**Collaborative software** — pieces that only work together. Like a cracked plate: fits perfectly, but you cannot take one piece out of the set.

**Reusable software** — a single component with a clean interface, usable in isolation.

**Hierarchically reusable software** — the highest form:

```
Application Layer          (uses all below)
    ↑
Calendar Subsystem         (uses Date + String)
    ↑
Date Library               (uses Integer Math)
    ↑
Integer Math Utilities     (uses nothing internal)
```

Each layer is independently testable and releasable.  
This is only achievable with **acyclic dependencies**.

---

## Slide 7 — Malleable vs. Stable Software

### Not All Software Ages the Same

**Malleable software** (applications):  
- Expected to change frequently with business needs  
- Can afford more coupling internally  
- Users are end users, not developers

**Stable software** (libraries):  
- Must evolve without breaking existing clients  
- Requires strict physical design discipline from day one  
- Users are other developers who build on top of it

**The trap:**  
Software that starts as "just an application" often gets used as a library.  
If it was designed carelessly, retrofitting good physical design is expensive.

> Design every reusable component as if it will be a library.  
> You don't know who will depend on it six months from now.

---

## Slide 8 — The Component: The Atomic Unit

### What Is a Component?

A **component** is a single `.h`/`.cpp` pair with the same base name.

```
my_stack.h    ←→    my_stack.cpp
   interface           implementation
```

- The `.h` file is the **contract** — what clients see and depend on
- The `.cpp` file is the **implementation** — clients never include this
- Every reusable piece of code lives in exactly one component

**Why this matters:**
- Automated dependency extraction works
- Fine-grained unit testing becomes possible
- Independent deployment is achievable

> The component is to C++ code what a brick is to a building.  
> You can reuse a brick anywhere. You cannot reuse half a wall.

---

## Slide 9 — How C++ Programs Are Built

### The Build Pipeline

```
my_stack.cpp
    │
    ↓ [preprocessor: expand #includes and macros]
    │
translation unit (.i)
    │
    ↓ [compiler: parse, optimize, generate machine code]
    │
object file (.o)   ← contains: .text, .data, .bss, symbol table
    │
    ↓ [linker: resolve symbols from .o files and .a archives]
    │
executable / shared library
```

**What happens when you `#include <iostream>`?**  
The preprocessor literally **pastes** the entire contents of `<iostream>` into your translation unit — before the compiler ever sees a single line of your code.

> Every `#include` you add to a header is a **compilation tax** paid by every file that includes that header — and every file that includes those files.

---

## Slide 10 — Object Files and Library Archives

### What the Linker Does

An **object file** (`.o`) contains:
- Machine code (`.text` section)
- Initialized data (`.data` section)
- A symbol table: names of things defined here + names of things needed from elsewhere

A **static library** (`.a`) is just a collection of `.o` files bundled together.

**The linker's rule:**  
When it needs a symbol, it pulls in the **entire `.o` file** that defines it — not just the function you used.

```
// library.a contains: math.o  string.o  graphics.o  network.o
// You use one function from math.o
// → Linker pulls in ALL of math.o
// → But NOT string.o, graphics.o, or network.o (unless you use those too)
```

**Lesson:** Component granularity directly controls link-time footprint.  
50 unrelated functions in one `.cpp` = 50 functions forced on every user of any one.

---

## Slide 11 — Declarations vs. Definitions

### The One Definition Rule

```cpp
// Declaration — a PROMISE: "this thing exists somewhere"
extern int g_counter;           // declared
class Date;                     // forward declaration
void processEvent(int type);    // declared

// Definition — "HERE it actually is" (storage allocated, code generated)
int g_counter = 0;              // defined
class Date { int d_year, d_month, d_day; };  // defined
void processEvent(int type) { /* body */ }   // defined
```

**The One Definition Rule (ODR):**  
- A declaration may appear as many times as needed (in any `.h` file)
- A definition may appear **exactly once** across the whole program

**Linkage types:**
| Type | Scope | Example |
|---|---|---|
| External linkage | Entire program | Functions, global variables, classes |
| Internal linkage | One `.cpp` file only | `static` functions, `namespace { ... }` |
| No linkage | Local only | Local variables |

---

## Slide 12 — Internal vs. External Linkage in Practice

### Why Linkage Matters for Components

```cpp
// BAD: function defined in header without inline → multiple definitions!
// every .cpp that includes this gets its own copy → ODR violation
void helper() { /* ... */ }   // in a header ← NEVER do this

// GOOD: internal linkage for helpers inside .cpp files
namespace {
    void helperOnlyThisFile() { /* hidden from other TUs */ }
}

// GOOD: inline in headers (compiler may inline, but only ONE logical definition)
inline void smallHelper() { /* ... */ }

// GOOD: declare in .h, define in .cpp (standard pattern)
// foo.h: void publicFunction();
// foo.cpp: void publicFunction() { /* body */ }
```

**Anonymous namespace** = internal linkage = safe helper implementation.  
**Never define non-inline functions with external linkage in headers.**

---

## Slide 13 — Header Files: What Goes Where

### The Contract vs. The Implementation

| Belongs in `.h` | Does NOT belong in `.h` |
|---|---|
| Class definitions | Non-inline function bodies |
| `inline` function definitions | `using namespace` directives |
| Template definitions (full body) | Global variable definitions |
| Forward declarations | Platform-specific conditionals (usually) |
| `typedef` / `using` aliases | `#include` of headers not needed by the interface |
| Function declarations | Implementation-only helper declarations |

**The key question:** *"Would a client need this to USE my component?"*  
If yes → it belongs in `.h`. If no → it belongs in `.cpp`.

> Moving things out of headers reduces the compilation tax on all clients.

---

## Slide 14 — Include Guards

### Preventing Multiple Inclusions

Every header file MUST have an include guard:

```cpp
// my_widget.h
#ifndef INCLUDED_MY_WIDGET        // ← INCLUDED_ prefix convention
#define INCLUDED_MY_WIDGET

class MyWidget { /* ... */ };

#endif  // INCLUDED_MY_WIDGET     // ← comment identifies the guard
```

**Naming convention:** `INCLUDED_<COMPONENT_NAME>` all uppercase.

**External include guards (performance optimization for large codebases):**

```cpp
// In my_widget.cpp — skip even opening the file if already included
#ifndef INCLUDED_MY_WIDGET
#include <my_widget.h>
#endif

#ifndef INCLUDED_STD_STRING
#include <string>
#endif
```

At thousands of headers, this can measurably reduce build times by avoiding file I/O for already-processed headers.

---

## Slide 15 — The Four Component Properties

### What Makes a Valid Component?

**CP1: The `.h` file is self-sufficient**  
The header must compile correctly on its own, without any prior `#include`s.

```cpp
// GOOD: my_foo.h includes everything IT needs
#ifndef INCLUDED_MY_FOO
#define INCLUDED_MY_FOO
#include <my_bar.h>   // if Bar is used in Foo's interface, include it here

class Foo { Bar d_bar; };  // works because bar.h is included above
#endif
```

**CP2: `.cpp` begins with `#include` of its own `.h` (enforces CP1)**  
```cpp
// my_foo.cpp — FIRST include is always the component's own header
#include <my_foo.h>   // ← this TESTS that the header is self-sufficient
#include <stdexcept>  // other deps come after
```

**CP3: Clients never `#include` the `.cpp` file**  
Only the `.h` is part of the public interface.

**CP4: Dependencies are fully determined by `#include` chains**  
No hidden dependencies — everything visible in `#include` directives.

---

## Slide 16 — Logical Relationships and Notation

### How Components Relate to Each Other

Lakos defines precise notation for component relationships:

| Relationship | Meaning | Physical Effect |
|---|---|---|
| **Is-A** | Public inheritance | Strong dependency on base class header |
| **Uses-In-Interface** | Used in `.h` by value, pointer, or reference | Must `#include` (or forward-declare) |
| **Uses-In-Implementation** | Used only in `.cpp` | `.cpp`-only include, clients unaffected |
| **Uses-In-Name-Only** | Only a forward declaration needed | Can forward-declare instead of full include |

```cpp
class Foo {
    Bar d_bar;         // Uses-In-Interface by VALUE → must include bar.h
    Baz* d_bazPtr;     // Uses-In-Interface by POINTER → can forward-declare
    void f(const Qux&); // Uses-In-Interface by REF → can forward-declare
};
```

**Uses-In-Name-Only is key:** When you only store a pointer or reference, you don't need the full class definition — just a forward declaration `class Bar;`. This breaks compile-time coupling!

---

## Slide 17 — Level Numbers

### Assigning Positions in the Dependency Hierarchy

Every component can be assigned a **level number:**

- **Level 1**: Depends on nothing internal (only standard library or platform)
- **Level N**: Depends on at least one Level N-1 component, nothing above N-1

```
Level 1: math_util, string_util, memory_util
Level 2: date            (uses math_util)
Level 3: calendar        (uses date, string_util)
Level 4: calendar_cache  (uses calendar)
Level 5: business_day    (uses calendar_cache, date)
```

**Two vital properties of level numbers:**

1. If you CAN assign level numbers → dependency graph is acyclic (a DAG) ✓
2. If you CANNOT → you have a cycle. Find it and fix it. ✗

> **Level numbers make cycles visible.**  
> If component A depends on B and B depends on A,  
> neither A nor B can be assigned a valid level number.

---

## Slide 18 — The Depends-On Relation

### What Does "Depends On" Really Mean?

Component `Y` **Depends-On** component `X` if:
- `Y`'s `.h` file `#include`s `X`'s `.h` file (**interface dependency**)  
- `Y`'s `.cpp` file `#include`s `X`'s `.h` file (**implementation dependency**)

Dependencies are **transitive**: If C depends-on B and B depends-on A, then C depends-on A.

**Implied dependencies — the hidden danger:**

```
A.h includes B.h
B.h includes C.h
→ Any file including A.h also transitively includes C.h

If B later removes its #include of C.h:
→ Files relying on C.h being available via A.h → BREAKS
```

**Rule:** Never rely on transitive includes. Always `#include` what you directly use.  
Transitive inclusions are an accident, not a contract.

---

## Slide 19 — The Physical Aggregation Hierarchy

### Three Levels Above the Component

```
┌─────────────────────────────────────────────────────────────┐
│  Package Group "bdl" (Unit of Release)                      │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  Package "bdlt"  (Bloomberg Date/Time)              │    │
│  │  ┌──────────────┐  ┌──────────────┐                 │    │
│  │  │ bdlt_date.h  │  │ bdlt_cal.h   │  ...            │    │
│  │  │ bdlt_date.cpp│  │ bdlt_cal.cpp │                 │    │
│  │  └──────────────┘  └──────────────┘                 │    │
│  └─────────────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  Package "bdlb"  (Bloomberg Basic)                  │    │
│  │  ┌──────────────┐  ┌──────────────┐                 │    │
│  │  │ bdlb_str.h   │  │ bdlb_num.h   │  ...            │    │
│  │  └──────────────┘  └──────────────┘                 │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

| Level | Name | Typical Scale | Released As |
|---|---|---|---|
| I | Component | 1 `.h` + `.cpp` | — |
| II | Package | 10–20 components | — |
| III | Package Group | 10–30 packages | `.a` / `.so` |

---

## Slide 20 — Unit of Release (UOR)

### What Gets Deployed

A **Unit of Release (UOR)** is the outermost physical aggregate that is deployed atomically.

**Clients either take the whole UOR — or none of it.**

```
libBdl.a   ← clients link against this
  contains: bdlt_date.o  bdlt_calendar.o  bdlb_string.o  ...
```

**No Cyclic Dependencies at ANY level:**
- No cycles between components
- No cycles between packages
- No cycles between package groups

This rule is absolute. Cyclic dependencies cannot be levelized,  
cannot be tested hierarchically, and prevent flexible deployment.

**Three-level balance:**  
The aggregation hierarchy should be roughly balanced.  
A package group with one giant package and many tiny ones is a design smell.

---

## Slide 21 — Name Cohesion

### Three-Way Naming Coherence

Every entity must have **consistent names** across all three levels:

```
Namespace:    bdlt
Component:    bdlt_date      (= package_name + "_" + local_name)
Header:       bdlt_date.h
Class:        bdlt::Date
```

**What this gives you:**  
When you see `bdlt::Date d` in code you've never read before, you know **immediately**:
- The class is `Date` in namespace `bdlt`
- It's declared in `bdlt_date.h`
- It's in the `bdlt` package of the `bdl` package group
- No searching, no guessing

> "The point of use should be sufficient to identify location."  
> — Lakos Design Rule 2.4.6

**Why opaque prefixes are fine:**  
`bdlt` doesn't mean anything at first glance. But after a few weeks of working in the codebase, it becomes immediately recognizable as the date/time package. Familiarity makes mnemonics meaningful.

---

## Slide 22 — Design Rules for Naming

### Key Naming Conventions

```
// Package: pkg
// Component: pkg_date

// Header file
pkg_date.h

// Class name
class pkg::Date { ... };

// Data member naming
class Date {
    int d_year;   // d_ prefix for instance data members
    int d_month;
    int d_day;
    static int s_instanceCount;  // s_ prefix for static class members
};
```

**Global uniqueness requirement:**  
Every `.h` file name must be unique across the **entire enterprise**.  
This enables:
- Installing all headers in one flat directory
- Automated dependency tools working without path context
- Clean refactoring without namespace collisions

---

## Slide 23 — Design Rules for `using` Directives

### The Most Important Rule You Probably Break

```cpp
// ❌ VIOLATION: Never in header files
using namespace std;           // pollutes EVERY file that includes this
using std::vector;             // same problem — leaks into every client

// ✅ OK: Inside function bodies only
void foo() {
    using std::cout;           // scoped — doesn't affect anything outside
    cout << "hello\n";
}
```

**Why this matters at scale:**  
A `using namespace X` in a header transitively imports namespace `X` into every file that includes your header, and every file that includes THOSE files.

In a large codebase, this means:
- Name collisions become unpredictable
- You cannot determine which namespace a name comes from by reading any single file
- Removing the `using` breaks all downstream code

**Also: Use `<iosfwd>` not `<iostream>` in headers when you only need `ostream&`:**  
```cpp
// ✅ In headers: forward-declare the stream type
#include <iosfwd>
std::ostream& operator<<(std::ostream& os, const Date& d);
// ❌ Don't include <iostream> just for a declaration
```

---

## Slide 24 — Component Source Code Organization

### Standard Layout

```cpp
// pkg_mytype.h                                              -*-C++-*-
//
// CLASSES:
//   pkg::MyType - a brief description
//
#ifndef INCLUDED_PKG_MYTYPE
#define INCLUDED_PKG_MYTYPE

#ifndef INCLUDED_STD_STRING
#include <string>
#endif

namespace pkg {

class MyType {
    std::string d_name;   // d_ prefix for data members
  public:
    explicit MyType(const std::string& name);
    const std::string& name() const;
    // ... (only primitive operations here)
};

}  // close namespace pkg

#endif  // INCLUDED_PKG_MYTYPE
```

```cpp
// pkg_mytype.cpp
#ifndef INCLUDED_PKG_MYTYPE        // external include guard
#include <pkg_mytype.h>            // ← ALWAYS first
#endif

namespace pkg {
MyType::MyType(const std::string& name) : d_name(name) {}
const std::string& MyType::name() const { return d_name; }
}
```

---

## Slide 25 — Packages: Structure and Rules

### What a Package Looks Like

```
bdlt/                          ← Package directory
├── bdlt_date.h                ← Component header
├── bdlt_date.cpp              ← Component implementation
├── bdlt_date.t.cpp            ← Test driver (not shipped)
├── bdlt_calendar.h
├── bdlt_calendar.cpp
├── bdlt_calendar.t.cpp
└── package/
    ├── bdlt.mem               ← Membership: list of components
    └── bdlt.dep               ← Dependency: allowed external packages
```

**Package rules:**
- All components share the package name as a prefix
- All components live in the same namespace
- The package declares its allowed dependencies explicitly
- No cyclic dependencies between packages

**Package charter:** Every package has documented purpose — what it does and what it doesn't do. This prevents "feature creep" that leads to an ever-growing monolithic package.

---

## Slide 26 — Package Groups: The Unit of Release

### How Packages Compose into Deployable Units

```
bdl/                           ← Package group (Unit of Release)
├── group/
│   ├── bdl.mem                ← Lists packages in this group
│   └── bdl.dep                ← Allowed external package groups
├── bdlt/                      ← Package "date/time"
│   ├── bdlt_date.h/cpp
│   └── bdlt_calendar.h/cpp
├── bdlb/                      ← Package "basic"
│   ├── bdlb_string.h/cpp
│   └── bdlb_numericparseutil.h/cpp
└── bdls/                      ← Package "system"
    ├── bdls_filesystemutil.h/cpp
    └── bdls_processutil.h/cpp
```

**Deployment:** Clients link against `libbdl.a` — the whole UOR.  
The internal package structure is an **organizational and testing detail**.  
Clients do not need to know which package a component lives in.

---

## Slide 27 — Hierarchical Testability Requirement

### The Most Important Consequence of Acyclic Design

**The Requirement:**  
Every component must be independently unit-testable using only components at strictly lower levels.

```
Test for Level 1 math_util:   links nothing internal
Test for Level 2 date:        links math_util only
Test for Level 3 calendar:    links math_util + date
Test for Level 4 cal_cache:   links math_util + date + calendar
```

**What you lose when you have cycles:**

```
If A depends on B AND B depends on A:
  → Cannot test A without B
  → Cannot test B without A
  → "Unit tests" become integration tests
  → Tests get slow, flaky, hard to diagnose
  → Developers stop writing them
```

**Acyclic dependencies are not optional preference — they are a testing requirement.**

> Fine-grained unit testing is ONLY possible in a codebase with acyclic dependencies.

---

## Slide 28 — Thinking Physically

### Asking the Right Question

When adding a new piece of code, the question is not just  
*"What class should I create?"*

It is also:  
*"Where should this code physically live?"*

**The dependency cone:**  
When you write `#include <X.h>`, you pull in everything that `X.h` depends on.  
Everything. Transitively.

```
#include <calendar.h>
  → includes date.h
    → includes math_util.h
      → includes <cstdint>
```

You are now bound to ALL of these at compile time.

**Good citizenship:**
- Find what already exists before creating something new
- Place new code at the lowest level that satisfies its requirements
- Never reach upward — lower levels must not depend on higher levels

---

## Slide 29 — Primitive vs. Non-Primitive Functionality

### A Critical Design Decision

A **primitive** operation is one that:
- NEEDS access to private state to be implemented efficiently, OR
- Is logically central to what the abstraction represents

A **non-primitive** operation is one that:
- COULD be implemented using only the public (primitive) interface
- Does not need private access

**Example: `Polygon` class**

```cpp
class Polygon {
  public:
    void addVertex(double x, double y);  // PRIMITIVE: accesses d_vertices
    double area() const;                 // PRIMITIVE: needs vertex data
    int vertexCount() const;             // PRIMITIVE: accesses d_vertices

    // ❌ These are NON-PRIMITIVE — they can be built from the primitives above:
    bool isConvex() const;     // computed from vertex angles
    void rotate(double angle); // uses addVertex + area
    bool isRegular() const;    // uses area + perimeter
};
```

**Non-primitives should go in a utility struct:**
```cpp
struct PolygonUtil {
    static bool isConvex(const Polygon& p);
    static void rotate(Polygon* p, double angle);
};
```

---

## Slide 30 — Why Separate Non-Primitive Functionality?

### The Benefits Are Compounding

1. **Smaller compile-time footprint:**  
   Clients who only need `area()` don't compile `isConvex()`'s dependencies

2. **Simpler, more stable core component:**  
   Fewer reasons for `Polygon` to change → fewer rebuilds for its clients

3. **Independent extensibility:**  
   Add new utilities without touching the core component (Open/Closed Principle)

4. **Easier, faster unit testing:**  
   Test `Polygon` with 20 test cases.  
   Test `PolygonUtil` separately with its own 20 test cases.  
   No entanglement.

5. **Explicit link-time footprint:**  
   Users who don't need `PolygonUtil` don't link it.  
   The linker only pulls in `.o` files you reference.

> Putting everything in one class feels convenient.  
> It becomes a maintenance nightmare at scale.

---

## Slide 31 — When to Colocate Classes in One Component

### Four Criteria for Colocation

By default, **keep classes in separate components**.  
Colocation (two classes in one `.h`/`.cpp` pair) requires justification.

| Criterion | Example | Why Colocate |
|---|---|---|
| **Friendship** | `NodeIterator` + `Node` | Iterator needs private access to Node |
| **Cyclic dependency** | `Event` + `EventQueue` | They mutually reference each other |
| **Single solution** | `DayCount` + concrete implementations | Logically inseparable unit |
| **Flea on elephant** | `StackIterator` + `Stack` | Iterator is tiny and tightly coupled |

**What NOT to use as a colocation criterion:**
- "They're related" — everything is related to something
- "They're small" — small classes deserve their own components
- "It's convenient" — convenience now = coupling forever

---

## Slide 32 — Cyclic Dependencies: Why They Are Fatal

### The Case Against Cycles

```
Component A includes Component B
Component B includes Component A
```

**Immediate consequences:**
1. Cannot compile A without B, cannot compile B without A → build fails
2. Cannot test A without linking B, cannot test B without linking A
3. Level numbers are undefined for both
4. Any change to A's header triggers B's recompilation (and vice versa)

**Long-term consequences at scale:**
- One cycle always becomes many (developers see it and copy the pattern)
- Integration testing replaces unit testing
- Rebuild times grow to minutes for every change
- Code cannot be extracted and reused in another project
- Refactoring becomes impossibly risky

> Cyclic dependencies are like cancer in a codebase.  
> Start small, grow, and eventually everything is connected to everything else.

---

## Slide 33 — Levelization Overview: 7 Techniques

### Your Toolbox for Breaking Cycles

When you encounter a cyclic dependency, use one of these:

| # | Technique | Best For |
|---|---|---|
| 1 | **Classic Levelization** | Bidirectional logical relationship — one side doesn't actually need the other |
| 2 | **Escalation** | Two components need to interact — move the interaction to a higher-level component |
| 3 | **Demotion** | Low-level capability trapped in a high-level component |
| 4 | **Opaque Pointers (PIMPL)** | Header includes too many/heavy dependencies |
| 5 | **Dumb Data** | Low-level type has logically upward-facing knowledge |
| 6 | **Callbacks** | Low-level component needs to notify high-level code |
| 7 | **Protocol Class** | Need runtime polymorphism without physical coupling |

Plus two supporting techniques:
- **Manager Class** — orchestrator at a higher level
- **Factoring** — extract shared code into a common lower-level component

---

## Slide 34 — Technique 1: Classic Levelization

### Cut the Reverse Dependency

**Scenario:** Manager and Employee seem to have a bidirectional relationship.

```
Before (cycle):
  Manager ←→ Employee     // Manager holds Employees, Employee knows its Manager

After (acyclic):
  Manager → Employee      // Manager holds Employees
                          // Employee knows NOTHING about Manager
```

**The key insight:**  
Many "bidirectional" logical relationships can be implemented with a **unidirectional** physical dependency.

Does `Employee` really need to know who its manager is at the physical level?  
- For HR operations — yes, probably (but that goes at a higher level)
- For the core Employee data — probably not

> Ask: "What does each side REALLY need from the other at THIS level?"  
> Often the answer reveals that one direction is optional.

---

## Slide 35 — Technique 2: Escalation

### Add a Third Component Above Both

**Scenario:** `EventQueue` processes `Event` objects, but `Event` needs to be put into queues.  
Making them mutually dependent creates a cycle.

```
Before (attempt):
  EventQueue ←→ Event

After (escalated):
  EventManager (Level 3) → EventQueue (Level 2)
  EventManager (Level 3) → Event      (Level 1)
  EventQueue   (Level 2) → Event      (Level 1)
  ← Event knows NOTHING about queues or managers
```

**`EventManager` at Level 3:**
- Creates Events
- Puts them in EventQueues
- Processes them
- Both Event and EventQueue remain clean, focused, low-level

> If two things need to talk, don't force them to talk directly.  
> Build a room above them that handles the conversation.

---

## Slide 36 — Technique 3: Demotion

### Move Low-Level Code Out of High-Level Components

**Scenario:** A "utility" module has grown to contain:
- String hashing (Level 1 capability)
- Date parsing (Level 3 capability)
- Report generation (Level 5 capability)

Components at Level 1 that need string hashing must link against Level 5! 

```
Before:
  all_utils (Level 5) → contains everything
  → Level 1 components link against Level 5 to get Level 1 functionality

After (demoted):
  str_hash (Level 1)  — just the hash function
  date_util (Level 3) — date parsing
  report (Level 5)    — report generation

  → Level 1 components link only str_hash (Level 1)
```

**This is how "big ball of mud" modules should be refactored.**  
Identify the lowest-level capability in the blob and demote it.

---

## Slide 37 — Technique 4: Opaque Pointers (PIMPL)

### Breaking Compile-Time Coupling

**Scenario:** `Device` stores a `HeavyDriver` member.  
Every client that includes `device.h` must also process `heavy_driver.h` and all its includes.

```cpp
// ❌ BEFORE — strong dependency
// device.h
#include <heavy_driver.h>   // every Device client pays this cost!

class Device {
    HeavyDriver d_driver;   // full type required
};
```

```cpp
// ✅ AFTER — opaque pointer
// device.h — only <memory> and <string> needed
class DeviceImpl;           // just a name — no include needed

class Device {
    std::unique_ptr<DeviceImpl> d_impl_p;
  public:
    Device();
    ~Device();              // must be defined in .cpp
    void read();
};
```

```cpp
// device.cpp — ONLY this file pays the cost
#include <device.h>
#include <heavy_driver.h>   // contained here!

class DeviceImpl { HeavyDriver d_driver; };
Device::Device() : d_impl_p(std::make_unique<DeviceImpl>()) {}
Device::~Device() = default;  // defined here so DeviceImpl is complete
```

---

## Slide 38 — PIMPL: Gotchas and Trade-offs

### What You Must Know

**Two mandatory `.cpp` definitions:**

```cpp
// ❌ WRONG — defaulting destructor in header triggers "incomplete type" error
class Device {
    std::unique_ptr<DeviceImpl> d_impl_p;
  public:
    ~Device() = default;   // DeviceImpl is INCOMPLETE here → compiler error
};

// ✅ CORRECT — declare in header, define in .cpp
class Device {
  public:
    ~Device();    // declared only
};
// In device.cpp:
Device::~Device() = default;  // DeviceImpl is COMPLETE here ✓
```

Same rule applies to copy constructor and move operations if needed.

**Trade-offs:**

| Benefit | Cost |
|---|---|
| Clients don't recompile when impl changes | One heap allocation per `Device` object |
| ABI stability (can change impl freely) | Indirect dispatch through pointer |
| Smaller header (fewer includes cascade) | Can't inline `Device`'s methods through impl |

> For short-lived, performance-critical value types: don't use PIMPL.  
> For long-lived objects, shared library APIs, or "implementation may change": always consider it.

---

## Slide 39 — Technique 5: Dumb Data

### Give Data Structure, Not Logic

**Scenario:** A `Node` stores a type tag. `Graph` needs to interpret that tag.  
If `Node` imports graph-level semantics, it creates an upward dependency.

```cpp
// ✅ DUMB DATA — Node just stores a number, makes no decisions
struct Node {
    int d_type;    // just a tag — no semantic knowledge embedded
    int d_value;
};

// Graph lives at a higher level and interprets the data
class Graph {
    void processNode(const Node& node) {
        switch (node.d_type) {
            case NODE_TYPE_SOURCE: /* ... */ break;
            case NODE_TYPE_SINK:   /* ... */ break;
        }
    }
};
```

**Key insight:**  
The low-level type stores **structure** (raw data).  
The higher-level component provides **semantics** (interpretation).

This keeps the low-level type clean and reusable,  
while allowing higher-level components to assign rich meaning to the data.

---

## Slide 40 — Technique 6: Callbacks

### Let the Caller Register Behavior

When a lower-level component needs to "report" or "notify" something,  
it should NOT reach up to a specific higher-level type.  
Instead, it should accept a **callback**.

**Four flavors of callbacks:**

```cpp
// 1. Data callback (function pointer + void* userdata)
using DataCb = void(*)(const char* msg, void* userData);

// 2. Function pointer callback
using Handler = void(*)(int eventType);

// 3. Functor / std::function callback
std::function<void(int)> onEvent;

// 4. Protocol (abstract interface) callback
class EventHandler {
  public:
    virtual ~EventHandler() = default;
    virtual void onEvent(int type) = 0;
};
```

Each flavor is more powerful (and heavier) than the previous.  
Choose the simplest one that satisfies the requirement.

---

## Slide 41 — Callbacks: Protocol Callbacks in Detail

### The Most Powerful Form

```cpp
// Level 1: Pure abstract interface (the "protocol")
class Serializer {
  public:
    virtual ~Serializer() = default;
    virtual void write(int value) = 0;
    virtual void write(const std::string& s) = 0;
};

// Level 2: Date uses Serializer without knowing any concrete implementation
class Date {
    int d_year, d_month, d_day;
  public:
    void serialize(Serializer& s) const {
        s.write(d_year);
        s.write(d_month);
        s.write(d_day);
    }
};

// Level 3+: Concrete serializers live at higher levels
class JsonSerializer  : public Serializer { /* ... */ };
class BinarySerializer: public Serializer { /* ... */ };
```

`Date` depends on `Serializer` (abstract). `Date` does NOT depend on `JsonSerializer`.  
The application wires the concrete implementation at the top level.

---

## Slide 42 — Technique 7: Protocol Class (Abstract Interface)

### Dependency Inversion, Lakos Style

**The Problem:** `DataStore` must persist data. But how?  
Hardcoding `SqliteDB` makes `DataStore` useless without SQLite.

```cpp
// ✅ Protocol class: a pure abstract base, no implementation
class PersistenceAdapter {
  public:
    virtual ~PersistenceAdapter() = default;
    virtual void save(const std::string& key, int value) = 0;
    virtual int  load(const std::string& key) = 0;
};

// Level 2: DataStore depends ONLY on the abstract interface
class DataStore {
    PersistenceAdapter& d_adapter;
  public:
    explicit DataStore(PersistenceAdapter& adapter) : d_adapter(adapter) {}
    void storeValue(const std::string& k, int v) { d_adapter.save(k, v); }
    int fetchValue(const std::string& k) { return d_adapter.load(k); }
};

// Level 3+: Concrete implementations (each in its own component)
class MemoryPersistence : public PersistenceAdapter { /* ... */ };
class FilePersistence   : public PersistenceAdapter { /* ... */ };
class SqlitePersistence : public PersistenceAdapter { /* ... */ };
```

---

## Slide 43 — Protocol Class: Why It Matters

### Swap Without Recompiling

```cpp
// Tests: fast, zero I/O
MemoryPersistence memStore;
DataStore testStore(memStore);   // runs in microseconds

// Production
FilePersistence fileStore("/var/data/prod.dat");
DataStore prodStore(fileStore);  // same DataStore, different adapter

// Upgrade to SQLite later
SqlitePersistence dbStore("prod.db");
DataStore newProdStore(dbStore); // DataStore never changes!
```

**What protocol classes enable:**
- Swap the persistence technology without touching `DataStore`
- Test `DataStore` in milliseconds (no file I/O)
- Write `DataStore` before the persistence backend exists
- Multiple storage backends coexist simultaneously

**Physical effect:**  
`DataStore`'s header includes only `PersistenceAdapter`'s header.  
Clients of `DataStore` have zero knowledge of SQLite, files, or memory backing.

---

## Slide 44 — Avoiding Excessive Link-Time Dependencies

### The Date Class That Got Too Fat

A `Date` class should provide primitive date operations.  
But "business day" logic requires holiday calendars, locale data, and market-specific rules.

```cpp
// ❌ BAD: business day logic IN the Date class
class Date {
  public:
    bool isBusinessDay(const Calendar& cal) const;  // requires Calendar.h!
};
// Every user of Date now must link against Calendar. Even in tests.

// ✅ GOOD: demoted to a utility
// bdlt_date.h: class Date { /* pure date arithmetic */ };
// bdlt_dateutil.h: struct DateUtil {
//     static bool isBusinessDay(const Date& d, const Calendar& cal);
// };
```

**Rule:** A component's link-time footprint should match its **conceptual footprint**.  
If a client only needs `Date`, they should not be forced to link `Calendar`.

**The gradual degradation pattern:**  
A clean `Date` class at Level 2 slowly has business logic added to it.  
One year later, it's at Level 7 and nobody can use it without the entire finance stack.

---

## Slide 45 — Lateral vs. Layered Architectures

### How Architecture Shapes Build Times

**Layered (vertical) architecture — preferred:**
```
Level 5:  ├─ A
Level 4:  ├─ B ─────────────────────────────┐
Level 3:  ├─ C                               │
Level 2:  ├─ D  E                            │
Level 1:  └─ F  G  H  I                     ↓
                                      Low CCD ✓
```

**Lateral (horizontal) architecture — antipattern:**
```
Level ?:  A ←→ B ←→ C ←→ D ←→ E
              ↑↓  ↑↓  ↑↓
          Everyone knows everyone
                                      High CCD ✗
```

**The analogy:**  
Layered = building a tower (each floor rests on those below, independently testable)  
Lateral = a spider web (cut one strand, everything else shifts)

---

## Slide 46 — Cumulative Component Dependency (CCD)

### Measuring Architectural Health

**Definition:**  
For each component in your system, count how many components it transitively depends on (including itself). Sum all of these counts. That is the **CCD**.

**Example:**

```
Layered (5 components A→B→C→D→E):
  A: 1 (itself only)
  B: 2 (B + A)
  C: 3 (C + B + A)
  D: 4 (D + C + B + A)
  E: 5 (E + D + C + B + A)
  CCD = 15

Lateral (everyone knows everyone):
  Each component depends on all 5 = 5 each
  CCD = 25   ← 67% worse for just 5 components
```

**At scale:** The difference is quadratic.  
100-component layered system: CCD ≈ 5050  
100-component lateral system: CCD = 10000  

**Practical impact:**  
Lower CCD = smaller recompile sets = faster incremental builds.  
CCD growing faster than line count = accumulating physical design debt.

---

## Slide 47 — Avoiding Inappropriate Physical Dependencies

### Don't Bet on a Single Technology

**Scenario:** A library component hardcodes a dependency on MySQL.  
Now ALL users of this library must have MySQL installed.

```cpp
// ❌ BAD: Library component depends on specific database
class UserRepository {
    MySqlConnection d_connection;  // requires mysql/mysql.h
  public:
    User findById(int id);
};

// ✅ GOOD: Library depends on abstract interface
class UserRepository {
    UserPersistence& d_store;   // abstract — requires only a forward header
  public:
    explicit UserRepository(UserPersistence& store);
    User findById(int id);
};
// Concrete MySqlUserPersistence lives in a separate component, higher up
```

**More antipatterns:**
- Conditional compilation for domain-specific features (`#ifdef BUYSIDE_ONLY`)
- Application-specific logic embedded in library components
- Accessing global/static state from library code
- Hiding potentially reusable code inside application-specific components

---

## Slide 48 — Unnecessary Compile-Time Dependencies

### Encapsulation Does Not Prevent Compile-Time Coupling

**A surprising fact:**  
Private member variables are fully visible in the header — even though clients can't access them.  
This means changing a private member forces ALL clients to recompile.

```cpp
// ❌ HEAVY header: private members force client recompilation when impl changes
class WidgetImpl {
  private:
    LargeDatabase d_db;        // clients see this even though it's private!
    ComplexCache  d_cache;     // → clients must include large_database.h
    NetworkConn   d_conn;      //    and complex_cache.h
};

// ✅ INSULATED header: private details hidden
class Widget {
    struct Impl;               // just a forward declaration
    std::unique_ptr<Impl> d_impl_p;  // only <memory> needed
  public:
    void doThing();
};
// Impl's details live entirely in widget.cpp
```

**Compile-time coupling vs. link-time coupling:**  
- Link-time: controlled by what you call at runtime
- Compile-time: controlled by what appears in your headers (including private sections!)

---

## Slide 49 — Architectural Insulation Techniques

### Three Approaches to Total Insulation

**1. Pure Abstract Interface (Protocol Class)**  
Best for: runtime polymorphism, multiple implementations  
Trade-off: virtual dispatch overhead

```cpp
class Renderer { virtual void draw(Shape*) = 0; };  // abstract
class OpenGLRenderer : public Renderer { /* ... */ };  // hidden from clients
```

**2. Fully Insulating Concrete Wrapper**  
Best for: wrapping a component you can't change  
Trade-off: forwarding all API calls through wrapper

```cpp
class Widget {               // public API — minimal header
    std::unique_ptr<WidgetImpl> d_impl_p;
  public:
    void doThing();          // forwards to d_impl_p->doThing()
};
```

**3. Procedural Interface (C-compatible)**  
Best for: shared libraries, cross-language ABI, maximum ABI stability  
Trade-off: verbose, no templates, manual resource management

```cpp
// C-compatible procedural interface
extern "C" {
    void* widget_create();
    void  widget_doThing(void* widget);
    void  widget_destroy(void* widget);
}
```

---

## Slide 50 — Insulation Trade-offs

### When to Insulate and When Not To

| Situation | Insulate? | Reason |
|---|---|---|
| Short-lived value types (`Point`, `Rect`) | ❌ No | Heap allocation overhead not worth it |
| Long-lived objects with complex implementations | ✅ Yes | Impl can change without client recompile |
| Shared library public API | ✅ Always | ABI stability is mandatory |
| Performance-critical, frequently constructed | ❌ No | Heap allocation in hot path |
| Implementation likely to change frequently | ✅ Yes | Cascading rebuilds avoided |
| Third-party library wrapping | ✅ Yes | Isolates third-party from your codebase |

**Rule of thumb:**  
If your header is **heavy** AND the implementation is **likely to change** → insulate.  
If your type is **simple and stable** → don't pay the overhead.

---

## Slide 51 — Designing with Components: A Real Example

### Building a Date Subsystem Properly

**Requirements:**
- Represent a date value
- Determine if today is a business day
- Parse and format date strings
- Support multiple calendar systems (US, UK, Japan...)

**Wrong approach (all in one):**
```cpp
class Date {
    bool isBusinessDay(const Calendar& cal) const;  // pulls in Calendar
    std::string format(const std::string& fmt) const;  // pulls in formatting
    static Date parse(const std::string& s);  // parsing engine in Date itself
};
```

**Right approach (levelized):**
```
Level 1: bdlt_date          — pure date value type
Level 2: bdlt_dateutil      — format/parse utilities
Level 3: bdlt_calendar      — calendar data
Level 4: bdlt_calendarutil  — business day checks
Level 5: bdlt_calendarcache — performance-optimized caching layer
```

Each level only knows about lower levels.  
Clients link EXACTLY what they need.

---

## Slide 52 — The Component as the Unit of Design

### Thinking in Components vs. Thinking in Classes

Traditional OOP mindset:  
*"I need a `Calendar` class with methods for business day calculations."*

Component-based physical design mindset:  
*"I need:*
- *A `bdlt_calendar` component (pure data structure)*  
- *A `bdlt_calendarutil` component (algorithms on calendars)*  
- *Both tested independently*  
- *Clients choose which to link*"

**This changes how you estimate complexity:**  
Not "how many classes?" but "how many components, at what levels?"

A 50-class system with one cycle can be worse to maintain than  
a 200-component system with clean levelization.

> The component is the UNIT of design, testing, and deployment.

---

## Slide 53 — Subpackages and Subordinate Components

### Advanced Organizational Patterns

**Subpackages:**  
Large packages can be organized into subpackages to manage internal complexity.  
These are an organizational tool — clients still see the package-level interface.

**Subordinate components:**  
Sometimes a component needs a helper that is NOT part of the public API.  
A subordinate component (named with an underscore convention) handles implementation details:

```
bdlt_date.h           ← public component
bdlt_date.cpp
bdlt_date_impl.h      ← subordinate: internal to package, not installed
bdlt_date_impl.cpp
```

**Component-private classes:**  
Small helpers that are entirely private to a single component can be forward-declared  
in the header and defined in the `.cpp`, with an underscore naming convention:

```cpp
class Date_Impl;   // declared in date.h, defined in date.cpp
// Clients see the name but cannot access it
```

---

## Slide 54 — Legacy, Third-Party, and Open-Source Software

### Integrating Code That Doesn't Follow the Methodology

**Legacy software:** Code predates the methodology.  
Wrap it in a conforming component that acts as an adapter.

**Third-party software:** You can't change it.  
Create a wrapper component that presents a clean interface,  
hiding the third-party header from your clients.

```cpp
// thirdparty_json_wrapper.h  — your wrapper
class JsonDocument {          // YOUR clean interface
  public:
    bool parse(const std::string& text);
    std::string getString(const std::string& key) const;
    // NO nlohmann::json visible in this header!
};

// thirdparty_json_wrapper.cpp
#include <thirdparty_json_wrapper.h>
#include <nlohmann/json.hpp>   // contained — clients NEVER see this
```

**Open-source software:**  
Use the same wrapper pattern.  
Isolate the OSS dependency to one component.  
If you need to swap the library later, only ONE component changes.

---

## Slide 55 — From Development to Deployment

### The Value of Unique Names

Every `.h` and `.o` file must have a **globally unique name** across the enterprise.

**This enables:**

1. **Flat deployment:** Install all headers in `/usr/include` without collisions
2. **Automated tooling:** Any tool can find a header without knowing paths
3. **Enterprise-wide refactoring:** Rename a component globally, grep finds exactly one file
4. **Flexible build systems:** Build systems can organize source differently from deployment

**Deployment flexibility:**  
The same source can be deployed as:
- Static libraries (`.a`) — fast link, larger executables
- Shared libraries (`.so`) — runtime binding, smaller executables
- Source distribution — maximum portability

The physical design must support ALL deployment modes.  
The choice of deployment mode is a **business decision**, not an architectural one.

---

## Slide 56 — Metadata

### Capturing Dependencies Explicitly

For packages and package groups, explicit **metadata** captures:

| Type | Content | Purpose |
|---|---|---|
| **Membership** | List of components in this package | Build system, validation |
| **Dependency** | Allowed external packages | Architectural enforcement |
| **Build requirements** | Compiler flags, platform conditions | Build automation |
| **Enterprise policy** | Licensing, ownership, compliance | Governance |

**Why explicit metadata matters:**  
Without it, dependency relationships exist only implicitly in `#include` chains.  
With it, a build system can **verify** that actual dependencies respect the declared constraints.

```
# bdlt.dep — allowed external package groups
bdlb       ← bdlt may depend on bdlb
bslstl     ← bdlt may depend on bslstl
# bdlt must NOT depend on any application-level package
```

**Metadata is policy, expressed as code.**

---

## Slide 57 — Diagnostic Framework

### How to Diagnose Physical Design Problems

When something feels wrong in a codebase, ask these questions in order:

```
1. "Is there a cycle here?"
   → Assign level numbers. If you can't, you found a cycle.

2. "Can I test this component in isolation?"
   → If not, there's a hidden dependency. Find it.

3. "Why is the build so slow?"
   → Check what's in the headers. CCD tells you the numerical answer.

4. "Why can't I reuse this without dragging in everything?"
   → The component has non-primitive functionality, or inappropriate deps.

5. "Why did changing X break Y?"
   → Transitive include chain. Implied dependency. Should have used
     forward declaration or PIMPL.
```

**Tools:**
- `cpp -M file.cpp` — shows all transitive includes
- `nm -C binary.o` — shows all symbols in an object file
- Dependency analysis tools (Bloomberg's `bde_verify`, etc.)

---

## Slide 58 — The Three Golden Rules

### Everything Reduces to These

**Rule 1: Acyclic Dependencies**  
No cycles in the physical dependency graph between components, packages, or package groups.  
If you cannot assign a level number to a component, you have a violation.

**Rule 2: Hierarchical Testability**  
Each component can be tested independently using only lower-level components.  
If your test driver requires components at the same or higher level, your design is wrong.

**Rule 3: Minimal Coupling**  
Expose only what clients need. Hide everything else in `.cpp` files.  
Track CCD. Apply PIMPL or protocol classes when headers become heavy.

```
Cycle?        → Levelize using one of the 7 techniques
Can't test?   → Find and cut the hidden dependency
Build too slow → Reduce header weight, reduce CCD
```

---

## Slide 59 — Quick Reference: Design Rules Checklist

### Use This During Code Review

**Component Rules (CP):**
- [ ] CP1: Header is self-sufficient (compiles standalone)
- [ ] CP2: `.cpp` includes its own header FIRST
- [ ] CP3: No non-header content exported from `.cpp`
- [ ] CP4: Dependencies fully determined by `#include` chains

**Naming Rules (DR):**
- [ ] DR-1: No cyclic physical dependencies
- [ ] DR-2: Component filename = package prefix + underscore + local name
- [ ] DR-3: Header names are unique enterprise-wide
- [ ] DR-4: Data members prefixed `d_`, static members prefixed `s_`
- [ ] DR-5: No `using namespace` in headers
- [ ] DR-6: No `using` declarations in headers (outside function scope)
- [ ] DR-7: Free operators declared in same namespace as the type
- [ ] DR-8: Use `<iosfwd>` (not `<iostream>`) in headers

**Architecture Rules:**
- [ ] Every component has an assigned level number
- [ ] Test drivers link only lower-level components
- [ ] Non-primitive functionality lives outside the core component

---

## Slide 60 — Summary and Key Takeaways

### What We Covered

| Chapter | Core Concept |
|---|---|
| Ch 0: Motivation | Software capital vs. debt; physical design matters as much as logical design |
| Ch 1: Components | The `.h`/`.cpp` pair; component properties; level numbers; declarations vs. definitions |
| Ch 2: Packaging | Components → Packages → Package Groups; name cohesion; design rules; hierarchical testability |
| Ch 3: Factoring | Primitive vs. non-primitive; levelization techniques; CCD; insulation |

**The philosophy in one sentence:**  
> *Design your physical structure so that every component can be compiled, tested, and deployed independently — and everything above it benefits automatically.*

**Further reading:**
- *Large-Scale C++, Volume I* — John Lakos (Pearson, 2020)
- Bloomberg BDE Library: https://github.com/bloomberg/bde
- Lakos's original: *Large-Scale C++ Software Design* (1996)

---

*End of Workshop*
