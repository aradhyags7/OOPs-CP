#ifndef RULE_CONDITION_HPP
#define RULE_CONDITION_HPP

#include "Packet.hpp"
#include <string>
#include <cstdint>

enum class IPField {
    SOURCE,
    DESTINATION
};

enum class PortField {
    SOURCE,
    DESTINATION
};

enum class ProtocolMatch {
    TCP,
    UDP,
    ICMP,
    ANY
};

enum class DirectionMatch {
    INBOUND,
    OUTBOUND,
    ANY
};

class RuleCondition {
public:
    virtual ~RuleCondition() = default;
    virtual bool matches(const Packet& pkt) const = 0;
    virtual std::string getDescription() const = 0;
};

class IPCondition : public RuleCondition {
private:
    uint32_t networkIP{0};
    uint32_t subnetMask{0xFFFFFFFFu};
    uint32_t prefix{32};
    IPField targetField{IPField::SOURCE};
    std::string originalSpec;

public:
    IPCondition(const std::string& cidrOrIP, IPField field);
    bool matches(const Packet& pkt) const override;
    std::string getDescription() const override;

    uint32_t getNetworkIP() const noexcept { return networkIP; }
    uint32_t getSubnetMask() const noexcept { return subnetMask; }
    uint32_t getPrefix() const noexcept { return prefix; }
    IPField getTargetField() const noexcept { return targetField; }

    static uint32_t calculateMask(uint32_t pfx);
};

class PortCondition : public RuleCondition {
private:
    uint16_t minPort{0};
    uint16_t maxPort{0};
    PortField targetField{PortField::DESTINATION};

public:
    PortCondition(uint16_t port, PortField field);
    PortCondition(uint16_t minP, uint16_t maxP, PortField field);
    bool matches(const Packet& pkt) const override;
    std::string getDescription() const override;

    uint16_t getMinPort() const noexcept { return minPort; }
    uint16_t getMaxPort() const noexcept { return maxPort; }
    PortField getTargetField() const noexcept { return targetField; }
};

class ProtocolCondition : public RuleCondition {
private:
    ProtocolMatch matchProto{ProtocolMatch::ANY};

public:
    explicit ProtocolCondition(ProtocolMatch proto);
    bool matches(const Packet& pkt) const override;
    std::string getDescription() const override;

    ProtocolMatch getProtocolMatch() const noexcept { return matchProto; }
};

class DirectionCondition : public RuleCondition {
private:
    DirectionMatch matchDir{DirectionMatch::ANY};

public:
    explicit DirectionCondition(DirectionMatch dir);
    bool matches(const Packet& pkt) const override;
    std::string getDescription() const override;

    DirectionMatch getDirectionMatch() const noexcept { return matchDir; }
};

#endif // RULE_CONDITION_HPP
