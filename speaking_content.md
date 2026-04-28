# Speaking Content — Large-Scale C++ Workshop
### Slides 1–20 · Speaker notes, conversational style

---

## Slide 1 — Title

Hey everyone, welcome. So today we're doing a deep dive into something that honestly doesn't get talked about nearly enough in C++ circles — and that's *physical design*.

Not algorithms, not design patterns, not the latest C++23 features. We're talking about the boring-sounding stuff: your files, your headers, which headers include which other headers, and how all that plumbing affects whether your codebase can actually scale or slowly turns into a pile of spaghetti.

This whole workshop is based on a book called *Large-Scale C++* by John Lakos. He's a Bloomberg engineer and he spent decades figuring out how you build a codebase of millions of lines of C++ that doesn't collapse under its own weight. The ideas in this book are battle-tested at scale — Bloomberg's entire trading infrastructure runs on code written following these rules.

I'm going to make this practical. Lots of code examples, real concepts you can apply. Let's get into it.

---

## Slide 2 — The Real Problem

Before I get into any theory, let me throw some scenarios at you and see if any hit home.

You change a single line in one header file — not even the public API, just an internal detail — and your entire project starts rebuilding. Fifteen minutes later you're still waiting. That's not a hardware problem. That's a physical design problem.

Or you try to write a unit test for one class. Simple enough, right? Except to link your test binary you need to pull in 30 other libraries. Your "unit test" now takes 3 seconds just to link and requires a database connection to run.

Or worse — component A includes component B's header, and component B includes component A's header. A depends on B, B depends on A. Neither one can be compiled or tested without the other. You've got a cycle and now your whole testing strategy falls apart.

These are not rare edge cases. These are the *normal* state of large C++ codebases that were built without thinking about physical design. And the scary part is — they get there gradually. One extra include here, one convenience dependency there. Nobody notices until the codebase is already a mess.

The root cause is always the same: no discipline around physical dependencies. And that's exactly what this workshop is about — how to fix that.

---

## Slide 3 — What Is Physical Design?

OK so before we go further, let me make sure we're all on the same page about what "physical design" actually means, because this word gets used differently in different contexts.

Logical design is the stuff most books teach — what classes should exist, what methods they should have, how they relate to each other. Inheritance, composition, the Strategy pattern, all of that.

Physical design asks a different question: *which files should exist?* What does each file include? What does a client need to link? How do compile times change when someone touches a header?

Here's why it matters: logical design is about *what your code means*. Physical design is about *whether your code can actually be built, tested, and deployed* at scale.

Lakos's key insight is that you need both. You can have perfect object-oriented design but terrible physical design — and your codebase will still be a mess to build, test, and maintain. The two have to work together.

Most C++ books spend 400 pages on logical design and maybe one paragraph on physical design. This book flips that — it's almost entirely about the physical side. That's what makes it rare and valuable.

---

## Slide 4 — Application vs. Library Software

Now here's a distinction the book makes right at the start, and it's worth understanding because it shapes the whole discussion.

Application software is software written for end users. Think of an app you ship to customers. It changes with every sprint, it might get deprecated in two years, and its main job is just to work — not to be reused by other developers.

Library software is completely different. Its consumers are other programmers. It needs to stay stable for years. It needs to be independently linkable and testable. Other teams, maybe even other companies, are going to build on top of it.

The rules are different for each. Applications can get away with more internal coupling because nobody's reusing their internals. Libraries have to be very disciplined because they're the foundation everything else sits on.

Here's the trap though. Even if you think you're writing "just an application" — the lower layers of that application are, functionally, libraries. Your date utilities, your event system, your string helpers — other parts of the app depend on them. If you design them carelessly, you'll feel that pain every single time you try to test something or reuse code across features.

So the rule is simple: any code that's going to outlive its first consumer should be designed with library-level discipline. That's the mindset we're carrying through this whole workshop.

---

## Slide 5 — Software Capital

This is one of my favourite framings in the book. Lakos treats software as an economic asset — and I think it's a really useful mental model.

Think about what "software capital" looks like. You've got components that are independently testable. Their dependencies are minimal and clean. Their interfaces are stable. You write them once, and then you can pull them into the next project, and the project after that. Each new thing you build can leverage everything you've already built. The value compounds over time. That's capital.

Now think about the opposite — software debt. Everything depends on everything else. You can't change anything without breaking ten other things. To write a unit test you need the entire system linked. You can't reuse a utility because it drags half the application along with it. Instead of reusing code, you rewrite it from scratch every project. The value doesn't compound — it decays. And you're paying interest in the form of slow builds, fragile tests, and constant rework.

The whole Lakos methodology is about making choices that accumulate capital instead of debt. Every rule we're going to look at today — file naming, include discipline, dependency rules — they all serve one goal: keep the codebase as an asset that grows in value, not a liability that grows in cost.

---

## Slide 6 — Hierarchical Reuse

So the book talks about three levels of software maturity. And this is a nice progression to understand before we get into the technical details.

The first level is *collaborative software* — pieces that only work together, like a cracked plate. You can't take one piece out and use it somewhere else. It only makes sense as a whole. Most big messy codebases are at this level.

The second level is *reusable software* — a single component with a clean interface. You can use it in isolation. You can test it alone. That's already much better.

But the *goal* is the third level: *hierarchically reusable software*. This is where the magic happens. Your Level 1 components are clean and standalone. Level 2 components build on Level 1. Level 3 builds on Level 2. And critically — each level can be independently tested and released.

Look at the diagram on the slide: Integer Math at the bottom, Date Library above it, Calendar Subsystem above that, Application at the top. Each layer builds on the ones below, and each layer can be tested without anything above it.

This is only possible when dependencies are acyclic. If A depends on B and B depends on A, you can't have this tower. The whole structure depends on keeping the dependency arrows pointing in one direction: downward.

---

## Slide 7 — Malleable vs. Stable Software

One more concept from Chapter 0 before we get into the mechanics of how all this works.

Lakos distinguishes between malleable and stable software. Malleable software is expected to change frequently — it bends to business requirements, sprints, feature requests. Think of the application layer. It changes every week and that's fine.

Stable software is the opposite. It evolves slowly and carefully. When it does change, existing clients should not be broken. This is library software. It needs to be right the first time, or at least right in a way that can be extended without breaking things.

The point isn't that one is better than the other — they serve different purposes. The point is that you need to know which one you're writing, because the discipline level is different.

And here's the thing that catches people out: software that starts as "just an application" often grows into something other teams depend on. If you designed it as malleable, and it becomes someone else's stable foundation, you've got a problem. Adding proper physical design later is painful and expensive.

So the practical advice is: when in doubt, design for stability. The extra upfront effort of good physical design is tiny compared to the cost of rework later. This wraps up Chapter 0 — now let's get into the actual mechanics, starting with the most fundamental unit of everything: the component.

---

## Slide 8 — The Component

Chapter 1 starts with a surprisingly simple idea: the component is the fundamental unit of physical design.

A component is just a `.h` file and a `.cpp` file that share the same base name. `my_stack.h` plus `my_stack.cpp` — that's one component. Very simple on the surface, but the book adds rules that turn this simple pair into a powerful building block.

The `.h` file is the *contract* — the public interface, what clients include, what they see. The `.cpp` file is the *implementation* — what clients never see, never include, and ideally never need to know about.

Think of it like a brick. A brick has a standard shape and structure. You can pick up any brick and use it in any building. Components have the same property — uniform structure, predictable rules, independently usable. The analogy in the book is: components are bricks, packages are walls, package groups are buildings. The whole physical hierarchy is built from this one unit.

One of the most practical rules: the very first line of your `.cpp` always includes its own header. Why? Because that's how you test that the header is self-sufficient. If the header is missing a dependency, the `.cpp` will fail to compile immediately. You get automatic enforcement with no extra tooling — just a one-line convention.

---

## Slide 9 — How C++ Programs Are Built

Before we go deeper into components, let me make sure everyone has the same picture of what actually happens when you build a C++ program — because this is key to understanding why physical design matters.

You start with a `.cpp` file. The preprocessor runs first and does something you might know but not fully appreciate: it *literally pastes* the entire contents of every `#include`d header, recursively, into your source file. What the compiler actually sees is one giant merged file. That giant file is called a translation unit.

Then the compiler parses that translation unit and produces an *object file* — machine code plus a table of symbols it defines and symbols it needs from somewhere else.

Then the linker takes all those object files plus any library archives you've specified, resolves all the "symbols I need" references, and produces your final executable or shared library.

This pipeline explains something important: every `#include` you add to a header is a *compilation tax* paid by every file that includes that header — and every file that includes those files. If `widget.h` includes `heavy_database.h`, then every file in your project that says `#include <widget.h>` is also compiling all of `heavy_database.h`, whether they want to or not.

This is why build times explode when headers get messy. It's not magic. It's just the preprocessor being very literal about what you asked it to do.

---

## Slide 10 — Object Files and Library Archives

