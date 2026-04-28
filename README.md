# Large-Scale C++, Volume I: Process and Architecture
## Study Guide with Practical Code Examples

**Source**: *Large-Scale C++, Volume I: Process and Architecture* by John Lakos (Pearson, 2020)

---

## Overview

This study guide summarizes the key ideas, design rules, and practical techniques from John Lakos's *Large-Scale C++*. Each chapter directory contains:
- A `README.md` with a detailed summary of concepts
- Compilable C++ source files demonstrating each concept
- A `CMakeLists.txt` to build all examples

The book teaches a **component-based physical design methodology** developed at Bloomberg LP over decades. Its central theme: **managing complexity at scale requires disciplined physical design as much as (or more than) logical design**.

---

## Directory Structure

```
large_scale_cpp/
├── README.md                          ← This file
├── CMakeLists.txt                     ← Top-level CMake script
│
├── ch0_motivation/                    ← Ch 0: Why physical design matters
│   ├── README.md
│   ├── CMakeLists.txt
│   └── src/
│       ├── software_capital_demo.cpp
│       └── hierarchical_reuse_demo.cpp
│
├── ch1_compilers_linkers_components/  ← Ch 1: Build process & components
│   ├── README.md
│   ├── CMakeLists.txt
│   └── src/
│       ├── my_stack.h
│       ├── my_stack.cpp
│       ├── my_stack.t.cpp            ← test driver
│       ├── linkage_demo.h
│       ├── linkage_demo.cpp
│       ├── include_guard_demo.h
│       └── component_demo.cpp
│
├── ch2_packaging_design_rules/        ← Ch 2: Packages, naming, design rules
│   ├── README.md
│   ├── CMakeLists.txt
│   └── src/
│       ├── pkg/                      ← Example package "pkg"
│       │   ├── pkg_date.h
│       │   ├── pkg_date.cpp
│       │   ├── pkg_date.t.cpp
│       │   ├── pkg_dateutil.h
│       │   └── pkg_dateutil.cpp
│       └── design_rules_demo.cpp
│
├── ch3_physical_design_factoring/     ← Ch 3: Levelization, avoiding cycles
│   ├── README.md
│   ├── CMakeLists.txt
│   └── src/
│       ├── levelization/
│       │   ├── opaque_pointer/
│       │   ├── dumb_data/
│       │   ├── callbacks/
│       │   ├── protocol_class/
│       │   └── demotion/
│       └── insulation/
│           ├── compile_time_coupling/
│           └── pimpl_insulation/
│
└── appendix_design_rules/             ← Quick reference: all design rules
    └── README.md
```

---

## Core Concepts Summary

| Concept | Book Location | Key Idea |
|---|---|---|
| Component | §1.6 | A `.h`/`.cpp` pair — the atomic unit of design |
| Component Properties 1–4 | §1.6, §1.11 | Rules ensuring component integrity |
| Unit of Release (UOR) | §2.2 | The outermost deployable physical aggregate |
| Package | §2.4.5 | Smallest architecturally significant aggregate above component |
| Package Group | §2.9 | Collection of related packages, forms a UOR |
| No Cyclic Dependencies | §2.2.24 | Physical dependencies must form a DAG |
| Levelization | §3.5 | Techniques to break/avoid cyclic dependencies |
| Insulation | §3.6–3.8 | Reducing unnecessary compile-time coupling |
| CCD | §3.7.4 | Cumulative Component Dependency — a complexity metric |

---

## Building All Examples

```bash
mkdir -p build && cd build
cmake ..
make
```

---

## Key Design Philosophy

> *"Physical design is not just about files and directories. It is about controlling the envelope of dependencies — both compile-time and link-time — to ensure that software can be developed, tested, reviewed, and reused independently."*
> — John Lakos, paraphrased

The three pillars of the methodology:
1. **Component-based design** — every reusable piece of code lives in a component
2. **Hierarchical physical aggregation** — components → packages → package groups
3. **Acyclic dependency graph** — no cycles at any level of aggregation
