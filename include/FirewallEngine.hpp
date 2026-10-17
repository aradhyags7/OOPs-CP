#ifndef FIREWALL_ENGINE_HPP
#define FIREWALL_ENGINE_HPP

#include "Packet.hpp"
#include "RuleManager.hpp"
#include "AuditLogger.hpp"
#include "Statistics.hpp"
#include <vector>
#include <string>

struct EvaluationResult {
    uint64_t packetId{0};
    Decision finalDecision{Decision::BLOCKED};
    int matchedRuleId{-1};
    std::string matchedRuleName{"Default Policy"};
    std::string reason;
    std::vector<int> loggedRuleIds;
    double timestamp{0.0};
};

class FirewallEngine {
private:
    RuleManager& ruleManager;
    Decision defaultPolicy{Decision::BLOCKED};
    AuditLogger logger;
    Statistics statistics;
    bool verboseTrace{false};

public:
    explicit FirewallEngine(RuleManager& rm);

    EvaluationResult processPacket(const Packet& pkt);

    void setDefaultPolicy(Decision policy) noexcept { defaultPolicy = policy; }
    Decision getDefaultPolicy() const noexcept { return defaultPolicy; }

    RuleManager& getRuleManager() noexcept { return ruleManager; }
    const RuleManager& getRuleManager() const noexcept { return ruleManager; }

    Statistics& getStatistics() noexcept { return statistics; }
    const Statistics& getStatistics() const noexcept { return statistics; }

    AuditLogger& getLogger() noexcept { return logger; }
    void setVerbose(bool v) noexcept { verboseTrace = v; }
};

#endif // FIREWALL_ENGINE_HPP