Let's zoom in on what an object file actually is, because it changes how you think about link-time dependencies.

An object file has a few sections. The `.text` section is your compiled machine code — function bodies. The `.data` section holds initialized global and static data. The `.bss` section holds uninitialized static data. And there's a *symbol table* that lists what this object file provides and what it needs from somewhere else.

A static library — a `.a` file — is just an archive of object files. When the linker is resolving symbols, it searches through the archive looking for object files that provide what it needs.

Here's the critical thing to understand: the linker pulls in entire object files, not individual functions. If your `math.o` contains 50 functions and a client uses just one of them, they get all 50 linked in. This is why component granularity matters so much. If you shove unrelated things into the same component — say, geometry utilities and string parsing — every client who needs string parsing is also linking geometry code they'll never use.

Well-designed components have tightly related functionality. What you link in matches what you actually use. Use one thing from a component, you get a handful of related things — not a kitchen sink.

This also means: separate non-primitive operations into utility components. We'll see exactly what that means in Chapter 3. But keep this in mind — how big and focused your components are directly affects how much gets linked in unnecessarily.

---

## Slide 11 — Declarations vs. Definitions and the ODR

OK, let me make sure we're all solid on declarations versus definitions, because mixing these up causes a lot of subtle bugs and linker errors.

A *declaration* is a promise. It says "hey compiler, this thing exists. I'm not showing you the full thing right now, but trust me, it's somewhere." A forward declaration of a class is a declaration. An `extern int g_counter;` is a declaration. A function signature without a body is a declaration. You can declare something as many times as you want.

A *definition* is where the thing actually is. The class body with all its members is a definition. The function with its body is a definition. The `int g_counter = 0;` with actual storage is a definition. The One Definition Rule — the ODR — says there must be exactly one definition across your whole program. Not two, not zero. One.

Why does this matter for physical design? Because headers get included in multiple translation units. If you put a function definition — not just a declaration, but an actual body — in a header, and that header gets included in ten files, you now have ten definitions. Linker error, or worse, silent undefined behavior.

The rule is simple: declarations go in headers. Definitions go in `.cpp` files. The only exception is `inline` functions, where multiple identical definitions are explicitly allowed by the standard — that's what `inline` means.

Forward declarations are particularly powerful — if you only use a pointer or reference to a type in your header, you can forward-declare it with just `class Foo;` instead of including its full header. That one small technique can dramatically cut compile times.

---

## Slide 12 — Linkage in Practice

Let's make the linkage stuff concrete with the patterns you'll actually use day to day.

The most important rule: never define a non-inline function in a header. If you write `void helper() { /* ... */ }` in a `.h` file, every `.cpp` that includes that header will have its own copy of that function. Multiple definitions. That violates the ODR.

For implementation helpers that nobody outside your component should know about, use an anonymous namespace in the `.cpp` file. Anonymous namespace gives you *internal linkage* — the function is completely invisible outside that one translation unit. It's the C++ way of saying "this is private to this file, forever." Way better than old-school `static` for functions.

For things that genuinely need to be in headers — like small utility functions where you want them inlined — use the `inline` keyword. That tells the linker "yes there might be multiple definitions, but they're all identical, so just pick one."

The standard pattern is straightforward: put declarations in `.h`, put definitions in `.cpp`. Declare `void publicFunction();` in the header, define `void publicFunction() { /* implementation */ }` in the `.cpp`. Simple, clear, consistent.

This is boring stuff but it's the foundation. If you don't have a clean mental model of what belongs in `.h` versus `.cpp`, the rest of the physical design rules won't make sense.

---

## Slide 13 — Header Files: What Goes Where

Building on what we just covered — let's be explicit about the rule for what belongs in a header and what doesn't.

The question to ask for every piece of code you're about to put in a header is: "Does a client need this just to *use* my component?" If yes, it goes in the header. If no, it goes in the `.cpp` and stays invisible.

Class definitions — yes, headers. Clients need to know the shape of a type. `inline` functions — yes, headers. Template definitions — yes, headers (templates have to be fully visible for instantiation). Forward declarations, type aliases, function declarations — all headers.

What doesn't belong: function bodies that aren't `inline`. `using namespace` directives — we'll come back to why this is a hard rule, but briefly: it forces those names into every file that ever includes yours. Global variable definitions. Platform-specific conditionals that only make sense in one place. And really importantly: `#include` directives for things your *interface* doesn't actually need. If you only use a type in your `.cpp`, include its header only in the `.cpp`.

There's an interesting footnote here about private members. Even though clients can't access private members, they still appear in the header. And changing a private member's type forces every client to recompile — even if they never touch that private data. We'll come back to how you fix this in Chapter 3 with something called the opaque pointer idiom.

---

## Slide 14 — Include Guards

Include guards might seem like the most boring topic in this entire workshop but there's a specific convention the book recommends that's worth following, especially at scale.

Every header must have an include guard. The form is `#ifndef INCLUDED_<FILENAME_IN_UPPERCASE>`. So `my_widget.h` gets `#ifndef INCLUDED_MY_WIDGET`. The `INCLUDED_` prefix is deliberate — it avoids collision with other macros that might happen to have the same name.

The `#endif` at the bottom should always have a comment saying what it's closing. When you're deep in a 300-line header, you want to know what that `#endif` is for. `// INCLUDED_MY_WIDGET` — done.

Now here's something you might not have seen before: external include guards in your `.cpp` file. The idea is to wrap each `#include` in a guard too. `#ifndef INCLUDED_MY_WIDGET` / `#include <my_widget.h>` / `#endif`. If the guard is already defined — meaning the header was already processed — the preprocessor doesn't even open the file again.

At small scale, this saves nothing noticeable. At enterprise scale — thousands of headers, hundreds of translation units — skipping already-processed headers can noticeably cut build times. It's one of those practices that costs almost nothing but pays off when the codebase gets big.

---

## Slide 15 — The Four Component Properties

Now let's formalize what makes a component a component. The book defines four properties, and every component you write should satisfy all four.

**CP1:** The header is self-sufficient. It compiles correctly in isolation — you can include just that header and nothing else, and it will work. It doesn't secretly rely on something being included before it. This is the most important one.

**CP2:** The `.cpp` file includes its own header *first*, before anything else. This is the automatic enforcement mechanism for CP1. If your header has a missing dependency, the `.cpp` will fail to compile immediately because it includes the header before anything else. You catch the problem instantly.

**CP3:** Clients never directly include the `.cpp` file. Only the header is part of the public interface.

**CP4:** All dependencies are fully determined by `#include` chains. There are no hidden side channels, no order-of-inclusion tricks, no global state tricks. A tool can analyze the `#include` directives and produce a complete, correct dependency graph.

Why do these four matter together? Because they make every component truly self-contained and easy to analyze. Automated tools can find its dependencies. Build systems can compute exactly what needs to rebuild. Test drivers can link exactly what they need. These four properties are what turn a pile of files into a proper physical design.

---

## Slide 16 — Logical Relationships and Their Physical Cost

Here's something that trips up a lot of developers: the same logical relationship between two classes can have very different physical costs depending on *how* you express it.

If `Foo` stores a `Bar` by value as a member — `Bar d_bar;` — then `Foo`'s header must include `Bar`'s header, because the compiler needs to know the full size of `Bar` to know the size of `Foo`. Every client of `Foo` now also implicitly includes `Bar`'s header.

If `Foo` only stores a *pointer* to `Bar` — `Bar* d_bazPtr;` — you don't need the full definition of `Bar`. A forward declaration `class Bar;` is enough. The pointer is always the same size regardless of what it points to. You've cut the compile-time dependency for basically nothing.

Same logic for references in function parameters. If your method takes `const Baz&`, you only need a forward declaration of `Baz`.

This table on the slide maps the four kinds of logical relationships to their physical cost. "Uses in interface by value" is the most expensive — forces a full `#include`. "Uses in name only" is basically free — just a forward declaration.

The practical takeaway: prefer pointers and references over values in class members and function signatures — when it makes sense. Not always — sometimes you genuinely need the full type. But when you have a choice, this can cut your compile-time coupling a lot.

---

## Slide 17 — Level Numbers

This is one of my favourite concepts in the book because it's so simple and so powerful at the same time.

The idea: every component in your system can be assigned a *level number*.

Level 1 is a component that doesn't depend on anything internal — it only uses standard library stuff or third-party things, but nothing from within your own codebase. That's your foundation.

Level 2 depends on at least one Level 1 component. Nothing higher. Level 3 depends on Level 2 or lower. Level N depends on Level N-1 or lower.

Look at the diagram: `math_util` at Level 1. `date` at Level 2, using `math_util`. `calendar` at Level 3, using `date`. `calendar_cache` at Level 4, using `calendar`. Clean, hierarchical, testable.

Now here's the magic: if you *can* successfully assign level numbers to all your components, your dependency graph is acyclic. No cycles. If you *can't* — if you get stuck trying to figure out what level something is — that means you've found a cycle. The level numbering breaks down exactly where the design is broken.

