# Chapter 3: Physical Design and Factoring

**Book Pages**: 495–934 | *Large-Scale C++, Volume I* by John Lakos

---

## Overview

Chapter 3 is the longest and most technically dense chapter. It covers the practical techniques for designing software that can be built, tested, and maintained at scale. The central topic is **levelization** — the art of breaking cyclic dependencies and structuring code so that the dependency graph is a proper DAG.

---

## 3.1 Thinking Physically

### Pure Logical Design Is Naïve

Traditional software design focuses on logical relationships: "what classes should exist, and how should they relate?" This is necessary but not sufficient. Every logical relationship has a **physical cost** — it creates a dependency in the build system.

**The right question**: "Where should this piece of code *physically live*?"

### Software Design Space Has Direction

Dependencies flow in one direction: **downward** (from higher-level to lower-level components). Any design decision that introduces an upward or sideways dependency is suspect.

```
High Level:  Application, Calendar, Persistence
Mid Level:   Date, String, Container
Low Level:   Math, Memory, Platform
```

### Every Component Has Absolute Location

A component doesn't just "depend on" other things in a vague sense — it occupies a specific **level number** in the dependency graph. Adding a dependency always potentially increases your level number (increasing the size of your dependency cone).

---

## 3.2 Avoiding Poor Physical Modularity

### The Polygon Example

Lakos uses a `Polygon` class to illustrate the difference between **primitive** and **non-primitive** functionality.

**Primitive functionality**: Operations that are inherently part of the abstraction and cannot be implemented efficiently without access to private state.
- `addVertex()`, `area()`, `perimeter()`, `vertexCount()` ← **PRIMITIVE**

**Non-primitive functionality**: Operations that could be implemented in terms of the primitive interface, without needing private access.
- `isConvex()`, `rotate()`, `translate()` ← could be utilities
- `isRegular()`, `toPolarCoordinates()` ← definitely utilities

**Design Rule**: Put only *primitive* functionality in the main component. Non-primitive operations go in *utility* components that USE the primitive interface.

### Why Separate Non-Primitive Functionality?

1. **Clients who don't need the extra operations don't link against them** (reduced link-time footprint)
2. **The primitive component is simpler, more stable** (fewer reasons to change)
3. **Non-primitive utilities can be extended independently** (open/closed principle)
4. **Testing the primitive component is simpler** (fewer dependencies)

---

## 3.3 Grouping Things Physically

### Four Criteria for Colocating Classes

When should two classes be in the **same component**?

| Criterion | Example | Why? |
|---|---|---|
| **Friendship** | `Node` and `NodeIterator` | `NodeIterator` needs private access to `Node` |
| **Cyclic dependency** | `Event` and `EventQueue` | They mutually reference each other |
| **Single solution** | `DayCount` base + concrete implementations | Logically inseparable |
| **Flea on elephant** | `StackIterator` and `Stack` | Iterator is tiny, tightly coupled |

**Otherwise, keep classes in separate components.** The default is separation; colocation requires justification.

---

## 3.4 Avoiding Cyclic Link-Time Dependencies

### Why Cycles Are Fatal at Scale

Consider:
```
Component A includes component B
Component B includes component A
```

Consequences:
1. **You cannot test A without B** (and vice versa)
2. **You cannot build a static library** that satisfies both A and B without having both
3. **Any change to A might require recompiling B** (and vice versa)
4. **The dependency graph is not a DAG** — level numbers become undefined

The situation doesn't improve at higher levels. Cyclic packages require cyclic libraries, which require circular link orders — impossible with standard linkers.

---

## 3.5 Levelization Techniques

This section describes the most important practical techniques for **breaking cyclic dependencies** in existing code and **preventing them** in new code.

### 3.5.1 Classic Levelization

**Problem**: Two classes that currently depend on each other.

**Solution**: Identify what each class *really needs* from the other. Often, one class needs only a minimal interface — which can be extracted.

```
Before (cycle):
  Manager ←→ Employee

After (acyclic):
  Manager → Employee    (Manager holds Employee objects)
  Employee knows nothing about Manager
```

**Key insight**: Bidirectional logical relationships can almost always be implemented with *unidirectional* physical dependencies.

---

### 3.5.2 Escalation

**Problem**: Class `A` and class `B` are at the same level but interact in a way that creates a cycle.

**Solution**: Escalate the interaction to a higher-level component `C` that knows about both `A` and `B`.

