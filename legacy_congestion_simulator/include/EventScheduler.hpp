#ifndef EVENT_SCHEDULER_HPP
#define EVENT_SCHEDULER_HPP

#include "Event.hpp"
#include <queue>
#include <vector>

class EventScheduler {
private:
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue;
    uint64_t eventIdCounter{1};

public:
    EventScheduler() = default;

    void scheduleEvent(double time, EventType type, int targetId, const Packet& pkt = Packet()) {
        Event evt(eventIdCounter++, time, type, targetId, pkt);
        eventQueue.push(evt);
    }

    void scheduleEvent(const Event& evt) {
        Event copyEvt = evt;
        if (copyEvt.id == 0) {
            copyEvt.id = eventIdCounter++;
        }
        eventQueue.push(copyEvt);
    }

    bool hasEvents() const {
        return !eventQueue.empty();
    }

    Event popNextEvent() {
        if (eventQueue.empty()) {
            return Event();
        }
        Event evt = eventQueue.top();
        eventQueue.pop();
        return evt;
    }

    Event peekNextEvent() const {
        if (eventQueue.empty()) {
            return Event();
        }
        return eventQueue.top();
    }

    size_t size() const {
        return eventQueue.size();
    }

    void clear() {
        while (!eventQueue.empty()) {
            eventQueue.pop();
        }
        eventIdCounter = 1;
    }
};

#endif // EVENT_SCHEDULER_HPP
