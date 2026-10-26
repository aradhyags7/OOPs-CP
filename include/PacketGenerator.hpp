#ifndef PACKET_GENERATOR_HPP
#define PACKET_GENERATOR_HPP

#include "Packet.hpp"
#include <vector>
#include <string>

using namespace std;

class PacketGenerator {
public:
    static Packet createHTTPS(uint64_t id, const std::string& srcIP, const std::string& dstIP,
                              Direction dir = Direction::INBOUND);

    static Packet createDNS(uint64_t id, const std::string& srcIP, const std::string& dstIP,
                            Direction dir = Direction::OUTBOUND);

    static Packet createSSH(uint64_t id, const std::string& srcIP, const std::string& dstIP,
                            Direction dir = Direction::INBOUND);

    static Packet createTelnet(uint64_t id, const std::string& srcIP, const std::string& dstIP,
                               Direction dir = Direction::INBOUND);

    static Packet createPing(uint64_t id, const std::string& srcIP, const std::string& dstIP,
                             Direction dir = Direction::INBOUND);

    static std::vector<Packet> generateBenchmarkBatch(size_t count);
};

#endif // PACKET_GENERATOR_HPP