And change impact becomes predictable. Change a Level 1 component — only Level 2 and above need to rebuild. Change a Level 4 component — nothing at Level 1, 2, or 3 is affected. When your level numbers are small and your dependency cones are narrow, incremental builds are fast.

---

## Slide 18 — The Depends-On Relation and Implied Dependencies

Let's get precise about what "depends on" means, because there's an important distinction that saves you from a common trap.

Component Y "depends on" component X if Y's header or Y's `.cpp` includes X's header. But the type of dependency matters enormously.

If Y's *header* includes X's header — that's an interface dependency. It propagates to all of Y's clients. They all implicitly include X too. Change X's header, and Y's clients recompile even if they never use X directly.

If only Y's *`.cpp`* includes X's header — that's an implementation dependency. Clients of Y are completely shielded. They don't even know X exists. This is the good case.

Whenever you can, move `#include` statements from the header to the `.cpp`. Use forward declarations instead of full includes. That's how you cut down compile-time coupling.

Now the trap: implied dependencies. If `A.h` includes `B.h`, and `B.h` includes `C.h`, then any file that includes `A.h` also indirectly gets `C.h`. You might start using things from `C.h` — not because you included it, but because something else pulled it in for you.

Then someday someone refactors `B.h` and removes the include of `C.h`. Your code, which was relying on that, suddenly breaks. You didn't even change anything — you just got burned by a dependency you didn't know you had.

The rule: always include what you directly use. Never rely on transitive includes as a guarantee. They're someone else's internal detail — not a promise to you.

---

## Slide 19 — Chapter 2: The 3-Level Physical Hierarchy

OK, we're moving into Chapter 2 now, and we're going to zoom out from individual components to the larger structures you organize them into.

At scale, components alone aren't granular enough. Hundreds of components with no grouping is chaos. So the book introduces two more levels of organization: packages and package groups.

A **package** is a directory of related components that all share a name prefix. All the date/time components live in the `bdlt` package — `bdlt_date.h`, `bdlt_calendar.h`, and so on. They all share the `bdlt` prefix and they all live in the same directory.

A **package group** is a collection of related packages that are developed and released together. The `bdl` group contains `bdlt`, `bdlb`, `bdls`, and more. This is the Unit of Release — the outermost thing you actually deploy.

The hierarchy on the slide shows you what this looks like in practice: `bdl` at the top, three packages inside, each with their components. It looks like a simple folder structure, but there's a critical rule that applies at every single level: **no cyclic dependencies**. Not between components within a package. Not between packages within a group. Not between groups.

The three levels in the table — component, package, package group — map to increasing scale: 100 lines, 20 components, 30 packages. The whole structure stays manageable because you apply the same rules at each level.

---

## Slide 20 — Unit of Release

Let's make sure we understand what a Unit of Release actually is and why it matters for how clients use your code.

A Unit of Release, or UOR, is the outermost thing you deploy as one chunk. When a client uses your library, they link the whole UOR — not individual packages, not individual components. They say `link libbdl.a` and they get everything inside it.

The internal structure — which package a component lives in, how packages are organized — that's an implementation detail. It helps *you* organize and test the code. Clients don't need to care about it.

Look at the build artifact: `libbdl.a` contains `bdlt_date.o`, `bdlt_calendar.o`, `bdlb_string.o`, `bdls_filesystemutil.o`, and everything else from every package. One archive, one version, one deployment.

Now, there are three ways a UOR can be deployed. As a static library (`.a`) — your client links it at build time, gets a bigger binary but no runtime dependencies. As a shared library (`.so` or `.dll`) — linked at runtime, smaller binary, but with ABI stability requirements. Or as source — the client compiles it themselves, maximum portability.

Good physical design has to support all three. If your design only works as a static library for some internal reason, you've painted yourself into a corner. And this point connects to something we'll see later: shared library APIs have the strictest insulation requirements, because changing a header forces a recompile of every consumer.

That wraps up the core concepts of how code is organized. Next we'll get into the naming rules and design conventions that make this whole structure actually usable by a team of engineers at scale.

---

## Slide 21 — Three-Way Naming Coherence

So we just talked about how code is organized into components, packages, and package groups. Now let's talk about how it's *named* — because this is where the whole approach really shows its value in day-to-day work.

The rule is what Lakos calls three-way naming coherence. Three names, one consistent identity. If your package is called `bdlt`, then your component is `bdlt_date`, your header is `bdlt_date.h`, and your class is `bdlt::Date`. The namespace matches the package. The filename matches the component. The class local name matches the filename suffix. They all derive from the same root.

Why does this matter? Look at the code example on the slide. You see `bdlt::Date d;` in unfamiliar code. You don't know this codebase at all. But from that one line you immediately know: the header is `bdlt_date.h`, it lives in the `bdlt` package, which is in the `bdl` group. You didn't need a search engine. You didn't need to ask a colleague. You worked it out from the name.

At small scale, this feels over-engineered. At enterprise scale — thousands of types across hundreds of packages — it's a superpower. And notice the rule doesn't say the prefix has to be a meaningful word. `bdlt` is opaque until you learn it. After a few weeks, your brain maps it to "Bloomberg Date/Time" automatically. The point is *uniqueness and predictability*, not readability of the prefix itself.

---

## Slide 22 — Key Naming Conventions

Following on from the three-way coherence, there are a few specific naming rules worth knowing because they each solve a real problem.

The `d_` prefix for instance data members and `s_` for static members — this looks like stylistic noise until you're in a large class with 20 methods and you need to know if you're reading local state or object state at a glance. In a long method body, `d_year` is instantly recognizable as "member of this object." `year` alone is ambiguous — could be a local, a parameter, or a member. The prefix removes the ambiguity without any extra context.

The global uniqueness rule for header filenames is more important than it sounds. When all your headers can coexist in one flat directory like `/usr/include/`, you don't need path context to find them. `#include <bdlt_date.h>` works from anywhere. No `../../../include/` nonsense. Tooling doesn't need to know the directory structure. Everything just works because the names are unique.

And there's the automated tooling angle. When naming follows a predictable pattern — when you can derive the header from the class name and vice versa — you can write tools that automate dependency analysis, code generation, and package management. Consistent naming is what makes tooling possible at scale.

---

## Slide 23 — The Most Important Rule You Probably Break

This slide has the rule that I think every single experienced C++ developer knows intellectually but still violates sometimes under time pressure. So I'm going to be very direct: **never put `using namespace` in a header file**. Not even `using std::vector`. Not `using std::string`. None of them.

Here's why this is a big deal. A `using namespace std;` in your header doesn't just affect your code. It spreads to every file that includes your header, and every file that includes *those* files. You've quietly pushed all of `std` into someone else's code without asking. They might have a class called `string` in their namespace. You just created a name collision they have no way to fix without changing your header.

And once it's there, it's almost impossible to remove. Every downstream file is depending on those names being available. You've created a permanent dependency on your implementation choice.

The fix is trivial: use `using` only inside function bodies, where it's scoped and contained. Or just type the full `std::vector` — it's five extra characters and it costs nothing.

Same logic applies to `<iostream>` in headers. If your interface just takes an `ostream&` reference, include `<iosfwd>` instead. That header has only forward declarations of the stream types — essentially free. You don't need to parse all of `<iostream>` just to declare that a function takes a stream parameter.

---

## Slide 24 — Standard Component Source Layout

OK so I want to show you what a properly formatted component looks like in full, because the book is quite specific about this and it's worth seeing all the pieces together.

The header starts with a file-mode comment — that little `// -*-C++-*-` thing is an Emacs hint but it also serves as a human signal that this is a C++ file. Then there's a comment block listing the classes in this component and a one-line description of each. This sounds like overkill but it's actually very useful — you can grep for it or read it as a quick API summary.

Then the include guard with `INCLUDED_` prefix. Then all the `#include` directives, each wrapped in an external guard. Then the namespace, the class definition, and the closing namespace comment. The `#endif` at the bottom with the guard name as a comment.

In the `.cpp`: external guard on the component's own header, then the component's own header included first (CP2), then other dependencies, then the implementation wrapped in `namespace pkg { }` with a closing comment.

Every single detail here is intentional. The file-mode comment, the class list, the external guards, the namespace comments — they all exist to help humans navigate at scale and to let automated tools parse the structure reliably. When you have a million lines of code, consistency of this kind is what separates maintainable from unmaintainable.

---

## Slide 25 — What a Package Looks Like

Now let's look at what a package actually looks like on disk, because there's more to it than just a directory of source files.

The most important thing on this slide is the `package/` subdirectory containing two metadata files: `.mem` and `.dep`.

The `.mem` file is a membership list — it says which components belong to this package. That's how the build system knows what to compile and what to ship.

The `.dep` file is the dependency declaration — it says which *other packages* this package is allowed to use. This is explicit, intentional, and enforced. If you write code in the `bdlt` package that `#include`s something from a package not listed in `bdlt.dep`, the build fails. Not a warning. Not a lint suggestion. A build failure.