```
Before (cycle):
  EventQueue ←→ Event

After (escalated):
  EventSystem (Level 3) → EventQueue (Level 2) → Event (Level 1)
  EventSystem handles the interaction
```

---

### 3.5.3 Demotion

**Problem**: A "utility" has been placed in a component that is too high in the hierarchy, causing other components to depend on a component that's heavier than they need.

**Solution**: Demote the utility to a lower-level component so others can depend on it without pulling in unwanted baggage.

```
Before: all_utils (Level 5) contains:
  - string_hash (Level 1 capability)
  - date_parser (Level 3 capability)
  - report_generator (Level 5 capability)
  
Components at Level 1 that need string_hash must link against Level 5!

After (demoted):
  str_hash (Level 1) — just the hash function
  date_util (Level 3) — date parsing
  report (Level 5) — report generation
```

---

### 3.5.4 Opaque Pointers (Handle/Body / PIMPL)

**Problem**: Class `Manager` stores a reference to `Employee` objects in its header, forcing all `Manager` clients to include `Employee`'s header — even if they never use `Employee` directly.

**Solution**: Use an opaque pointer (forward declaration + pointer).

```cpp
// BEFORE (strong dependency — pulls in employee.h everywhere):
// manager.h
#include "employee.h"

class Manager {
    Employee d_employee;   // full type required
};

// AFTER (opaque pointer — forward declaration only):
// manager.h
class Employee;  // forward declaration — no #include needed!

class Manager {
    Employee* d_emp_p;   // pointer to Employee — only forward decl needed
};
```

**Effect**: Clients of `Manager` no longer transitively include `Employee`'s header. The compile-time dependency is broken.

**Trade-off**: The `Manager` implementation (`.cpp`) still needs `#include <employee.h>`, so the *link-time* dependency remains. But compile-time isolation is often very valuable at scale.

---

### 3.5.5 Dumb Data

**Problem**: Class `Node` stores type information about itself in a field, and `Graph` switches on this type. When the type changes, both `Node` and `Graph` must be updated — and `Graph` must include `Node`'s full type definition.

**Solution**: Use "dumb data" — `Node` stores raw data (e.g., an `int` type tag) and `Graph` interprets it. `Node` knows nothing about `Graph`; `Graph` knows about `Node`'s data layout but not vice versa.

```cpp
// "Dumb" node: just stores data, makes no decisions about interpretation
struct Node {
    int d_type;       // type tag — just a number, no semantic knowledge
    int d_value;
};

// Graph interprets the node data
class Graph {
    void processNode(const Node& node) {
        switch (node.d_type) {  // Graph knows what the types mean
            case 1: /* ... */ break;
            case 2: /* ... */ break;
        }
    }
};
```

---

### 3.5.6–3.5.7 Callbacks

Callbacks are the most flexible levelization technique. Instead of class `A` knowing about class `B` directly, `A` holds a **callback** that allows external code to hook in behavior.

#### Data Callbacks

```cpp
// Level 1: Logger doesn't know about any specific log handler
class Logger {
    using DataCallback = void(*)(const char* message, void* userData);

    DataCallback d_callback;
    void*        d_userData;
  public:
    void setCallback(DataCallback cb, void* data) {
        d_callback = cb;
        d_userData = data;
    }
    void log(const char* msg) {
        if (d_callback) d_callback(msg, d_userData);
    }
};
```

#### Function Callbacks

```cpp
// Level 1: EventQueue doesn't know about any specific handler
class EventQueue {
    using Handler = void(*)(int eventType);
    Handler d_handler;
  public:
    void setHandler(Handler h) { d_handler = h; }
    void dispatch(int type) { if (d_handler) d_handler(type); }
};
```

#### Functor Callbacks

```cpp
// Level 1: Accepts any callable — decoupled from specific types
class Timer {
    std::function<void()> d_callback;
  public:
    template <typename F>
    void setCallback(F&& f) { d_callback = std::forward<F>(f); }
    void fire() { if (d_callback) d_callback(); }
};
```

#### Protocol (Interface) Callbacks

The most powerful form: define an abstract interface that high-level code must implement, and pass it down.

```cpp
// Level 1: Abstract interface
class Serializer {
  public:
    virtual ~Serializer() = default;
    virtual void write(int value) = 0;
    virtual void write(const std::string& s) = 0;
};

// Level 2: Date uses Serializer interface (not any concrete implementation)
class Date {
  public:
    void serialize(Serializer& s) const {
        s.write(d_year);
        s.write(d_month);
        s.write(d_day);
    }
};
```

---

### 3.5.8 Manager Class

