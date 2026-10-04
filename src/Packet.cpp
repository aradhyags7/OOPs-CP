#include "Packet.hpp"
#include <sstream>
#include <stdexcept>
#include <vector>

uint32_t Packet::parseIPv4(const std::string& ipStr) {
    std::stringstream ss(ipStr);
    std::string segment;
    std::vector<std::string> parts;

    while (std::getline(ss, segment, '.')) {
        parts.push_back(segment);
    }

    if (parts.size() != 4) {
        throw std::invalid_argument("Malformed IPv4 address (must have 4 octets): '" + ipStr + "'");
    }

    uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
        const auto& part = parts[i];
        if (part.empty() || (part.size() > 1 && part[0] == '0')) {
            // Check for empty or invalid leading zeros (except single '0')
            if (part.empty()) throw std::invalid_argument("Empty octet in IPv4: '" + ipStr + "'");
        }

        for (char c : part) {
            if (c < '0' || c > '9') {
                throw std::invalid_argument("Non-numeric character in IPv4: '" + ipStr + "'");
            }
        }

        int val = 0;
        try {
            val = std::stoi(part);
        } catch (...) {
            throw std::invalid_argument("Invalid octet overflow in IPv4: '" + ipStr + "'");
        }

        if (val < 0 || val > 255) {
            throw std::invalid_argument("Octet out of range [0, 255] in IPv4: '" + ipStr + "'");
        }

        result = (result << 8) | static_cast<uint32_t>(val);
    }

    return result;
}

std::string Packet::ipToString(uint32_t ip) {
    return std::to_string((ip >> 24) & 0xFF) + "." +
           std::to_string((ip >> 16) & 0xFF) + "." +
           std::to_string((ip >> 8) & 0xFF) + "." +
           std::to_string(ip & 0xFF);
}

std::string Packet::getProtocolString() const {
    switch (protocol) {
        case Protocol::TCP:  return "TCP";
        case Protocol::UDP:  return "UDP";
        case Protocol::ICMP: return "ICMP";
        default:             return "UNKNOWN";
    }
}

std::string Packet::getDirectionString() const {
    switch (direction) {
        case Direction::INBOUND:  return "INBOUND";
        case Direction::OUTBOUND: return "OUTBOUND";
        default:                  return "UNKNOWN";
    }
}

std::string Packet::toString() const {
    std::string s = "Pkt #" + std::to_string(id) + " [" + getProtocolString() + " " + getDirectionString() + "] "
                  + ipToString(srcIP) + ":" + std::to_string(srcPort) + " -> "
                  + ipToString(dstIP) + ":" + std::to_string(dstPort)
                  + " (" + std::to_string(payloadSize) + " B)";
    return s;
}