That's the key insight here: design rules become real policy when they're written into metadata files and enforced by the build system. Without `.dep` files, the architecture rules are just guidelines that slowly erode. With them, violations are caught immediately and automatically on every build.

The `.t.cpp` test driver files — notice they live in the same directory as the component during development, but they're excluded from the deliverable. Tests are first-class citizens of the codebase, not afterthoughts tucked into a separate folder that nobody maintains.

---

## Slide 26 — Package Groups: The Unit of Release

Package groups are the top of the hierarchy and they follow the same rules as everything below — but at a larger scale.

A package group is a directory containing multiple packages, plus a `group/` subdirectory with two more metadata files: the group-level `.mem` listing the packages, and the group-level `.dep` declaring which other package *groups* this group is allowed to depend on.

Look at the structure on the slide: `bdl` contains `bdlt`, `bdlb`, and `bdls`. Three packages, each with their own components. The whole group compiles into one archive — `libbdl.a` — and that's what clients link against.

The rule is the same all the way up: no cyclic dependencies between groups. If `bdl` depends on `bsl`, then `bsl` cannot depend on `bdl`. Acyclic all the way to the top.

Bloomberg's real BDE library has dozens of these groups, hundreds of packages, thousands of components. The whole thing follows these rules. And because it does, any component in the system can be tested independently. Any package can be released independently if needed. The hierarchy is a real engineering guarantee, not just a naming convention.

---

## Slide 27 — The Most Important Consequence of Acyclic Design

We've been talking about acyclic dependencies as an architectural rule. Now let's make clear why it matters at the most practical level: testing.

The rule is simple but powerful: every component must be testable using only components at *strictly lower* levels. To test `math_util` at Level 1, you link nothing internal — just the standard library. To test `date` at Level 2, you link only `math_util`. To test `calendar` at Level 3, you link `math_util` and `date`.

This has a beautiful consequence: if your Level 1 tests pass and your Level 3 test fails, the bug must be in Level 2 or Level 3 code. You've already proven the foundation. You're not chasing a bug that might be anywhere in the entire system.

Now think about what happens with cycles. If A depends on B and B depends on A, you literally cannot link a test for A without also linking B. Your test for A is now also a test for B whether you want it to be or not. These aren't unit tests anymore — they're integration tests. They're slower, harder to set up, harder to diagnose, and harder to maintain. And eventually developers just stop writing them, because it's too painful.

This is why acyclic dependencies are non-negotiable. They're not a style preference. They're what you need to have real unit tests at all.

---

## Slide 28 — Thinking Physically

This slide is where Chapter 2 wraps up and we move into Chapter 3. Before we get into the specific techniques, I want to lock in the mindset shift.

The question most developers ask is "what class should I create?" Physical thinking asks a different question: "where does this code *physically live*?" Which level? Which package? Which component? What level number does this get?

Look at the code on the slide. You write `#include <calendar.h>`. Looks simple. But you're not just including calendar — you're also pulling in `date.h`, which pulls in `math_util.h`, which pulls in `<cstdint>`. You are now *bound* to all of those at compile time. Your recompile set just grew. Every client of every one of those headers will recompile if any of them change.

The habit you want to build is: every time you reach for `#include`, ask "does this go in the header or the `.cpp`?" And then ask "is there a lighter alternative — a forward declaration?" Most of the time there is.

Physical thinking is also about placement. When you create a new function, ask "am I placing this at the right level?" If your function only uses stdlib, it probably belongs at Level 1. If you're putting it at Level 7 inside a blob component, you're forcing Level 1 clients to link Level 7 code to get it. That's a design mistake that compounds over time.

This is the mindset that Chapter 3 is built around. Let's get into the specific techniques.

---

## Slide 29 — Primitive vs. Non-Primitive

Chapter 3 is the biggest and most practical chapter in the book. It's all about how you *factor* your design — how you decide what goes where. And the first decision is fundamental: primitive versus non-primitive functionality.

A primitive operation is one that *needs* direct access to private state to be implemented correctly or efficiently. For a `Polygon` class: `addVertex`, `area`, `vertexCount`. These genuinely need to see the internal data. They belong in the class.

A non-primitive operation is one you *could* implement using only the public interface. `isConvex` — you can figure out convexity just by querying vertices. `rotate` — you can rotate by calling `addVertex` with transformed coordinates. These don't need private access. They don't need to be in the class.

When you put non-primitives in the class, you make the class heavier. Every time you add a non-primitive, you add a potential dependency on something new. The class gets pulled to a higher level. Its clients pay for everything it depends on, even if they only need the primitive operations.

The fix is to move non-primitives to a separate utility component — `PolygonUtil`. Now clients who only need `area()` link `polygon.o`. Clients who also need `isConvex()` link `polygon.o` and `polygon_util.o`. The link-time footprint is proportional to actual use.

This sounds simple. It is simple. But consistently applying it is what keeps large codebases lean.

---

## Slide 30 — Why Separate Non-Primitive Functionality

Let me drive home why this separation is worth the discipline, because I've seen people resist it with "it's just one more method, what's the big deal?"

At small scale, it's no big deal. At large scale, every unnecessary method in a class is a reason for that class to depend on more things, be at a higher level, and force more recompilation on its clients. The effects multiply.

There are five concrete benefits and they're all on the slide.

First: smaller compile-time footprint. If `isConvex` needs some geometry math library and that method lives in `Polygon`, then every person who includes `polygon.h` also compiles that geometry math dependency — even if they never call `isConvex`.

Second: simpler, more stable core. Fewer methods in `Polygon` means fewer reasons for `Polygon`'s header to change. Every time `Polygon`'s header changes, every file that includes it recompiles. A stable header is a build-time asset.

Third: independent extensibility. You can add `PolygonUtil2` with more algorithms without touching `Polygon` at all. That's the Open/Closed Principle made physical.

Fourth: independent testing. `Polygon` has its own focused test suite. `PolygonUtil` has its own. Neither contaminates the other.

Fifth: proportional link-time footprint. You only link what you actually use.

The scale effect is the key message here. These benefits grow much faster than the codebase itself. A codebase with a hundred classes following this rule is dramatically easier to work with than one where every class has accumulated two decades of convenience methods.

---

## Slide 31 — When to Colocate Classes

The default rule is one class, one component. But there are specific situations where colocating two classes in the same component is justified, and I want to be clear about what those situations are — because "it seemed like a good idea" is not one of them.

The first legitimate reason is *friendship*. If class `NodeIterator` needs private access to class `Node` to work correctly, and you put them in separate components, you'd have to use public API everywhere — or have `Node` declare `NodeIterator` as a friend and include its header, which creates a dependency. Colocating them in one component solves this cleanly.

The second is *unavoidable mutual dependency*. If two classes genuinely reference each other's types in a way that can't be resolved with forward declarations, colocating is the solution. But be skeptical — most apparent bidirectional dependencies can be resolved with levelization.

The third is the "flea on an elephant" case. A tiny helper class that's tightly coupled to a large class and has no independent use. An iterator for a stack. It's so small and so tied to the stack that making it its own component adds overhead without benefit.

What's NOT a valid reason: "they're related," "they're small," "it's convenient," "I didn't want to make another file." Those are symptoms of not wanting to think about the design. Each class added to a component increases the dependency surface for all clients of that component. That's a real cost.

---

## Slide 32 — Cyclic Dependencies Are Fatal

This is the big one. Everything we've built up to this point is about enabling the absence of cycles. Now let's be explicit about why cycles are catastrophic.

The immediate effects: you cannot compile A without B, you cannot compile B without A. You cannot test A in isolation, you cannot test B in isolation. Level numbers are undefined for both — the graph is not a DAG. Any change to either header triggers the other's recompile.

These are annoying but manageable in a small system. The long-term effects at scale are what make cycles truly dangerous.

One cycle makes it feel normal. Others follow — why not? The precedent is there. Each new cycle makes the system harder to test, understand, and change. Unit testing gets replaced by integration testing because nothing can be tested alone. Build times grow. People stop writing tests because they're too slow and painful. Refactoring becomes too risky because you can't tell what will break.

The book uses the word "fatal" deliberately. Cycles are not a code smell. They are not something you fix when you have time. They destroy the entire foundation that the methodology is built on.

The rule is absolute: no cyclic dependencies. Not at the component level, not at the package level, not at the package group level. Any cycle must be fixed before it gets bigger.

---

## Slide 33 — Chapter 3: Your Toolbox — 7 Techniques

OK so now you're convinced that cycles are bad. But what do you do when you have one? This is where Chapter 3 really earns its place in the book. Lakos gives you seven concrete techniques for breaking cycles. Not theory — actual patterns with names and tradeoffs.

