#include "FirewallEngine.hpp"

using namespace std;

FirewallEngine::FirewallEngine(RuleManager& rm)
    : ruleManager(rm) {
    logger.initialize("firewall_audit.log", "firewall_decisions.csv");
}

EvaluationResult FirewallEngine::processPacket(const Packet& pkt) {
    EvaluationResult result;
    result.packetId = pkt.id;
    result.timestamp = pkt.timestamp;

    if (verboseTrace) {
        std::cout << "\n>>> Inspecting " << pkt.toString() << " ...\n";
    }

    for (const auto& rule : ruleManager.getRules()) {
        if (!rule.isEnabled()) {
            if (verboseTrace) {
                std::cout << "    [Rule #" << rule.getId() << ": " << rule.getName() << "] (DISABLED) -> Skipped\n";
            }
            continue;
        }

        if (rule.matches(pkt)) {
            if (verboseTrace) {
                std::cout << "    [Rule #" << rule.getId() << ": " << rule.getName() << "] MATCH SUCCESSFUL! Action: "
                          << rule.getActionString() << "\n";
            }

            if (rule.getActionType() == ActionType::LOG) {
                // Non-terminal audit tap
                logger.logNonTerminalTap(pkt, rule.getId(), rule.getName());
                statistics.recordLogEvent(rule.getId());
                result.loggedRuleIds.push_back(rule.getId());
                // Continue evaluating remaining rules!
                continue;
            } else if (rule.getActionType() == ActionType::ALLOW) {
                result.finalDecision = Decision::ALLOWED;
                result.matchedRuleId = rule.getId();
                result.matchedRuleName = rule.getName();
                result.reason = "Permitted by matching rule";

                logger.logTerminalDecision(pkt, result.finalDecision, result.matchedRuleId,
                                           result.matchedRuleName, result.reason);
                statistics.recordDecision(result.finalDecision, result.matchedRuleId, pkt.payloadSize);
                return result;
            } else if (rule.getActionType() == ActionType::BLOCK) {
                result.finalDecision = Decision::BLOCKED;
                result.matchedRuleId = rule.getId();
                result.matchedRuleName = rule.getName();
                result.reason = "Silently dropped by matching rule";

                logger.logTerminalDecision(pkt, result.finalDecision, result.matchedRuleId,
                                           result.matchedRuleName, result.reason);
                statistics.recordDecision(result.finalDecision, result.matchedRuleId, pkt.payloadSize);
                return result;
            } else if (rule.getActionType() == ActionType::REJECT) {
                result.finalDecision = Decision::REJECTED;
                result.matchedRuleId = rule.getId();
                result.matchedRuleName = rule.getName();
                result.reason = "Rejected with simulated ICMP Destination Port Unreachable";

                logger.logTerminalDecision(pkt, result.finalDecision, result.matchedRuleId,
                                           result.matchedRuleName, result.reason);
                statistics.recordDecision(result.finalDecision, result.matchedRuleId, pkt.payloadSize);
                return result;
            }
        } else {
            if (verboseTrace) {
                std::cout << "    [Rule #" << rule.getId() << ": " << rule.getName() << "] -> No Match\n";
            }
        }
    }

    // Default Fallback Policy
    result.finalDecision = defaultPolicy;
    result.matchedRuleId = -1;
    result.matchedRuleName = "Default Policy";
    result.reason = (defaultPolicy == Decision::ALLOWED)
                  ? "Permitted by system default policy"
                  : "Discarded by system default-deny policy";

    if (verboseTrace) {
        std::cout << "    [UNMATCHED] Falling back to Default Policy: "
                  << AuditLogger::decisionToString(defaultPolicy) << "\n";
    }

    logger.logTerminalDecision(pkt, result.finalDecision, result.matchedRuleId,
                               result.matchedRuleName, result.reason);
    statistics.recordDecision(result.finalDecision, result.matchedRuleId, pkt.payloadSize);

    return result;
}
