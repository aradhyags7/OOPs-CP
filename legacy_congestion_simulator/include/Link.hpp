#ifndef LINK_HPP
#define LINK_HPP

#include "Packet.hpp"
#include "QueueManager.hpp"
#include <memory>
#include <string>
#include <random>

class Link {
private:
    int id{-1};
    int fromNodeId{-1};
    int toNodeId{-1};
    double bandwidthMbps{10.0};       // Megabits per second
    double propagationDelaySec{0.010}; // 10ms delay
    double randomLossRate{0.0};       // Random loss probability [0, 1]
    bool active{true};

    std::unique_ptr<QueueManager> queue;
    bool transmitting{false};
    double currentTransmitEndTime{0.0};

    uint64_t totalBitsTransmitted{0};
    uint64_t totalPacketsTransmitted{0};
    uint64_t totalPacketsDropped{0};

    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> lossDist{0.0, 1.0};

public:
    Link(int linkId, int srcNode, int dstNode, double bwMbps = 10.0, double propDelaySec = 0.01,
         double lossRate = 0.0, std::unique_ptr<QueueManager> queueMgr = std::make_unique<DropTailQueue>(50));

    // Disable copy constructor due to unique_ptr
    Link(const Link&) = delete;
    Link& operator=(const Link&) = delete;
    Link(Link&&) = default;
    Link& operator=(Link&&) = default;

    int getId() const { return id; }
    int getFromNodeId() const { return fromNodeId; }
    int getToNodeId() const { return toNodeId; }
    double getBandwidthMbps() const { return bandwidthMbps; }
    double getPropagationDelaySec() const { return propagationDelaySec; }
    double getRandomLossRate() const { return randomLossRate; }
    bool isActive() const { return active; }

    void setBandwidthMbps(double bw) { bandwidthMbps = bw; }
    void setPropagationDelaySec(double delay) { propagationDelaySec = delay; }
    void setRandomLossRate(double loss) { randomLossRate = loss; }
    void setActive(bool status) { active = status; }

    // Enqueue packet onto link's buffer
    bool enqueuePacket(Packet& pkt, double currentTime);

    // Dequeue next packet from buffer
    Packet dequeuePacket();

    // Calculate transmission delay for packet size: (sizeBytes * 8) / (bandwidthMbps * 1e6)
    double getTransmissionDelaySec(uint32_t packetSizeBytes) const;

    // Check if link queue is empty
    bool isQueueEmpty() const;

    // Get current queue length
    size_t getQueueSize() const;
    size_t getQueueCapacity() const;
    std::string getQueueType() const;

    bool isTransmitting() const { return transmitting; }
    void setTransmitting(bool status, double endTime = 0.0) {
        transmitting = status;
        currentTransmitEndTime = endTime;
    }
    double getCurrentTransmitEndTime() const { return currentTransmitEndTime; }

    // Random drop check
    bool shouldRandomlyDrop();

    // Stats tracking
    void addTransmittedBytes(uint32_t bytes) {
        totalBitsTransmitted += (static_cast<uint64_t>(bytes) * 8);
        totalPacketsTransmitted++;
    }
    void incrementDroppedCount() { totalPacketsDropped++; }

    uint64_t getTotalBitsTransmitted() const { return totalBitsTransmitted; }
    uint64_t getTotalPacketsTransmitted() const { return totalPacketsTransmitted; }
    uint64_t getTotalPacketsDropped() const { return totalPacketsDropped + (queue ? queue->getDroppedCount() : 0); }

    QueueManager* getQueueManager() { return queue.get(); }
};

#endif // LINK_HPP
