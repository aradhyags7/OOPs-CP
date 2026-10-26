#include "PacketGenerator.hpp"
#include <random>

using namespace std;

Packet PacketGenerator::createHTTPS(uint64_t id, const std::string& srcIP, const std::string& dstIP, Direction dir) {
    return Packet(id, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), 51234, 443,
                  Protocol::TCP, dir, 1200, 0.0);
}

Packet PacketGenerator::createDNS(uint64_t id, const std::string& srcIP, const std::string& dstIP, Direction dir) {
    return Packet(id, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), 53421, 53,
                  Protocol::UDP, dir, 128, 0.0);
}

Packet PacketGenerator::createSSH(uint64_t id, const std::string& srcIP, const std::string& dstIP, Direction dir) {
    return Packet(id, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), 49812, 22,
                  Protocol::TCP, dir, 768, 0.0);
}

Packet PacketGenerator::createTelnet(uint64_t id, const std::string& srcIP, const std::string& dstIP, Direction dir) {
    return Packet(id, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), 48112, 23,
                  Protocol::TCP, dir, 64, 0.0);
}

Packet PacketGenerator::createPing(uint64_t id, const std::string& srcIP, const std::string& dstIP, Direction dir) {
    return Packet(id, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), 0, 0,
                  Protocol::ICMP, dir, 64, 0.0);
}

std::vector<Packet> PacketGenerator::generateBenchmarkBatch(size_t count) {
    std::vector<Packet> batch;
    batch.reserve(count);

    std::mt19937 rng(42); // Deterministic seed for reproducible testing
    std::uniform_int_distribution<uint32_t> ipDist(0x0A000001, 0x0AFFFFFF); // 10.0.0.0/8
    std::uniform_int_distribution<uint16_t> portDist(1024, 65535);
    std::uniform_int_distribution<int> typeDist(0, 3);

    for (size_t i = 1; i <= count; ++i) {
        int t = typeDist(rng);
        if (t == 0) {
            batch.push_back(createHTTPS(i, "10.0.1.25", "192.0.2.1", Direction::INBOUND));
        } else if (t == 1) {
            batch.push_back(createDNS(i, "10.0.1.50", "8.8.8.8", Direction::OUTBOUND));
        } else if (t == 2) {
            batch.push_back(createTelnet(i, "192.168.100.5", "10.0.0.1", Direction::INBOUND));
        } else {
            batch.push_back(createPing(i, "172.16.0.10", "10.0.0.1", Direction::INBOUND));
        }
    }

    return batch;
}
