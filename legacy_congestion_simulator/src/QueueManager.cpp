#include "QueueManager.hpp"

// ==================== DropTailQueue Implementation ====================

DropTailQueue::DropTailQueue(size_t capacity)
    : QueueManager(capacity) {}

bool DropTailQueue::enqueue(Packet& pkt, double currentTime) {
    pkt.enqueueTime = currentTime;

    if (pktQueue.size() >= maxCapacityPackets) {
        pkt.isDropped = true;
        pkt.dropReason = "DropTail Queue Overflow";
        totalDropped++;
        return false;
    }

    pktQueue.push(pkt);
    totalEnqueued++;
    return true;
}

Packet DropTailQueue::dequeue() {
    if (pktQueue.empty()) {
        return Packet();
    }
    Packet pkt = pktQueue.front();
    pktQueue.pop();
    return pkt;
}

bool DropTailQueue::isEmpty() const {
    return pktQueue.empty();
}

size_t DropTailQueue::size() const {
    return pktQueue.size();
}

// ==================== REDQueue Implementation ====================

REDQueue::REDQueue(size_t capacity, double minT, double maxT, double maxProb, double ewmaWeight)
    : QueueManager(capacity), minThresh(minT), maxThresh(maxT), maxP(maxProb), weightEWMA(ewmaWeight) {
    rng.seed(1337); // Fixed seed for reproducible RED behavior
}

bool REDQueue::enqueue(Packet& pkt, double currentTime) {
    pkt.enqueueTime = currentTime;
    double currentSize = static_cast<double>(pktQueue.size());

    // Update Exponentially Weighted Moving Average (EWMA) of Queue Size
    avgQueueSize = (1.0 - weightEWMA) * avgQueueSize + weightEWMA * currentSize;

    // Hard drop if queue capacity is reached
    if (pktQueue.size() >= maxCapacityPackets) {
        pkt.isDropped = true;
        pkt.dropReason = "RED Hard Queue Full";
        totalDropped++;
        countSinceLastDrop = 0;
        return false;
    }

    // RED Algorithmic Drop Decisions
    if (avgQueueSize < minThresh) {
        countSinceLastDrop = -1;
    } else if (avgQueueSize >= maxThresh) {
        // Drop packet
        pkt.isDropped = true;
        pkt.dropReason = "RED Early Drop (Avg Queue >= MaxThresh)";
        totalDropped++;
        countSinceLastDrop = 0;
        return false;
    } else {
        // minThresh <= avgQueueSize < maxThresh
        countSinceLastDrop++;
        double p_b = maxP * ((avgQueueSize - minThresh) / (maxThresh - minThresh));
        double p_a = p_b;
        if (1.0 - countSinceLastDrop * p_b > 0) {
            p_a = p_b / (1.0 - countSinceLastDrop * p_b);
        }

        if (dist(rng) < p_a) {
            // Probabilistic Drop
            pkt.isDropped = true;
            pkt.dropReason = "RED Probabilistic Early Drop";
            totalDropped++;
            countSinceLastDrop = 0;
            return false;
        }
    }

    pktQueue.push(pkt);
    totalEnqueued++;
    return true;
}

Packet REDQueue::dequeue() {
    if (pktQueue.empty()) {
        return Packet();
    }
    Packet pkt = pktQueue.front();
    pktQueue.pop();
    return pkt;
}

bool REDQueue::isEmpty() const {
    return pktQueue.empty();
}

size_t REDQueue::size() const {
    return pktQueue.size();
}
