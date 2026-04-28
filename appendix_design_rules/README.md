# Appendix: Design Rules Quick Reference

All design rules and design imperatives from *Large-Scale C++, Volume I*  
by John Lakos. Use this as a quick checklist during code review.

---

## Chapter 1 — Compilers, Linkers, and Components

| Rule | Description |
|------|-------------|
| **CP1** | Every component header is self-sufficient (compiles alone without prior includes) |
| **CP2** | Every `.cpp` file includes its own header as the **first** `#include` |
| **CP3** | The component's `.cpp` file has no **logical** content not declared in its header |
| **CP4** | The component's `.h` and `.cpp` files share the same base name |

**Include Guard Convention:**
```cpp
#ifndef INCLUDED_<COMPONENT_NAME>
#define INCLUDED_<COMPONENT_NAME>
// ...
#endif  // INCLUDED_<COMPONENT_NAME>
```

**Naming Conventions (Lakos style):**
- Data members: `d_` prefix  
- Static class members: `s_` prefix  
- Internal (file-scope) items: anonymous namespace

---

## Chapter 2 — Packaging and Design Rules

### Physical Aggregation Hierarchy

```
Package Group  (e.g., bdl)
  └── Package  (e.g., bdlb)
        └── Component  (e.g., bdlb_string)
              ├── bdlb_string.h
              └── bdlb_string.cpp
```

### Design Rules (DR)

| Rule | Description |
|------|-------------|
| **DR-1** | No cyclic physical dependencies between components |
| **DR-2** | Every component file name matches its package-prefixed name |
| **DR-3** | A component's `.h` must not `#include` anything from a higher-level package |
| **DR-4** | A component's name must be unique across all packages in a UOR |
| **DR-5** | No `using` declarations in header files (pollutes client namespace) |
| **DR-6** | No `using namespace` in header files |
| **DR-7** | Free operators must be declared in the same namespace as the type they operate on |
| **DR-8** | Use `iosfwd` for forward-declaring stream types instead of including `<iostream>` |

### Hierarchical Testability Requirement

Every component must be testable using only components at strictly **lower** levels.  
A test driver (`*.t.cpp`) for component at level N may only `#include` components at levels 1..N.

### Unit of Release (UOR)

A UOR is the unit that is released, versioned, and linked together:
- A **package** for small-to-medium libraries
- A **package group** for larger systems
- Clients depend on a UOR as a whole; no partial linking

---

## Chapter 3 — Physical Design and Factoring

### Levelization Techniques

| Technique | When to Use | How |
|-----------|-------------|-----|
| **Classic Levelization** | Bidirectional logical relationship | Make one side NOT know about the other physically |
| **Escalation** | Two components need interaction logic | Move interaction to a NEW higher-level component |
| **Demotion** | Low-level code trapped in high-level component | Move to a new component at the appropriate level |
| **Opaque Pointer (PIMPL)** | Header includes too many/heavy deps | Forward-declare impl class; `unique_ptr<Impl>` in header |
| **Dumb Data** | Low-level type has logical dependencies upward | Use enums/raw data in low-level; interpret at higher level |
| **Callbacks** | Low-level needs to notify high-level | Accept a function pointer, `std::function`, or protocol class |
| **Protocol Class** | Need runtime polymorphism without physical coupling | Pure abstract base class; concrete impls at higher levels |
| **Factoring** | Non-primitive elements slow down development | Identify and separate primitive vs. non-primitive elements |

### CCD (Cumulative Component Dependency)

```
CCD = Σ (for each component C) [count of components that C transitively depends on, including itself]
```

- **Layered** (vertical) architecture: CCD = O(n²/2) — preferred
- **Lateral** (horizontal) architecture: CCD = O(n²) — avoid

Lower CCD → fewer components to recompile when a low-level component changes.

### Insulation Rules

| Situation | Action |
|-----------|--------|
| Header includes many/heavy headers | Apply PIMPL to hide implementation |
| Shared library (ABI stability needed) | Always insulate public API classes |
| Simple value types (Date, Point) | Do NOT insulate (heap alloc overhead not worth it) |
| Implementation details likely to change | Insulate to avoid cascading recompilation |

### Primitive vs. Non-Primitive

| Kind | Description | Location |
|------|-------------|----------|
| **Primitive** | Cannot be implemented efficiently without lower-level access | Inside the component/class |
| **Non-primitive** | Can be implemented using the component's primitive interface | Outside (free function, utility) |

Moving non-primitives outside the class reduces coupling and compile overhead.

---

## Summary: The Three Golden Rules of Large-Scale C++ Physical Design

1. **Acyclic Dependencies** — No cycles in the physical dependency graph between components, packages, or package groups.

2. **Hierarchical Testability** — Each component can be tested independently using only lower-level components. Test drivers link only against components at strictly lower levels.

3. **Minimal Coupling** — Expose only what clients need. Hide everything else in `.cpp` files. Track CCD and insulate when headers become heavy.
