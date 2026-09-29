#include "Link.hpp"

Link::Link(int linkId, int srcNode, int dstNode, double bwMbps, double propDelaySec,
           double lossRate, std::unique_ptr<QueueManager> queueMgr)
    : id(linkId), fromNodeId(srcNode), toNodeId(dstNode), bandwidthMbps(bwMbps),
      propagationDelaySec(propDelaySec), randomLossRate(lossRate), queue(std::move(queueMgr)) {
    if (!queue) {
        queue = std::make_unique<DropTailQueue>(50);
    }
}

bool Link::enqueuePacket(Packet& pkt, double currentTime) {
    if (!active) {
        pkt.isDropped = true;
        pkt.dropReason = "Link Inactive / Link Down";
        totalPacketsDropped++;
        return false;
    }

    if (shouldRandomlyDrop()) {
        pkt.isDropped = true;
        pkt.dropReason = "Random Physical Bit Error Loss";
        totalPacketsDropped++;
        return false;
    }

    bool enqueued = queue->enqueue(pkt, currentTime);
    if (!enqueued) {
        totalPacketsDropped++;
    }
    return enqueued;
}

Packet Link::dequeuePacket() {
    if (!queue || queue->isEmpty()) {
        return Packet();
    }
    return queue->dequeue();
}

double Link::getTransmissionDelaySec(uint32_t packetSizeBytes) const {
    if (bandwidthMbps <= 0.0) return 0.0;
    return (static_cast<double>(packetSizeBytes) * 8.0) / (bandwidthMbps * 1e6);
}

bool Link::isQueueEmpty() const {
    return !queue || queue->isEmpty();
}

size_t Link::getQueueSize() const {
    return queue ? queue->size() : 0;
}

size_t Link::getQueueCapacity() const {
    return queue ? queue->capacity() : 0;
}

std::string Link::getQueueType() const {
    return queue ? queue->getQueueType() : "None";
}

bool Link::shouldRandomlyDrop() {
    if (randomLossRate <= 0.0) return false;
    return lossDist(rng) < randomLossRate;
}