The table on the slide is your reference. Classic Levelization: cut the reverse dependency when one side doesn't truly need the other. Escalation: add a third component above both when they need to interact. Demotion: pull low-level code down from a bloated higher-level component. Opaque Pointers (PIMPL): hide implementation details so the header doesn't need heavy includes. Dumb Data: separate structure from semantics. Callbacks: let high-level code register behavior with low-level code. Protocol Class: define an abstract interface so the lower level doesn't need to know the concrete type.

There are also two supporting techniques mentioned: Manager Class (an orchestrator component above two lower ones) and Factoring (extract shared code into a new lower-level component that both depend on).

In practice, most real-world cycles are solved by the first two techniques alone — Classic Levelization and Escalation. PIMPL and Protocol Class solve specific structural problems that come up less often but are more powerful when they do.

Over the next several slides we'll walk through each one with code. Think of this as building a mental toolkit you can apply the next time you run into a cycle.

---

## Slide 34 — Technique 1: Classic Levelization

This is the simplest technique and often the most surprising one, because the solution is just to question an assumption you didn't realize you were making.

The classic example: Manager and Employee. These seem like they have a bidirectional relationship — Manager holds a list of Employees, and Employee knows its Manager. So you include each other's headers and you've got a cycle. Neither can be tested without the other.

But wait — does `Employee` *really* need to know about `Manager` at the level of its core data model? Think about it. An employee has a name, a salary, a job title. None of that fundamentally requires knowing who manages you. The "employee knows their manager" relationship is needed for *HR operations* — which belong at a higher level anyway.

So you cut the reverse dependency. `Manager` holds `Employee` objects. `Employee` knows nothing about `Manager`. The cycle is gone. `Employee` is now a clean Level 1 component you can test with nothing else linked.

The question to ask is: "Does this side truly need the other *at this level of abstraction*?" In most cases, one direction is just there out of habit — because in the real world the relationship goes both ways. In the physical design, you only need the direction that serves the actual operations at this level.

This is the most underused technique because it costs nothing — you just delete code — and it requires questioning a design assumption that felt obvious.

---

## Slide 35 — Technique 2: Escalation

Where Classic Levelization cuts a dependency, Escalation moves the interaction to a different level. These are complementary tools.

The scenario: `EventQueue` and `Event`. `EventQueue` obviously needs to hold `Event` objects. But imagine `Event` also has a reference back to its queue so it can re-enqueue itself. Now you've got a cycle — `EventQueue` includes `Event`, `Event` includes `EventQueue`.

Escalation says: don't try to make them talk directly. Build a `EventManager` component above both of them. `EventManager` knows about both `Event` and `EventQueue`. It creates events, puts them in queues, processes them, and handles re-queuing. `Event` knows nothing about queues. `EventQueue` knows about `Event` objects only as data.

The structure after escalation: `Event` at Level 1 (knows nothing about queues). `EventQueue` at Level 2 (holds `Event` objects). `EventManager` at Level 3 (knows both, handles the interaction).

Each level is independently testable. `Event` tests are tiny and fast. `EventQueue` tests work with mock events. `EventManager` tests exercise the full interaction.

The mental model: if two things need to interact, don't make them interact directly. Build the room above them where the conversation happens. The "knowledge" of how they work together belongs at a higher level than either of them.

---

## Slide 36 — Technique 3: Demotion

Demotion is the technique you apply when you have a bloated utility module — the "big ball of mud" component that accumulated functionality over years because it was convenient to put things there.

Imagine an `all_utils` component sitting at Level 5. It contains string hashing (conceptually Level 1 stuff), date parsing (Level 3), and report generation (genuinely Level 5). Because they're all in one component, they're all at Level 5. Any Level 1 component that needs string hashing has to link against a Level 5 component to get it. That's pulling in date parsing and report generation just to hash a string.

Demotion says: extract each capability and move it to its *natural* level. Pull the string hashing out into a `str_hash` component at Level 1. Pull date parsing into a `date_util` component at Level 3. Leave report generation in a component at Level 5.

After demotion, Level 1 code links only Level 1 components. Level 3 code links Level 3 and below. Everything links the minimum it needs.

The blob refactoring strategy in practice: don't try to split the whole thing at once. Start by identifying the lowest-level capability in the blob and demoting that first. Repeat until every remaining piece naturally belongs at its level.

This is the most useful technique for legacy codebases, because most older codebases have at least one of these utility blobs that just piled up over time.

---

## Slide 37 — Technique 4: Opaque Pointers (PIMPL)

PIMPL — Pointer to Implementation — is probably the most well-known technique from this area of C++ design. Let's walk through it carefully because the details matter.

The problem: your `Device` class has a `HeavyDriver` member by value in the header. Every file that includes `device.h` must also parse `heavy_driver.h` — with all its transitive includes. Maybe that drags in platform-specific headers, low-level driver APIs, or a massive third-party library. Every user of `Device` pays that compilation cost, whether they care about `HeavyDriver` at all.

The fix: forward-declare `DeviceImpl` in the header (just `class DeviceImpl;`), store a `unique_ptr<DeviceImpl>` instead of the heavy member, and move all the heavy stuff to `device.cpp`.

Now `device.h` only needs `<memory>` for `unique_ptr`. That's it. The compilation cost of `HeavyDriver` is isolated to the one file that implements `Device`. Every other file in the codebase just sees the slim header.

When you change `DeviceImpl`'s internals, only `device.cpp` needs to recompile. All clients of `Device` are completely unaffected.

The practical cost is one heap allocation per `Device` object. In most cases — long-lived objects, objects constructed once per request — this is completely negligible compared to the build-time savings.

---

## Slide 38 — PIMPL Gotchas and Trade-offs

PIMPL has one gotcha that catches people every single time they first use it, and I want to make sure you don't hit it.

When you store a `unique_ptr<DeviceImpl>` in your class, and `DeviceImpl` is an incomplete type in the header, `unique_ptr`'s destructor needs to call the deleter — which needs the complete type. If you write `~Device() = default;` in the header, the compiler tries to generate the destructor there, `DeviceImpl` is incomplete at that point, and you get an error.

The fix is simple but easy to forget: declare the destructor in the header without a body (`~Device();`), and *define* it in the `.cpp` as `Device::~Device() = default;`. By the time the `.cpp` is compiled, you've included `heavy_driver.h` and `DeviceImpl` is fully defined. The compiler can generate the destructor correctly.

The same rule applies to the move constructor and move assignment operator. If your class has a `unique_ptr` member, you need to explicitly declare and define all five special member functions that touch the impl — otherwise the compiler tries to generate them in the header where the type is incomplete.

This sounds like a lot of boilerplate and it is. But it's the boring kind you write once and never touch again. The trade-off table on the slide sums it up: clients don't recompile when the implementation changes, ABI stability, smaller headers — in exchange for one heap allocation per object and no inlining through the impl pointer.

For shared library public APIs, PIMPL is essentially mandatory. Without it, any private member change forces all consumers to recompile. That defeats the whole purpose of a shared library.

---

## Slide 39 — Technique 5: Dumb Data

Dumb Data is a subtle technique but once you see the pattern, you'll recognize it everywhere.

The scenario: you have a low-level `Node` type that needs to carry some kind of type tag so that higher-level code can decide how to process it. The naive approach puts an enum in `Node` that describes all the node types — SOURCE, SINK, INTERMEDIATE, etc. But those enum values only have meaning in the context of a specific graph algorithm. `Node` now has knowledge of the semantics of the system that uses it.

Even worse, if those node type definitions come from a higher-level component, `Node` now depends on that higher-level component. That's an upward dependency — a cycle.

The Dumb Data solution: `Node` stores a raw integer tag with no interpretation. Just `int d_type;`. The `Node` struct has no idea what the tag means. It doesn't need to.

The higher-level `Graph` component has the switch statement that interprets the tag. `Graph` knows about NODE_TYPE_SOURCE and NODE_TYPE_SINK. `Node` doesn't know and doesn't care.

This separation is incredibly flexible. The same `Node` data structure can be used by multiple different algorithms at different levels, each assigning their own meaning to the tag. The data carries structure; the semantics are applied by whoever uses the data.

You see this pattern everywhere in C: protocol headers are full of dumb data structs. The bytes mean nothing without the layer above that knows the protocol. Physical design follows the same principle.

---

## Slide 40 — Technique 6: Callbacks

The last technique before we hit the most powerful one — protocol class — is callbacks. And this one solves a very common problem: a lower-level component needs to "talk back" to higher-level code without knowing anything about that higher-level code.

Think of a logging component at Level 2. It processes data and needs to report errors. But to who? In a big system, the error handler could be a file logger, a network logger, a UI notification, anything. If `Logger` includes any of those, it's pulled to a higher level and loses its reusability.

The callback solution: `Logger` defines a callback type — a function signature or an abstract interface. Higher-level code registers its handler at setup time. When `Logger` needs to report something, it calls the registered callback. It has zero knowledge of who registered it or what they do with the notification.

