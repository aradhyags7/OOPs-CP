#include "RuleCondition.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

uint32_t IPCondition::calculateMask(uint32_t pfx) {
    if (pfx > 32) {
        throw std::invalid_argument("CIDR prefix must be between 0 and 32, got: " + std::to_string(pfx));
    }
    if (pfx == 0) {
        return 0u;
    }
    return 0xFFFFFFFFu << (32u - pfx);
}

IPCondition::IPCondition(const std::string& cidrOrIP, IPField field)
    : targetField(field), originalSpec(cidrOrIP) {
    auto slashPos = cidrOrIP.find('/');
    if (slashPos != std::string::npos) {
        std::string ipPart = cidrOrIP.substr(0, slashPos);
        std::string prefixPart = cidrOrIP.substr(slashPos + 1);

        if (prefixPart.empty()) {
            throw std::invalid_argument("Empty CIDR prefix in: '" + cidrOrIP + "'");
        }
        for (char c : prefixPart) {
            if (c < '0' || c > '9') {
                throw std::invalid_argument("Invalid non-numeric CIDR prefix: '" + cidrOrIP + "'");
            }
        }

        int p = std::stoi(prefixPart);
        if (p < 0 || p > 32) {
            throw std::invalid_argument("CIDR prefix out of range [0, 32]: '" + cidrOrIP + "'");
        }
        prefix = static_cast<uint32_t>(p);
        uint32_t rawIP = Packet::parseIPv4(ipPart);
        subnetMask = calculateMask(prefix);
        networkIP = rawIP & subnetMask;
    } else {
        prefix = 32;
        networkIP = Packet::parseIPv4(cidrOrIP);
        subnetMask = 0xFFFFFFFFu;
    }
}

bool IPCondition::matches(const Packet& pkt) const {
    uint32_t targetIP = (targetField == IPField::SOURCE) ? pkt.srcIP : pkt.dstIP;
    return (targetIP & subnetMask) == (networkIP & subnetMask);
}

std::string IPCondition::getDescription() const {
    std::string s = (targetField == IPField::SOURCE ? "SrcIP: " : "DstIP: ");
    s += Packet::ipToString(networkIP) + "/" + std::to_string(prefix);
    return s;
}

PortCondition::PortCondition(uint16_t port, PortField field)
    : minPort(port), maxPort(port), targetField(field) {}

PortCondition::PortCondition(uint16_t minP, uint16_t maxP, PortField field)
    : minPort(minP), maxPort(maxP), targetField(field) {
    if (minP > maxP) {
        throw std::invalid_argument("Invalid port range (minPort > maxPort): "
                                    + std::to_string(minP) + "-" + std::to_string(maxP));
    }
}

bool PortCondition::matches(const Packet& pkt) const {
    if (pkt.protocol == Protocol::ICMP) {
        return false; // ICMP packets do not have transport ports
    }
    uint16_t port = (targetField == PortField::SOURCE) ? pkt.srcPort : pkt.dstPort;
    return port >= minPort && port <= maxPort;
}

std::string PortCondition::getDescription() const {
    std::string s = (targetField == PortField::SOURCE ? "SrcPort: " : "DstPort: ");
    if (minPort == maxPort) {
        s += std::to_string(minPort);
    } else {
        s += std::to_string(minPort) + "-" + std::to_string(maxPort);
    }
    return s;
}

ProtocolCondition::ProtocolCondition(ProtocolMatch proto)
    : matchProto(proto) {}

bool ProtocolCondition::matches(const Packet& pkt) const {
    if (matchProto == ProtocolMatch::ANY) return true;
    if (matchProto == ProtocolMatch::TCP && pkt.protocol == Protocol::TCP) return true;
    if (matchProto == ProtocolMatch::UDP && pkt.protocol == Protocol::UDP) return true;
    if (matchProto == ProtocolMatch::ICMP && pkt.protocol == Protocol::ICMP) return true;
    return false;
}

std::string ProtocolCondition::getDescription() const {
    std::string s = "Proto: ";
    switch (matchProto) {
        case ProtocolMatch::TCP:  s += "TCP"; break;
        case ProtocolMatch::UDP:  s += "UDP"; break;
        case ProtocolMatch::ICMP: s += "ICMP"; break;
        case ProtocolMatch::ANY:  s += "ANY"; break;
    }
    return s;
}

DirectionCondition::DirectionCondition(DirectionMatch dir)
    : matchDir(dir) {}

bool DirectionCondition::matches(const Packet& pkt) const {
    if (matchDir == DirectionMatch::ANY) return true;
    if (matchDir == DirectionMatch::INBOUND && pkt.direction == Direction::INBOUND) return true;
    if (matchDir == DirectionMatch::OUTBOUND && pkt.direction == Direction::OUTBOUND) return true;
    return false;
}

std::string DirectionCondition::getDescription() const {
    std::string s = "Dir: ";
    switch (matchDir) {
        case DirectionMatch::INBOUND:  s += "INBOUND"; break;
        case DirectionMatch::OUTBOUND: s += "OUTBOUND"; break;
        case DirectionMatch::ANY:      s += "ANY"; break;
    }
    return s;
}
