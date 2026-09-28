#ifndef NODE_HPP
#define NODE_HPP

#include "Packet.hpp"
#include "CongestionControl.hpp"
#include "TrafficGenerator.hpp"
#include <string>
#include <unordered_map>
#include <memory>

enum class NodeType {
    HOST,
    ROUTER
};

class Node {
protected:
    int id{-1};
    std::string name;
    NodeType type{NodeType::HOST};

public:
    Node(int nodeId, const std::string& nodeName, NodeType nType)
        : id(nodeId), name(nodeName), type(nType) {}

    virtual ~Node() = default;

    int getId() const { return id; }
    std::string getName() const { return name; }
    NodeType getType() const { return type; }

    virtual std::string getTypeString() const {
        return (type == NodeType::HOST) ? "Host" : "Router";
    }
};

class HostNode : public Node {
private:
    std::unique_ptr<CongestionControlAlgorithm> congestionControl;
    std::unique_ptr<TrafficGenerator> trafficGen;

    uint64_t nextSequenceNumber{1};
    uint64_t expectedSequenceNumber{1};
    uint64_t lastAckedSequenceNum{0};

    // Karn/Jacobson RTT Estimation algorithm state
    double srtt{0.1};      // Smoothed RTT (seconds)
    double rttvar{0.05};   // RTT Variation (seconds)
    double rto{0.5};       // Retransmission Timeout (seconds)
    double alpha{0.125};
    double beta{0.25};

    uint64_t packetsSent{0};
    uint64_t packetsReceived{0};
    uint64_t acksSent{0};
    uint64_t acksReceived{0};
    uint64_t retransmissions{0};

public:
    HostNode(int nodeId, const std::string& nodeName,
             std::unique_ptr<CongestionControlAlgorithm> cc = std::make_unique<TCPReno>(),
             std::unique_ptr<TrafficGenerator> gen = nullptr);

    CongestionControlAlgorithm* getCongestionControl() { return congestionControl.get(); }
    TrafficGenerator* getTrafficGenerator() { return trafficGen.get(); }

    void setCongestionControl(std::unique_ptr<CongestionControlAlgorithm> cc) {
        congestionControl = std::move(cc);
    }
    void setTrafficGenerator(std::unique_ptr<TrafficGenerator> gen) {
        trafficGen = std::move(gen);
    }

    // Process incoming packet (Data or ACK)
    Packet processReceivedPacket(const Packet& pkt, double currentTime, bool& generateAckOut, Packet& ackPktOut);

    // Update RTT calculation on ACK receipt
    void updateRTT(double sampleRTT);

    // Handle retransmission event
    void handleTimeout(double currentTime);

    double getRTO() const { return rto; }
    double getSRTT() const { return srtt; }

    uint64_t getNextSequenceNum() { return nextSequenceNumber++; }
    uint64_t getPacketsSent() const { return packetsSent; }
    uint64_t getPacketsReceived() const { return packetsReceived; }
    uint64_t getAcksSent() const { return acksSent; }
    uint64_t getAcksReceived() const { return acksReceived; }
    uint64_t getRetransmissions() const { return retransmissions; }

    void incrementPacketsSent() { packetsSent++; }
};

class RouterNode : public Node {
private:
    // Routing table mapping destination Node ID -> outgoing Link ID
    std::unordered_map<int, int> routingTable;

public:
    RouterNode(int nodeId, const std::string& nodeName)
        : Node(nodeId, nodeName, NodeType::ROUTER) {}

    void addRoute(int destinationNodeId, int outgoingLinkId) {
        routingTable[destinationNodeId] = outgoingLinkId;
    }

    int getNextHopLink(int destinationNodeId) const {
        auto it = routingTable.find(destinationNodeId);
        if (it != routingTable.end()) {
            return it->second;
        }
        return -1; // Route not found
    }

    const std::unordered_map<int, int>& getRoutingTable() const {
        return routingTable;
    }
};

#endif // NODE_HPP