When two classes at the same level both need to interact, introduce a **Manager** class at a higher level that orchestrates their interaction.

```
Level 1: Event     (pure data)
Level 1: EventQueue (stores Events)
Level 2: EventManager (links Event creation to EventQueue processing)
```

---

### 3.5.9 Factoring

"Factoring" means extracting common functionality shared by multiple components into its own component, which those components can then both depend on.

```
Before: A has {shared_stuff + A_stuff}
        B has {shared_stuff + B_stuff}  [duplication!]

After:  Common has {shared_stuff}
        A has {A_stuff} → depends on Common
        B has {B_stuff} → depends on Common
```

---

### 3.5.10 Escalating Encapsulation (Multicomponent Wrappers)

Sometimes a component's physical design can't be changed but needs to be wrapped for use in a higher-level context. A *wrapper component* encapsulates the low-level component and presents a cleaner interface.

---

## 3.6 Avoiding Excessive Link-Time Dependencies

### The Well-Factored Date Class Example

A `Date` class should provide primitive date operations. But "business day" logic (which requires knowledge of holidays, locales, and market calendars) should NOT be in the same component.

```
WRONG (too much in one component):
  bdlt_date.h: class Date {
      bool isBusinessDay(const Calendar& cal) const;  // requires Calendar!
  };
  // Now every user of Date must link against Calendar.

RIGHT (demoted functionality):
  bdlt_date.h:     class Date { /* pure date arithmetic */ };
  bdlt_dateutil.h: struct DateUtil { static bool isBusinessDay(Date, Calendar); };
```

---

## 3.7 Lateral vs. Layered Architectures

### Cumulative Component Dependency (CCD)

**CCD** is the total number of component dependencies in the system (sum of "how many components does each component depend on, transitively").

Lower CCD = simpler system = easier to build, test, and understand.

**Layered architecture**: Dependencies flow strictly downward. CCD grows linearly with the number of components.

**Lateral architecture** (antipattern): Many peer components all depend on each other. CCD grows quadratically — each new component may depend on all existing ones.

### The Construction Analogy

Think of layered architecture as building a tower:
- Each floor rests only on the floors below it
- You can test each floor independently before adding the next
- If floor 3 is broken, it only affects floors 4+ (not floors 1–2)

A lateral architecture is like a spider web:
- Every node connected to every other
- Breaking one thread may unravel the whole structure
- Impossible to test any node in isolation

---

## 3.8 Avoiding Inappropriate Link-Time Dependencies

**Problem**: "Betting on a single technology" — implementing a component that hardcodes a dependency on a specific database, OS API, or third-party library.

**Solution**: Use **abstract interfaces** (protocol classes) to decouple the logical interface from the concrete implementation.

```cpp
// Abstract interface — no dependency on any specific persistence technology
class PersistenceAdapter {
  public:
    virtual ~PersistenceAdapter() = default;
    virtual void save(const std::string& key, int value) = 0;
    virtual int  load(const std::string& key) = 0;
};

// Concrete implementations (separate components, higher-level packages):
class SqlitePersistence : public PersistenceAdapter { /* ... */ };
class FilePersistence   : public PersistenceAdapter { /* ... */ };
```

---

## 3.9 Ensuring Physical Interoperability

### Key Issues at Scale

1. **Conditional compilation**: Domain-specific `#ifdef` guards for platform-specific code
2. **Application-specific dependencies**: Code that only makes sense in one application context should NOT be in a reusable library
3. **Global resources**: Minimize use of global/static state (complicates testing and multi-threaded use)
4. **Hidden header files**: Sometimes legitimate for implementation-detail headers shared only within a package

---

## Code Examples

| Directory | Demonstrates |
|---|---|
| [src/levelization/opaque_pointer/](src/levelization/opaque_pointer/) | PIMPL / Handle-Body pattern |
| [src/levelization/dumb_data/](src/levelization/dumb_data/) | Dumb data to break upward deps |
| [src/levelization/callbacks/](src/levelization/callbacks/) | Data, function, functor, protocol callbacks |
| [src/levelization/protocol_class/](src/levelization/protocol_class/) | Abstract interface (protocol class) |
| [src/levelization/demotion/](src/levelization/demotion/) | Demoting overloaded components |
| [src/insulation/pimpl_insulation/](src/insulation/pimpl_insulation/) | Compile-time dependency insulation |
| [src/insulation/compile_time_coupling/](src/insulation/compile_time_coupling/) | Excessive vs. minimal coupling |
