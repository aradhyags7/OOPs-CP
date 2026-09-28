#ifndef QUEUE_MANAGER_HPP
#define QUEUE_MANAGER_HPP

#include "Packet.hpp"
#include <queue>
#include <string>
#include <random>

class QueueManager {
protected:
    size_t maxCapacityPackets{50};
    uint64_t totalEnqueued{0};
    uint64_t totalDropped{0};

public:
    explicit QueueManager(size_t capacity = 50) : maxCapacityPackets(capacity) {}
    virtual ~QueueManager() = default;

    virtual bool enqueue(Packet& pkt, double currentTime) = 0;
    virtual Packet dequeue() = 0;
    virtual bool isEmpty() const = 0;
    virtual size_t size() const = 0;
    virtual size_t capacity() const { return maxCapacityPackets; }
    virtual std::string getQueueType() const = 0;

    uint64_t getDroppedCount() const { return totalDropped; }
    uint64_t getEnqueuedCount() const { return totalEnqueued; }
    void resetStats() { totalEnqueued = 0; totalDropped = 0; }
};

class DropTailQueue : public QueueManager {
private:
    std::queue<Packet> pktQueue;

public:
    explicit DropTailQueue(size_t capacity = 50);

    bool enqueue(Packet& pkt, double currentTime) override;
    Packet dequeue() override;
    bool isEmpty() const override;
    size_t size() const override;
    std::string getQueueType() const override { return "DropTail"; }
};

class REDQueue : public QueueManager {
private:
    std::queue<Packet> pktQueue;
    double minThresh{10.0};
    double maxThresh{40.0};
    double maxP{0.1}; // Maximum drop probability
    double weightEWMA{0.002}; // Queue weight for EWMA
    double avgQueueSize{0.0};
    int countSinceLastDrop{-1};
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist{0.0, 1.0};

public:
    REDQueue(size_t capacity = 50, double minT = 10.0, double maxT = 40.0, double maxProb = 0.1, double ewmaWeight = 0.002);

    bool enqueue(Packet& pkt, double currentTime) override;
    Packet dequeue() override;
    bool isEmpty() const override;
    size_t size() const override;
    std::string getQueueType() const override { return "RED (Random Early Detection)"; }

    double getAvgQueueSize() const { return avgQueueSize; }
};

#endif // QUEUE_MANAGER_HPP