The four flavors on the slide cover the spectrum. Function pointer plus `void*` userdata: maximum simplicity, C-compatible, zero overhead. Plain function pointer without userdata: even simpler, no state. `std::function`: modern, captures lambdas, slightly heavier. Protocol class (abstract interface): most powerful, supports full dependency injection.

The rule of thumb: use the simplest one that satisfies your requirement. If you're writing a C-compatible library, use function pointers. If you need to capture state and lambda syntax is acceptable, `std::function`. If you need full polymorphism and testability with mock objects, protocol class — which is exactly what we're going to cover next.

---

## Slide 41 — Protocol Callbacks: The Protocol Form

OK so we teased it at the end of the last slide — now let's go deep on the protocol class form of callbacks, because this is the most powerful pattern in the entire toolkit.

Let's use a concrete example from the slide. You have a `Serializer` at Level 1. It's a pure abstract class — just `virtual void write(int)` and `virtual void write(const string&)`, a virtual destructor, nothing else. No data. No implementation. Just a contract.

At Level 2, you have `Date`. `Date::serialize` takes a `Serializer&` and calls `s.write(d_year)`, `s.write(d_month)`, `s.write(d_day)`. That's it. `Date` has zero knowledge of JSON, binary encoding, XML, anything. It just calls the abstract interface.

At Level 3 and above, you have `JsonSerializer : public Serializer` and `BinarySerializer : public Serializer`. These know how to write things in their format. At the application level, you create a `JsonSerializer`, create a `Date`, and call `d.serialize(js)`. Everything wires together at the top.

The critical insight is which direction the arrows point. `Date` depends on `Serializer` — which is abstract and at Level 1. `Date` does *not* depend on `JsonSerializer` — which is concrete and at a higher level. Lower-level code depends only on abstractions. Higher-level code provides the concrete implementations.

You want to add `XmlSerializer` tomorrow? Go ahead. Add a new file at Level 3. Touch nothing. `Date` doesn't recompile. `Serializer` doesn't recompile. You added capability to the system by adding code, not by changing code. That's the dream.

---

## Slide 42 — Technique 7: Protocol Class

Now let's formalize the full Protocol Class technique, because what we just saw with `Serializer` generalizes to everything.

The slide uses `DataStore` and `PersistenceAdapter`. `PersistenceAdapter` is the pure abstract interface at Level 1 — `save`, `load`, virtual destructor. `DataStore` at Level 2 holds a reference to `PersistenceAdapter` and delegates all persistence operations to it. `DataStore`'s header includes only `PersistenceAdapter`'s header. Zero knowledge of SQLite, files, or memory.

Then at Level 3 and above, you have `MemoryPersistence`, `FilePersistence`, `SqlitePersistence` — each in their own component, each implementing the protocol. The application wires them together at the top.

What can you do with this? You can swap the entire persistence backend by changing one line at the application level. You can write `DataStore` code before any backend exists — just mock it out with a `MemoryPersistence` stub. You can test `DataStore` without touching a disk or a database.

This is what people mean by "depending on abstractions, not concretions." The protocol class is the mechanism that makes that principle physically real. Without the explicit level separation, it's just good intentions. With it, it's a build-time guarantee.

---

## Slide 43 — Protocol Class Benefits: Swap Without Recompiling

Let me show you what this looks like in practice, because the slide has a code example that really drives the point home.

For tests: you create a `MemoryPersistence`, inject it into `DataStore`, and your tests run in microseconds. No disk. No network. No setup. No teardown. Just in-memory maps.

For production: you create a `FilePersistence` with your path, inject it, done. Same `DataStore`. Same tests already verified it works. The `DataStore` code didn't change — not even a recompile.

Then a year later the team decides to migrate to SQLite. You write `SqlitePersistence`. You change the application wiring. `DataStore` doesn't recompile. All your existing tests still pass. The migration touches exactly the code that changed.

And here's the thing that makes this really powerful for large teams: if you have 10,000 tests that each create a `DataStore` with `MemoryPersistence`, that test suite might run in under a minute. The same tests with `FilePersistence` could take hours. Protocol classes are not just about design aesthetics — they are the mechanism that makes fast test suites *possible* at scale.

In OOP textbooks this is called the Strategy Pattern. In physical design terms, it's just a protocol class at Level 1 with concretions at higher levels. Same idea, but now enforced by the build system, not just by developer discipline.

---

## Slide 44 — Avoiding Excessive Link-Time Dependencies

We've covered the seven techniques now. Before we move on to the broader architectural patterns, let's talk about a specific antipattern that comes from not applying the primitive/non-primitive separation consistently over time. The `Date` class that got fat.

You start with a clean `Date` class. It has `day()`, `month()`, `year()`, `operator+` for date arithmetic. Perfect — all primitive, all low level.

Then someone needs to check if a date is a business day. They're already in the `Date` class for something else, so they add `isBusinessDay(const Calendar& cal)`. Now `Date` requires `Calendar`. Every file that includes `date.h` now also has to compile `calendar.h`. Users who never call `isBusinessDay` pay this cost anyway.

Then someone adds `format(const string& pattern)`. Now `Date` needs a formatting engine. Then a static `parse` method. Now it needs a parser library. A year later, `Date` is at Level 7 and every simple program that needs a date has to link the entire finance stack.

The fix is exactly the same one we covered with `Polygon`: move `isBusinessDay` to `DateUtil`, a separate component. Move `format` and `parse` to `DateUtil` as well. `Date` stays at Level 1 with pure date arithmetic. Clients who only need a date value link one tiny component. Clients who need business day logic link `DateUtil` too. The link-time footprint is proportional to actual usage.

The question to ask about every method on every class: "Should every single client of this class be forced to link everything this method requires?" If the answer is no for even a few users, the method belongs in a utility.

---

## Slide 45 — Lateral vs. Layered Architectures

This slide puts a name and a visual on two architectural patterns you've probably seen in the wild without having names for them.

A layered architecture looks like a tower. Level 1 at the bottom, Level 5 at the top. Each component depends only on things below it. When you change something at Level 2, only the components at Level 3 and above need to consider recompiling. The damage is bounded and predictable.

A lateral architecture looks like a web. Every component depends on every other component at roughly the same "level." Nobody can assign level numbers because there aren't any — it's one big tangle. When you change anything, everything might rebuild. When you modify component A, you have no idea what else might break.

The tower analogy in the slide is exactly right. With a tower, you can inspect, repair, or replace a single floor. The floors below it are unaffected. The floors above need to account for the change, but they're limited in number. With a spider web, cutting one strand shifts everything else. There's no safe place to make a change.

Here's the scary thing: lateral architectures don't start that way. They start as decent systems that slowly pick up "convenience dependencies" one at a time. Developer A needs one function from module B, so they add an include. Developer B sees that and adds one back. Neither was obviously wrong. A year later, everything depends on everything and CCD has doubled.

The only defense is following the acyclic rule consistently and tracking CCD regularly. Which brings us to the next slide.

---

## Slide 46 — CCD: Measuring Architectural Health

CCD stands for Cumulative Component Dependency. This is a number you can track to see whether your architecture is getting better or worse over time.

Here's how you compute it. For each component in your system, count how many other components it depends on, directly or indirectly — including itself. Then sum all those counts across the whole system. That total is the CCD.

The slide walks through a concrete example with 5 components in a perfect chain: A→B→C→D→E. Component A depends only on itself: count 1. Component B depends on itself and A: count 2. C depends on B, A, itself: count 3. And so on up to E which depends on all 5: count 5. Total CCD = 1+2+3+4+5 = 15.

Now take those same 5 components and make them all depend on each other — the lateral antipattern. Every component depends on all 5. Each count is 5. Total CCD = 25. Same 5 components, 67% worse CCD, just because of how they're organized.

At scale, this difference becomes quadratic. 100 perfectly layered components: CCD is around 5050. 100 fully lateral components: CCD is 10,000. Same number of components, double the build cost and coupling complexity.

The practical use: compute CCD before and after a refactoring. If it went down, your refactoring genuinely improved the architecture. If it went up, you made things more coupled — even if the code looks cleaner. CCD doesn't lie. Track it weekly. If it grows faster than your line count, you're accumulating architecture debt faster than you're adding features.

---

## Slide 47 — Avoiding Inappropriate Physical Dependencies

This slide is about a specific class of mistake that's very easy to make when you're working under time pressure: letting application-specific decisions leak into reusable library components.

The example is `UserRepository` with a `MySqlConnection` member. That one decision — choosing a concrete database class — means every single user of your library must have MySQL installed. Even if they're building a CLI tool that never touches a database. Even if they're on a platform where MySQL isn't available. You've forced a technology choice on everyone.

The fix is to make `UserRepository` depend on a `UserPersistence` abstract interface — a protocol class — and put the `MySqlUserPersistence` implementation in a separate, higher-level component. The library becomes policy-free. It provides mechanism. It doesn't decide which database you use.

There's an antipattern catalog on the slide worth remembering. `#ifdef BUYSIDE_ONLY` inside library code — that's domain policy embedded in what should be neutral mechanism. Application-specific logic in reusable components. Accessing global state from library code. Heavy platform headers in public interfaces.

