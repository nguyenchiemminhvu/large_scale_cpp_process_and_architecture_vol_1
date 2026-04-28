// callbacks_demo.cpp                                                 -*-C++-*-
//
// Demonstrates all four callback types from Lakos §3.5.6-3.5.7:
//   1. Data callbacks (C-style void* + function pointer)
//   2. Function callbacks (function pointer, no user data)
//   3. Functor callbacks (std::function, accepts lambdas)
//   4. Protocol/interface callbacks (abstract base class)

#ifndef INCLUDED_EVENT_QUEUE
#include <event_queue.h>
#endif

#ifndef INCLUDED_LOGGER
#include <logger.h>
#endif

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// CALLBACK TYPE 1: Data Callback (C-style void* + function pointer)
// Logger uses this pattern.
// ============================================================

struct LogContext {
    std::vector<std::string> messages;
    int                      errorCount;
};

static void logCallbackFunction(const char* message, int severity, void* userData)
{
    auto* ctx = static_cast<LogContext*>(userData);
    ctx->messages.push_back(message);
    if (severity >= Logger::k_ERROR) ++ctx->errorCount;

    const char* sevStr = severity == Logger::k_DEBUG   ? "DEBUG" :
                         severity == Logger::k_INFO    ? "INFO"  :
                         severity == Logger::k_WARNING ? "WARN"  : "ERROR";
    std::cout << "  [" << sevStr << "] " << message << "\n";
}

static void demoDataCallback()
{
    std::cout << "--- 1. Data Callback (C-style void* pattern) ---\n";
    std::cout << "   Logger stays at Level 1 — no knowledge of LogContext\n\n";

    LogContext ctx{};
    ctx.errorCount = 0;

    Logger log;
    log.setCallback(logCallbackFunction, &ctx);  // passes context as void*

    log.log("System initialized", Logger::k_INFO);
    log.log("Connection timeout", Logger::k_WARNING);
    log.log("Fatal error occurred", Logger::k_ERROR);

    assert(ctx.messages.size() == 3);
    assert(ctx.errorCount == 1);
    std::cout << "  Messages captured: " << ctx.messages.size() << "\n";
    std::cout << "  Errors: " << ctx.errorCount << "\n\n";
}

// ============================================================
// CALLBACK TYPE 2: Functor Callback (std::function)
// EventQueue uses this pattern.
// ============================================================

static void demoFunctorCallback()
{
    std::cout << "--- 2. Functor Callback (std::function / lambda) ---\n";
    std::cout << "   EventQueue accepts ANY callable — complete decoupling\n\n";

    EventQueue queue;

    // Lambda as callback — EventQueue knows nothing about this lambda's type
    std::vector<std::string> received;
    queue.setHandler([&received](const EventQueue::Event& ev) {
        received.push_back(ev.d_data);
        std::cout << "  Handled event type=" << ev.d_type
                  << " data='" << ev.d_data
                  << "' priority=" << ev.d_priority << "\n";
    });

    queue.push(1, "Login",   5);
    queue.push(2, "Logout",  1);
    queue.push(3, "Error!",  10);  // highest priority

    assert(queue.pendingCount() == 3);
    int count = queue.dispatchAll();

    assert(count == 3);
    assert(queue.empty());
    assert(received[0] == "Error!");   // dispatched first (priority 10)
    assert(received[1] == "Login");    // priority 5
    assert(received[2] == "Logout");   // priority 1

    std::cout << "  Dispatched in priority order: ✓\n\n";
}

// ============================================================
// CALLBACK TYPE 3: Protocol Class (Abstract Interface)
// The most powerful and structured form of callback.
// ============================================================

// Level 1: Abstract "protocol" class — defines the interface
class EventObserver {
  public:
    virtual ~EventObserver() = default;
    virtual void onEvent(int type, const std::string& data) = 0;
    virtual void onError(const std::string& message) = 0;
};

// Level 1: Publisher that accepts a protocol callback
class EventPublisher {
    EventObserver* d_observer;  // pointer to abstract interface

  public:
    EventPublisher() : d_observer(nullptr) {}

    void setObserver(EventObserver* obs) { d_observer = obs; }

    void publishEvent(int type, const std::string& data) {
        if (d_observer) d_observer->onEvent(type, data);
    }

    void publishError(const std::string& msg) {
        if (d_observer) d_observer->onError(msg);
    }
};

// Level 2+: Concrete observer — EventPublisher doesn't know about this class
class ConcreteObserver : public EventObserver {
    int d_eventCount;
    int d_errorCount;

  public:
    ConcreteObserver() : d_eventCount(0), d_errorCount(0) {}

    void onEvent(int type, const std::string& data) override {
        ++d_eventCount;
        std::cout << "  ConcreteObserver: event type=" << type
                  << " data='" << data << "'\n";
    }

    void onError(const std::string& message) override {
        ++d_errorCount;
        std::cout << "  ConcreteObserver: error='" << message << "'\n";
    }

    int eventCount() const { return d_eventCount; }
    int errorCount() const { return d_errorCount; }
};

static void demoProtocolCallback()
{
    std::cout << "--- 3. Protocol Class (Abstract Interface) Callback ---\n";
    std::cout << "   EventPublisher depends on EventObserver (abstract only)\n";
    std::cout << "   ConcreteObserver is at a higher level\n\n";

    EventPublisher publisher;
    ConcreteObserver observer;

    publisher.setObserver(&observer);  // register via abstract interface
    publisher.publishEvent(100, "Order placed");
    publisher.publishEvent(101, "Order confirmed");
    publisher.publishError("Network timeout");

    assert(observer.eventCount() == 2);
    assert(observer.errorCount() == 1);
    std::cout << "  Events handled: " << observer.eventCount() << "\n";
    std::cout << "  Errors handled: " << observer.errorCount() << "\n\n";
}

// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "=== Chapter 3: Callback Levelization Techniques ===\n\n";

    std::cout << "CONCEPT:\n";
    std::cout << "  Instead of A depending on B directly (creating a potential cycle),\n";
    std::cout << "  A accepts a callback. B registers itself with A via the callback.\n";
    std::cout << "  Result: A stays at a low level; B can be at any higher level.\n\n";

    demoDataCallback();
    demoFunctorCallback();
    demoProtocolCallback();

    std::cout << "=== Comparison of Callback Styles ===\n\n";
    std::cout << "Style              | Complexity | ABI stable | Type safety | Overhead\n";
    std::cout << "Data (void*)       | Low        | Yes        | None        | None\n";
    std::cout << "Function pointer   | Low        | Yes        | Partial     | None\n";
    std::cout << "std::function      | Medium     | No         | Full        | Some\n";
    std::cout << "Protocol class     | High       | Yes        | Full        | vtable\n\n";
    std::cout << "Choose based on: ABI requirements, performance needs, type safety needs.\n";

    return 0;
}
