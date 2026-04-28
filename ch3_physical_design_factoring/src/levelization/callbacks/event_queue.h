// event_queue.h                                                      -*-C++-*-
//
// Demonstrates CALLBACK levelization techniques.
// Book reference: Lakos §3.5.6–3.5.7
//
// EventQueue uses FUNCTION CALLBACKS to avoid knowing about specific handlers.

#ifndef INCLUDED_EVENT_QUEUE
#define INCLUDED_EVENT_QUEUE

#include <functional>
#include <string>
#include <vector>

/// A simple event queue that dispatches events to registered callbacks.
///
/// LEVELIZATION TECHNIQUE: Instead of EventQueue knowing about specific
/// event handler classes (which would create upward dependencies),
/// it accepts callbacks. The handler registers itself via setHandler().
///
/// This allows EventQueue to stay at Level 1 while handlers
/// can be at any higher level.
class EventQueue {
  public:
    struct Event {
        int         d_type;
        std::string d_data;
        int         d_priority;
    };

    // Function callback type (C-style pointer)
    using FunctionCallback = void(*)(const Event&);

    // Functor callback type (any callable)
    using FunctorCallback = std::function<void(const Event&)>;

  private:
    std::vector<Event>           d_events;
    FunctorCallback              d_handler;

  public:
    EventQueue();

    // MANIPULATORS

    /// Sets the callback handler for dispatching events.
    /// Accepts any callable: function pointer, lambda, functor.
    void setHandler(FunctorCallback handler);

    /// Pushes an event onto the queue.
    void push(int type, const std::string& data, int priority = 0);

    /// Dispatches all pending events to the registered handler.
    /// Events are processed in priority order (higher first).
    int dispatchAll();

    // ACCESSORS

    /// Returns the number of pending events.
    int pendingCount() const;

    /// Returns true if there are no pending events.
    bool empty() const;
};

#endif  // INCLUDED_EVENT_QUEUE
