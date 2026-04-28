// levelization_demo.cpp                                              -*-C++-*-
//
// Combined demonstration of all levelization techniques from Chapter 3.
// This is a self-contained executable that illustrates the concepts
// without needing to link against other demo libraries.
//
// Techniques covered:
//   1. Classic levelization (§3.5.1)
//   2. Escalation (§3.5.2)
//   3. Demotion (§3.5.3)
//   4. Opaque pointer (§3.5.4) — see opaque_pointer/ for full demo
//   5. Dumb data (§3.5.5)      — see dumb_data/ for full demo
//   6. Callbacks (§3.5.6-7)   — see callbacks/ for full demo
//   7. CCD analysis (§3.7.4)

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// TECHNIQUE 1: Classic Levelization
// BEFORE: Manager ←→ Employee (cycle)
// AFTER:  Manager → Employee (acyclic)
// ============================================================

namespace classic_levelization {

// Level 1: Employee — knows nothing about Manager
class Employee {
    int         d_id;
    std::string d_name;
    double      d_salary;

  public:
    Employee(int id, const std::string& name, double salary)
    : d_id(id), d_name(name), d_salary(salary) {}

    int           id()     const { return d_id; }
    const std::string& name()   const { return d_name; }
    double        salary() const { return d_salary; }
};

// Level 2: Manager — knows about Employee (one-way dependency)
// Manager does NOT have Employee knowing about Manager.
class Manager {
    std::string             d_name;
    std::vector<Employee*>  d_reports;  // Manager has employees; Employee doesn't have Manager

  public:
    explicit Manager(const std::string& name) : d_name(name) {}

    void addReport(Employee* emp) { d_reports.push_back(emp); }

    double totalPayroll() const {
        double total = 0.0;
        for (const auto* e : d_reports) total += e->salary();
        return total;
    }

    int reportCount() const { return static_cast<int>(d_reports.size()); }
    const std::string& name() const { return d_name; }
};

static void demo()
{
    std::cout << "--- Classic Levelization: Manager → Employee ---\n";
    Employee e1(1, "Alice", 80000);
    Employee e2(2, "Bob",   75000);
    Employee e3(3, "Carol", 90000);

    Manager mgr("Dave");
    mgr.addReport(&e1);
    mgr.addReport(&e2);
    mgr.addReport(&e3);

    std::cout << "  Manager: " << mgr.name() << "\n";
    std::cout << "  Reports: " << mgr.reportCount() << "\n";
    std::cout << "  Total payroll: $" << mgr.totalPayroll() << "\n\n";

    // KEY: Employee can be tested WITHOUT Manager.
    // Manager's test links against Employee (Level 1).
    assert(e1.id() == 1);
    assert(mgr.reportCount() == 3);
    assert(mgr.totalPayroll() == 245000);
}

}  // close namespace classic_levelization


// ============================================================
// TECHNIQUE 2: Escalation
// BEFORE: EventQueue ←→ Event (cycle due to EventQueue::addEvent creating Events)
// AFTER:  EventManager (Level 3) → EventQueue (Level 2) → Event (Level 1)
// ============================================================

namespace escalation {

// Level 1: Pure data, no dependencies
struct Event {
    int         d_type;
    std::string d_payload;
    Event(int t, const std::string& p) : d_type(t), d_payload(p) {}
};

// Level 2: EventQueue — stores Events, knows nothing about EventManager
class EventQueue {
    std::vector<Event> d_events;
  public:
    void enqueue(Event ev)        { d_events.push_back(std::move(ev)); }
    bool empty()            const { return d_events.empty(); }
    Event dequeue() {
        Event ev = d_events.front();
        d_events.erase(d_events.begin());
        return ev;
    }
    int size() const { return static_cast<int>(d_events.size()); }
};

// Level 3: EventManager — knows about both Event and EventQueue
// This is where the "escalated" coordination logic lives.
class EventManager {
    EventQueue d_queue;
    int        d_processed;
  public:
    EventManager() : d_processed(0) {}

    // Creates AND enqueues an event — escalated from EventQueue
    void fireEvent(int type, const std::string& payload) {
        d_queue.enqueue(Event(type, payload));  // creates Event, uses EventQueue
    }

    int processAll(std::function<void(const Event&)> handler) {
        int count = 0;
        while (!d_queue.empty()) {
            handler(d_queue.dequeue());
            ++count;
            ++d_processed;
        }
        return count;
    }

