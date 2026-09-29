#include "TrafficGenerator.hpp"

// ==================== CBRGenerator Implementation ====================

Packet CBRGenerator::generateNextPacket(double currentTime) {
    Packet pkt(packetIdCounter++, sourceNodeId, destinationNodeId, flowId,
               packetSizeBytes, nextSeqNum++, PacketType::DATA, currentTime);
    return pkt;
}

double CBRGenerator::getNextPacketTime(double currentTime) {
    // Inter-arrival time in seconds = (bits) / (bits per second)
    double intervalSec = (static_cast<double>(packetSizeBytes) * 8.0) / (rateMbps * 1e6);
    return currentTime + intervalSec;
}

// ==================== PoissonGenerator Implementation ====================

Packet PoissonGenerator::generateNextPacket(double currentTime) {
    Packet pkt(packetIdCounter++, sourceNodeId, destinationNodeId, flowId,
               packetSizeBytes, nextSeqNum++, PacketType::DATA, currentTime);
    return pkt;
}

double PoissonGenerator::getNextPacketTime(double currentTime) {
    double meanRatePacketsPerSec = (rateMbps * 1e6) / (static_cast<double>(packetSizeBytes) * 8.0);
    std::exponential_distribution<double> expDist(meanRatePacketsPerSec);
    double dt = expDist(rng);
    return currentTime + dt;
}

// ==================== BurstGenerator Implementation ====================

Packet BurstGenerator::generateNextPacket(double currentTime) {
    Packet pkt(packetIdCounter++, sourceNodeId, destinationNodeId, flowId,
               packetSizeBytes, nextSeqNum++, PacketType::DATA, currentTime);
    return pkt;
}

double BurstGenerator::getNextPacketTime(double currentTime) {
    // Toggle state if duration expired
    if (currentTime >= stateSwitchTime) {
        inBurst = !inBurst;
        stateSwitchTime = currentTime + (inBurst ? burstDuration : idleDuration);
    }

    if (inBurst) {
        // High rate generation during burst
        double intervalSec = (static_cast<double>(packetSizeBytes) * 8.0) / (rateMbps * 1e6);
        return currentTime + intervalSec;
    } else {
        // Idle period delay
        return stateSwitchTime;
    }
}
