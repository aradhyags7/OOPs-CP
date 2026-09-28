#ifndef TRAFFIC_GENERATOR_HPP
#define TRAFFIC_GENERATOR_HPP

#include "Packet.hpp"
#include <random>
#include <string>

enum class TrafficPattern {
    CBR,
    POISSON,
    BURST
};

class TrafficGenerator {
protected:
    int flowId{-1};
    int sourceNodeId{-1};
    int destinationNodeId{-1};
    uint32_t packetSizeBytes{1000};
    double rateMbps{10.0};
    uint64_t nextSeqNum{1};
    uint64_t packetIdCounter{1};

public:
    TrafficGenerator(int fId, int src, int dst, double rate = 10.0, uint32_t pktSize = 1000)
        : flowId(fId), sourceNodeId(src), destinationNodeId(dst), packetSizeBytes(pktSize), rateMbps(rate) {}

    virtual ~TrafficGenerator() = default;

    virtual Packet generateNextPacket(double currentTime) = 0;
    virtual double getNextPacketTime(double currentTime) = 0;
    virtual std::string getPatternName() const = 0;

    int getFlowId() const { return flowId; }
    int getSourceNodeId() const { return sourceNodeId; }
    int getDestinationNodeId() const { return destinationNodeId; }
    double getRateMbps() const { return rateMbps; }
    uint32_t getPacketSizeBytes() const { return packetSizeBytes; }
};

class CBRGenerator : public TrafficGenerator {
public:
    CBRGenerator(int fId, int src, int dst, double rate = 10.0, uint32_t pktSize = 1000)
        : TrafficGenerator(fId, src, dst, rate, pktSize) {}

    Packet generateNextPacket(double currentTime) override;
    double getNextPacketTime(double currentTime) override;
    std::string getPatternName() const override { return "Constant Bit Rate (CBR)"; }
};

class PoissonGenerator : public TrafficGenerator {
private:
    std::mt19937 rng{std::random_device{}()};

public:
    PoissonGenerator(int fId, int src, int dst, double rate = 10.0, uint32_t pktSize = 1000)
        : TrafficGenerator(fId, src, dst, rate, pktSize) {}

    Packet generateNextPacket(double currentTime) override;
    double getNextPacketTime(double currentTime) override;
    std::string getPatternName() const override { return "Poisson Process"; }
};

class BurstGenerator : public TrafficGenerator {
private:
    double burstDuration{0.1}; // Active burst time in seconds
    double idleDuration{0.2};  // Idle time in seconds
    bool inBurst{true};
    double stateSwitchTime{0.0};
    std::mt19937 rng{std::random_device{}()};

public:
    BurstGenerator(int fId, int src, int dst, double rate = 20.0, uint32_t pktSize = 1000,
                   double burstSec = 0.1, double idleSec = 0.2)
        : TrafficGenerator(fId, src, dst, rate, pktSize),
          burstDuration(burstSec), idleDuration(idleSec) {}

    Packet generateNextPacket(double currentTime) override;
    double getNextPacketTime(double currentTime) override;
    std::string getPatternName() const override { return "Burst Traffic"; }
};

#endif // TRAFFIC_GENERATOR_HPP
