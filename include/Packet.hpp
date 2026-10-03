#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <string>

enum class Protocol {
    TCP,
    UDP,
    ICMP
};

enum class Direction {
    INBOUND,
    OUTBOUND
};

struct Packet {
    uint64_t id{0};
    uint32_t srcIP{0};
    uint32_t dstIP{0};
    uint16_t srcPort{0};
    uint16_t dstPort{0};
    Protocol protocol{Protocol::TCP};
    Direction direction{Direction::INBOUND};
    uint32_t payloadSize{0};
    double timestamp{0.0};

    Packet() = default;

    Packet(uint64_t pId, uint32_t sIP, uint32_t dIP, uint16_t sPort, uint16_t dPort,
           Protocol proto, Direction dir, uint32_t size = 64, double ts = 0.0)
        : id(pId), srcIP(sIP), dstIP(dIP), srcPort(sPort), dstPort(dPort),
          protocol(proto), direction(dir), payloadSize(size), timestamp(ts) {
        if (protocol == Protocol::ICMP) {
            srcPort = 0;
            dstPort = 0;
        }
    }

    static uint32_t parseIPv4(const std::string& ipStr);
    static std::string ipToString(uint32_t ip);

    std::string getProtocolString() const;
    std::string getDirectionString() const;
    std::string toString() const;
};

#endif // PACKET_HPP