    int totalProcessed() const { return d_processed; }
};

static void demo()
{
    std::cout << "--- Escalation: EventManager orchestrates Event + EventQueue ---\n";

    EventManager mgr;
    mgr.fireEvent(1, "user_login");
    mgr.fireEvent(2, "page_view");
    mgr.fireEvent(3, "logout");

    std::vector<std::string> received;
    int count = mgr.processAll([&received](const Event& ev) {
        received.push_back(ev.d_payload);
        std::cout << "  Processed: type=" << ev.d_type
                  << " payload=" << ev.d_payload << "\n";
    });

    assert(count == 3);
    assert(mgr.totalProcessed() == 3);
    std::cout << "  Total processed: " << mgr.totalProcessed() << "\n\n";
}

}  // close namespace escalation


// ============================================================
// TECHNIQUE 3: CCD Analysis
// Shows how layered vs. lateral architecture affects CCD
// ============================================================

namespace ccd_analysis {

static void demo()
{
    std::cout << "--- CCD (Cumulative Component Dependency) Analysis ---\n\n";

    // LAYERED ARCHITECTURE: CCD = O(n^2 / 2)
    // 5 components, each at a different level
    // A(L1), B(L2)→A, C(L3)→B→A, D(L4)→C→B→A, E(L5)→D→C→B→A
    // CCD = 0+1+2+3+4 = 10 (each component counts its own transitive deps)

    std::cout << "LAYERED (good): 5 components, strict levels\n";
    std::cout << "  A (Level 1): 0 deps  → CCD contribution = 0\n";
    std::cout << "  B (Level 2): 1 dep   → CCD contribution = 1\n";
    std::cout << "  C (Level 3): 2 deps  → CCD contribution = 2\n";
    std::cout << "  D (Level 4): 3 deps  → CCD contribution = 3\n";
    std::cout << "  E (Level 5): 4 deps  → CCD contribution = 4\n";
    int layeredCCD = 0+1+2+3+4;
    std::cout << "  Total CCD = " << layeredCCD << "\n\n";

    // LATERAL ARCHITECTURE: CCD = O(n^2)
    // 5 components that ALL depend on each other (except no cycles,
    // but each new one depends on all previous)
    // A: 0, B→A: 1, C→A,B: 2, D→A,B,C: 3, E→A,B,C,D: 4
    // (same numbers here, but in practice lateral architectures tend to
    //  have more horizontal deps)

    std::cout << "LATERAL (bad, extreme example): component N depends on ALL previous\n";
    std::cout << "  This creates maximum CCD for a given N.\n";
    std::cout << "  With 5 components: CCD same as layered ONLY IF strictly ordered.\n";
    std::cout << "  In practice, lateral means MANY cross-dependencies per level.\n\n";

    // Real lateral scenario: each component depends on MULTIPLE peers
    // A(L1): 0 deps
    // B(L1): depends on A (1 dep)
    // C(L1): depends on A AND B (2 deps)
    // D(L1): depends on A, B, AND C (3 deps)
    // E(L1): depends on A, B, C, AND D (4 deps)
    // But they're all "Level 1" conceptually — just lots of horizontal deps
    std::cout << "  If all 5 are at 'Level 1' with lateral cross-deps:\n";
    std::cout << "  CCD = 0+1+2+3+4 = " << 0+1+2+3+4 << "\n";
    std::cout << "  Worse: can't test any single component in isolation!\n\n";

    std::cout << "RULE: Prefer layered (vertical) dependencies over lateral (horizontal).\n";
    std::cout << "  Each additional level of layering is a compile-test-deploy checkpoint.\n";
    std::cout << "  Lateral dependencies make ALL components at a given level monolithic.\n\n";
}

}  // close namespace ccd_analysis


// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "=== Chapter 3: Levelization Techniques Combined Demo ===\n\n";

    classic_levelization::demo();
    escalation::demo();
    ccd_analysis::demo();

    std::cout << "=== Summary of Levelization Techniques ===\n\n";
    std::cout << "1. CLASSIC LEVELIZATION: Make logical bidirectional deps\n";
    std::cout << "   into physical unidirectional deps.\n\n";
    std::cout << "2. ESCALATION: Move interaction logic UP to a higher-level\n";
    std::cout << "   component that knows about both participants.\n\n";
    std::cout << "3. DEMOTION: Move reusable logic DOWN to minimize the\n";
    std::cout << "   transitive dependency footprint of clients.\n\n";
    std::cout << "4. OPAQUE POINTER: Break compile-time coupling by\n";
    std::cout << "   forward-declaring the impl class (PIMPL idiom).\n\n";
    std::cout << "5. DUMB DATA: Make low-level types hold raw data;\n";
    std::cout << "   higher-level code interprets it.\n\n";
    std::cout << "6. CALLBACKS: Invert the dependency — low-level code\n";
    std::cout << "   accepts a callback instead of calling high-level code.\n\n";
    std::cout << "7. CCD MINIMIZATION: Prefer layered over lateral architectures.\n";
    std::cout << "   Track CCD as a metric of complexity.\n";

    return 0;
}
