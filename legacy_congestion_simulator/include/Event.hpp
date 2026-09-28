#ifndef EVENT_HPP
#define EVENT_HPP

#include "Packet.hpp"
#include <string>

enum class EventType {
    PACKET_GENERATE,
    PACKET_ENQUEUE,
    PACKET_TRANSMIT_START,
    PACKET_TRANSMIT_END,
    PACKET_ARRIVE_NODE,
    ACK_RECEIVE,
    TIMEOUT_RETRANSMISSION,
    PERIODIC_METRICS_LOG
};

struct Event {
    uint64_t id{0};
    double time{0.0};
    EventType type{EventType::PACKET_GENERATE};
    int nodeLinkOrFlowId{-1}; // Target Node ID, Link ID, or Flow ID depending on event type
    int targetNodeId{-1};
    Packet packet;

    Event() = default;

    Event(uint64_t eId, double t, EventType eType, int targetId, const Packet& pkt = Packet())
        : id(eId), time(t), type(eType), nodeLinkOrFlowId(targetId), targetNodeId(targetId), packet(pkt) {}

    // Comparator for priority queue (min-heap based on event time)
    bool operator>(const Event& other) const {
        if (time == other.time) {
            return id > other.id; // Tie breaker using event creation ID
        }
        return time > other.time;
    }

    std::string getTypeString() const {
        switch (type) {
            case EventType::PACKET_GENERATE:        return "PACKET_GENERATE";
            case EventType::PACKET_ENQUEUE:         return "PACKET_ENQUEUE";
            case EventType::PACKET_TRANSMIT_START:  return "PACKET_TRANSMIT_START";
            case EventType::PACKET_TRANSMIT_END:    return "PACKET_TRANSMIT_END";
            case EventType::PACKET_ARRIVE_NODE:     return "PACKET_ARRIVE_NODE";
            case EventType::ACK_RECEIVE:            return "ACK_RECEIVE";
            case EventType::TIMEOUT_RETRANSMISSION: return "TIMEOUT_RETRANSMISSION";
            case EventType::PERIODIC_METRICS_LOG:   return "PERIODIC_METRICS_LOG";
            default:                                return "UNKNOWN";
        }
    }
};

#endif // EVENT_HPP
