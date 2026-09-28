#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <string>

enum class PacketType {
    DATA,
    ACK,
    SYN,
    FIN
};

struct Packet {
    uint64_t id{0};
    int sourceId{-1};
    int destinationId{-1};
    int flowId{-1};
    uint32_t sizeBytes{1000};
    uint64_t sequenceNum{0};
    uint64_t ackSequenceNum{0};
    PacketType type{PacketType::DATA};

    double generationTime{0.0};
    double enqueueTime{0.0};
    double departureTime{0.0};
    double receiveTime{0.0};
    double originalPacketGenTime{0.0}; // Used by ACK to calculate RTT

    bool isDropped{false};
    std::string dropReason{""};

    Packet() = default;

    Packet(uint64_t pId, int src, int dst, int fId, uint32_t size, uint64_t seq, PacketType pType, double genTime)
        : id(pId), sourceId(src), destinationId(dst), flowId(fId), sizeBytes(size),
          sequenceNum(seq), type(pType), generationTime(genTime), originalPacketGenTime(genTime) {}

    std::string getTypeString() const {
        switch (type) {
            case PacketType::DATA: return "DATA";
            case PacketType::ACK:  return "ACK";
            case PacketType::SYN:  return "SYN";
            case PacketType::FIN:  return "FIN";
            default:               return "UNKNOWN";
        }
    }
};

#endif // PACKET_HPP