Here's a quick gut check. Ask yourself: "Could I use this component in a completely different application, for a completely different company, on a completely different platform?" If the answer is no, it has an inappropriate dependency that needs to be moved upward or extracted behind an abstraction.

Library code should be a set of tools that don't care who picks them up or what they're used for.

---

## Slide 48 — Unnecessary Compile-Time Dependencies

Here's a fact that surprises a lot of developers when they first hear it: in C++, private member variables are fully visible in the header file. The compiler sees them. Any file that includes your header must parse the types of your private members, even though it can never access them.

This means changing a private member forces all clients to recompile. It's not just public interface changes that cause cascading rebuilds — any change to your class's private section does too.

Look at the example on the slide. A `Widget` class with `LargeDatabase d_db`, `ComplexCache d_cache`, and `NetworkConn d_conn` as private members. All of those headers need to be parsed by every client. If any of them changes — even internally — everything that includes `widget.h` must recompile.

The fix is exactly what we covered with PIMPL: forward-declare `struct Impl`, store a `unique_ptr<Impl>`. Now `widget.h` only needs `<memory>`. All the heavy headers move to `widget.cpp`. Private members are completely invisible to clients — not just linguistically inaccessible, but literally invisible to the compiler.

There's an important distinction here that's easy to mix up. OOP encapsulation protects you at the *language level* — other code can't call your private methods. Physical insulation protects you at the *compiler level* — other translation units don't even know what's in your implementation. Both matter. They're completely different mechanisms.

In a large codebase — say 10,000 files — adding one heavy private member to a widely-used class can turn a 30-second incremental build into a 20-minute wall-clock rebuild. PIMPL doesn't just look cleaner. It saves real developer time, every single day.

---

## Slide 49 — Three Approaches to Total Insulation

At this point you know two of the three insulation techniques from our earlier discussions. This slide puts all three side by side so you can choose the right tool for the situation.

The first is the **Protocol Class** — a pure abstract interface with virtual functions. Best when you need runtime polymorphism and multiple interchangeable implementations. The trade-off is virtual dispatch overhead and having to inherit from the interface. But you get full dependency inversion — the caller only knows the abstraction, never the concrete type.

The second is **PIMPL** — the concrete wrapper. Best when you're wrapping something you can't change or when you need to hide a heavy implementation behind a stable, slim header. The trade-off is that you have to forward all your API calls through the impl pointer. But the client's compiler never sees the impl at all.

The third is a **procedural C-compatible interface** — `extern "C"` functions that create, use, and destroy objects through opaque `void*` pointers. This is the most extreme form. Maximum ABI stability. Works across C, Python, Java, any language with a C FFI. The trade-off is it's verbose and you manage resources manually.

All three serve the same fundamental goal: the client's compiler should not see the implementation details. The choice depends on what you need — polymorphism, stability, cross-language compatibility, or some combination.

And they compose. A protocol class can be implemented using PIMPL internally. A C procedural interface can delegate to a protocol class. Use what the situation requires, combine when you need to.

---

## Slide 50 — When to Insulate and When Not To

I want to be direct about this because over-engineering is a real risk. Not everything needs PIMPL. Not everything needs a protocol class. The question is always: what are the costs and benefits for this specific type in this specific context?

The table on the slide lays this out cleanly. Short-lived value types like `Point` or `Rect` — don't insulate. They're constructed millions of times per second in some contexts. The heap allocation from PIMPL would be catastrophic. Also, value types rarely have implementations that change.

Long-lived objects with complex implementations — yes, insulate. The heap allocation is a one-time cost per object lifetime. The build-time savings from hiding a heavy implementation behind a slim header are worth it.

Shared library public APIs — insulate, always, full stop. This is not optional. Without insulation, any private member change requires all consumers to recompile. That's what "binary incompatible" means, and it breaks everyone who uses your shared library without rebuilding it.

Performance-critical types that get constructed in hot paths — don't insulate. Profile first, but generally the indirect dispatch and heap allocation are unacceptable in tight loops.

Third-party library wrapping — almost always yes. You want one wrapper component as the single point of contact with the external library. If you need to swap the library, you touch one file.

The underlying principle is: if your header is heavy AND the implementation is likely to change, insulate. If your type is simple, stable, and used in hot paths, don't pay the overhead. Match the engineering to the actual problem.

---

## Slide 51 — Designing a Real Subsystem: The Date Example

Let me show you what everything we've learned looks like when you apply it to a real subsystem design. This example is directly from Bloomberg's BDE library — it's not hypothetical.

First, what the bad design looks like. One `Date` class with `isBusinessDay(const Calendar&)`, `format(const string&)`, and a static `parse`. Three methods. Three dependencies. Every single user of `Date` in the entire codebase now needs `Calendar`, a formatting engine, and a parser library linked in — whether they use those features or not.

Now the good design. You levelized it into a hierarchy of five components:

`bdlt_date` at Level 1 — just a date value type. Day, month, year, arithmetic operators. Zero external dependencies. Lightning fast to compile, trivial to test.

`bdlt_dateutil` at Level 2 — formatting and parsing utilities. Depends on `bdlt_date` and string machinery. Can be tested with just `bdlt_date`.

`bdlt_calendar` at Level 3 — calendar data structures. Knows about dates, knows about what days are in the calendar. Can be tested with `bdlt_date` and `bdlt_dateutil`.

`bdlt_calendarutil` at Level 4 — business day calculations. Needs the calendar data. Tested with everything below.

`bdlt_calendarcache` at Level 5 — performance-optimized caching layer for calendar lookups. Most people never need this. It exists for high-frequency trading contexts where even the calendar lookup latency matters.

The beauty is proportional linking. A simple program that just needs to store a date links one tiny object file. A program that needs business day validation links up through Level 4. Only the high-performance trading system links Level 5. Every client pays exactly what they use.

---

## Slide 52 — The Component as the Unit of Design

This slide is a mindset shift, and it's one of the most important things to take away from this whole workshop.

When most developers are asked to design a system, they think in terms of classes. "I need a `Calendar` class with formatting, parsing, business day methods, and caching." That thinking leads directly to the monolithic `Date` we just saw.

The component-based physical design mindset asks a different set of questions. Not "what class?" but "what component, at what level?" The design conversation sounds different: "I need `bdlt_calendar` as a pure data component, `bdlt_calendarutil` for the algorithms, and I want them independently testable so I can verify the data structure separately from the business logic."

Look at the contrast on the slide. 50 classes, one cycle: cannot be tested, cannot be reused, maintenance nightmare because everything is entangled. 200 components, zero cycles: every part independently verifiable, independently releasable, maintenance is proportional to the part you changed.

The component is the atom of your software system. Chemistry is built from atoms that can't be split without losing their chemical properties. Software at scale is built from components that can't be split without losing their testability and reusability properties.

The component is also the unit at all three levels of the development process. You design one component at a time — well-defined responsibility, well-defined dependencies. You test one component at a time — its test driver links only lower-level components. You release a collection of components as a package group — a Unit of Release.

---

## Slide 53 — Advanced: Subpackages and Subordinate Components

This is an advanced topic you'll encounter when you're working with large, mature packages, or when you're building one. I want to mention it so it's not a surprise.

Subpackages are an organizational tool for when a single package gets large enough that the internal structure needs management. You can subdivide it into subpackages — but from the outside, clients still see the package-level interface. The prefix convention still governs everything. The acyclic rule still applies. It's just an internal organizational layer.

Subordinate components are more interesting. Sometimes a component needs a helper that is not part of the public API at all. It's used internally by the implementation but clients should never see or include it. You create a subordinate component — `bdlt_date_impl.h` and `bdlt_date_impl.cpp` — which is compiled but not installed as part of the public deliverable.

There's also the component-private class pattern on the slide. If a helper type is needed only inside one component, you can forward-declare it in the header so the compiler knows the name exists, then give it a full definition only in the `.cpp`. Clients see the name when they compile the header, but they can never instantiate it or use it. The implementation detail is completely private to the component.

The important point is that none of these advanced features change the fundamental rules. The package prefix convention applies to subordinate components too. The acyclic dependency rule applies at every level. The testability requirement holds. These are just ways to manage complexity within the system while keeping all the invariants.

---

## Slide 54 — Integrating Code That Doesn't Follow the Methodology

Reality check: you will always have to work with code that doesn't follow these rules. Legacy systems, open-source libraries, third-party SDKs — they weren't written with Lakos's conventions and you can't rewrite them. What do you do?

You wrap them. You create one conforming component that presents a clean interface to your codebase and hides the third-party code completely.

The example on the slide is `thirdparty_json_wrapper.h`. The header exposes only your clean types and methods: `JsonDocument`, `parse`, `getString`. No mention of `nlohmann::json` anywhere in the header. Clients who include this header have no idea what JSON library is being used under the hood.

