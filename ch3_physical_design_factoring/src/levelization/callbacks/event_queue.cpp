// event_queue.cpp
#ifndef INCLUDED_EVENT_QUEUE
#include <event_queue.h>
#endif

#include <algorithm>

EventQueue::EventQueue() = default;

void EventQueue::setHandler(FunctorCallback handler)
{
    d_handler = std::move(handler);
}

void EventQueue::push(int type, const std::string& data, int priority)
{
    d_events.push_back({type, data, priority});
}

int EventQueue::dispatchAll()
{
    if (!d_handler) return 0;

    // Sort by priority (descending)
    std::sort(d_events.begin(), d_events.end(),
              [](const Event& a, const Event& b) {
                  return a.d_priority > b.d_priority;
              });

    int dispatched = static_cast<int>(d_events.size());
    for (const auto& ev : d_events) {
        d_handler(ev);
    }
    d_events.clear();
    return dispatched;
}

int EventQueue::pendingCount() const
{
    return static_cast<int>(d_events.size());
}

bool EventQueue::empty() const
{
    return d_events.empty();
}