Then in `thirdparty_json_wrapper.cpp`, you include your own header first, then you include `<nlohmann/json.hpp>` — which is now completely hidden from all your clients. The third-party header never escapes beyond this one `.cpp`.

The payoff comes when the team decides to switch from `nlohmann::json` to `RapidJSON` for performance. You change exactly one file. Every other component in your codebase is completely unaffected. Without the wrapper, that migration would touch every file that ever did anything with JSON.

For legacy code, the same logic applies. Don't try to clean up the legacy system directly — that's risky and expensive. Build a wrapper component at the boundary. Introduce your naming conventions at that boundary. Over time, gradually extract well-defined pieces of the legacy system into proper conforming components, and slowly replace the legacy code from the outside in. That's the strangler pattern, applied at the physical design level.

---

## Slide 55 — From Development to Deployment

Let me zoom out for a moment and talk about what this methodology enables at the deployment level, because everything we've built up connects to how software actually gets shipped.

Remember the global uniqueness rule for header names? That rule does something big at deployment time. If every header in your entire enterprise has a unique name, you can install all of them into a single flat directory like `/usr/include`. No path management. No include path configuration. No `../../third_party/include/` nonsense. Just `#include <bdlt_date.h>` and it works from anywhere.

It also makes automated tooling easier. If the name tells you the location, tools can find any header without knowing your directory structure. You can grep the entire enterprise codebase for a component name and get exactly one match. Renaming a component is a single search-and-replace.

The three deployment modes in the table — static, shared, source — are all viable with this methodology, and that matters. Static `.a` files: link everything in, larger binary, zero runtime binding. Shared `.so` or `.dll`: smaller binary, multiple programs share one copy, but ABI stability is critical. Source distribution: maximum portability, consumers compile it themselves.

The key point is that good physical design must support all three. If your design only works as a static library because of some internal linkage trick, you've eliminated options you might need in the future. Keep all three doors open.

And here's a nice unifying insight: a codebase where every component is independently testable is also a codebase where every component is independently deployable. Those two properties come from the same root: acyclic, hierarchical design.

---

## Slide 56 — Metadata: Capturing Dependencies Explicitly

We've talked about the `.dep` and `.mem` files throughout the workshop. This slide brings them together and explains why they matter as a category.

Metadata is how you take design rules that live in people's heads and turn them into policy that lives in the build system. Without metadata files, your architectural rules are suggestions that slowly erode. With metadata files, they are constraints that the build system enforces on every single build.

The table on the slide shows four types. Membership metadata — which components belong to this package. Dependency metadata — which other packages this package is allowed to use. Build requirements — compiler flags, platform constraints. Policy metadata — licensing, ownership.

The `.dep` file example is the most important. `bdlt.dep` says `bdlt` may depend on `bdlb` and `bslstl`. It must NOT depend on any application-level package. If a developer inside the `bdlt` package writes an `#include` that pulls in something not listed in `bdlt.dep`, the build fails. Immediately. Automatically. Before any code review, before any CI pipeline, right there on the developer's machine.

Think about what that means. You can hire a new developer, give them access to the `bdlt` package, and they physically cannot introduce an unauthorized dependency. Not because you told them not to. Not because there's a lint warning. Because the build system won't let them.

Metadata transforms architecture from aspiration into enforcement. That's the difference between a codebase that stays clean over years and one that gradually accumulates architectural debt nobody planned for.

---

## Slide 57 — How to Diagnose Physical Design Problems

OK, we're in the final stretch. Let's talk about how to actually use this knowledge when something goes wrong in your codebase. Because you're going to inherit code that has problems, and you need a systematic way to find them.

The slide gives you five questions to work through. Work through them in order.

First: "Is there a cycle here?" Try to assign level numbers. If you can't assign a level number to some component because it's part of a mutual dependency, you found a cycle. That's your first thing to fix.

Second: "Can I test this in isolation?" Write the test driver. Try to link it with only lower-level components. If it won't link without pulling in something at the same level or above, you have a hidden dependency. Find the `#include` causing it and decide if it belongs in the header or if you can move it to the `.cpp`.

Third: "Why is the build so slow?" Run `cpp -M file.cpp` to see all transitive includes. You'll probably be surprised at how many files get pulled in. That's your CCD problem made visible. Find the heaviest headers and apply PIMPL or forward declarations.

Fourth: "Why can't I reuse this?" It means the component has non-primitive functionality tangled in with the core, or inappropriate dependencies at the wrong level. Extract and demote.

Fifth: "Why did changing X break Y?" That's a transitive include chain. A change in a private implementation somewhere propagated to a client that shouldn't have cared. The fix is forward declarations, PIMPL, or moving includes to `.cpp`.

The tooling list on the slide is your practical kit: `cpp -M`, `nm -C`, `bde_verify`, `iwyu`. They're free and they're already on your system.

---

## Slide 58 — The Three Golden Rules

Everything in this workshop — all seven techniques, all the naming rules, all the packaging conventions — ultimately reduces to three rules. If you remember nothing else, remember these.

**Rule 1: Acyclic Dependencies.** No cycles in the physical dependency graph, at any level. Not between components, not between packages, not between package groups. If you can't assign a level number to a component, you have a violation. Fix it.

**Rule 2: Hierarchical Testability.** Every component can be tested using only lower-level components. If your test driver requires something at the same level or above, your design is wrong. The test exposes the design problem. Fix the design, not the test.

**Rule 3: Minimal Coupling.** Expose only what clients need. Hide everything else in the `.cpp`. Track CCD. Apply PIMPL or protocol classes when headers become heavy. The goal is that changing the implementation of any component should affect as few clients as possible.

Look at the diagnostic table on the slide. These three rules cover every problem we talked about today. Cycle? Use one of the seven levelization techniques. Can't test? Find and cut the hidden dependency. Build too slow? Reduce header weight, reduce CCD. Can't reuse? Separate non-primitive functionality. ABI breaks? Apply insulation.

And here's the thing that's easy to miss: these rules don't work if you apply them inconsistently. One cycle makes the pattern feel normal and others follow. One heavy header that nobody bothers to fix signals that the standard doesn't matter. The rules must be applied consistently and enforced by tooling. That's not being too strict — it's the bare minimum for software that needs to last for years.

---

## Slide 59 — Design Rules Checklist

This slide is your cheat sheet. Print it, put it on your monitor, add it to your code review template. These are the questions you should be asking on every PR that touches component design.

The four Component Properties, CP1 through CP4, are the baseline. CP1: does the header compile standalone? CP2: does the `.cpp` include its own header first? CP3: nothing exported from `.cpp` that shouldn't be? CP4: are all dependencies visible from the `#include` chains? These are quick mechanical checks.

The Design Rules, DR-1 through DR-8, are the ones most people violate without realizing it. DR-1 is the big one — no cycles. DR-5 and DR-6 are the ones most often violated by habit — no `using namespace` in headers. DR-8 surprises people — use `<iosfwd>` not `<iostream>` in headers when you only need a stream reference.

The code review questions at the bottom are the conversational ones. "Does this new include belong in the header or the `.cpp`?" — ask this every single time someone adds an include. "Is this operation primitive or non-primitive?" — ask this every time someone adds a method to a class. "Does this create a cycle? What is the level number?" — assign level numbers explicitly so you know.

The checklist is most powerful when it becomes a team habit, not a personal one. A team that consistently asks these questions in every code review will build a codebase that remains healthy over years. A team that skips them will end up with a big ball of mud that nobody can safely modify.

---

## Slide 60 — Summary: What We Covered

We've been through a lot today. Let me bring it all together.

Chapter 0 was about why this matters. Software either accumulates as capital — reusable, testable, maintainable — or it becomes debt that slows everything down. Physical design determines which one you end up with. It matters just as much as logical design, but it gets far less attention.

Chapter 1 was the foundation. The component is the atom — the `.h`/`.cpp` pair. You learned the four component properties, how the build pipeline works, what declarations and definitions mean, how linkage works, and what level numbers are.

Chapter 2 was the structure. Components aggregate into packages, packages into package groups. The three-way naming coherence makes everything self-documenting. Metadata files make architectural rules enforceable. Hierarchical testability is the goal that proves you got it right.

Chapter 3 was the practice. Seven techniques for breaking cycles and managing dependencies. CCD as a quantitative health metric. Insulation patterns for hiding implementation details. The date subsystem as a real-world example of all of it working together.

The philosophy in one sentence: design your physical structure so that every component can be compiled, tested, and deployed independently — and everything above it benefits automatically.

Before you leave, five things to do Monday morning. Try to assign level numbers to your main components. Run `cpp -M` on a header you think is lightweight — you'll probably be surprised. Find one non-primitive operation in a core class and move it to a utility. Check if any of your tests require components you didn't expect to need. Find one `using namespace` in a header and remove it.

These aren't heroic refactoring projects. They're small, safe improvements you can make immediately. Start there. The rest follows.

Thank you for your time today. This investment pays back every single day for the lifetime of the codebase.

---
