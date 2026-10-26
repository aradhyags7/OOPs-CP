#include "FirewallRule.hpp"
#include <stdexcept>

using namespace std;

FirewallRule::FirewallRule(int rId, int rPriority, bool isEnabled, std::string rName, ActionType action)
    : id(rId), priority(rPriority), enabled(isEnabled), name(std::move(rName)), actionType(action) {
    if (name.empty()) {
        name = "Rule_" + std::to_string(id);
    }
}

void FirewallRule::addCondition(std::unique_ptr<RuleCondition> cond) {
    if (!cond) {
        throw std::invalid_argument("Cannot add null RuleCondition to FirewallRule #" + std::to_string(id));
    }
    conditions.push_back(std::move(cond));
}

bool FirewallRule::matches(const Packet& pkt) const {
    if (!enabled) {
        return false;
    }
    if (conditions.empty()) {
        return false; // Rules without conditions do not match by policy
    }
    for (const auto& cond : conditions) {
        if (!cond->matches(pkt)) {
            return false;
        }
    }
    return true;
}

std::string FirewallRule::getActionString() const {
    switch (actionType) {
        case ActionType::ALLOW:  return "ALLOW";
        case ActionType::BLOCK:  return "BLOCK";
        case ActionType::REJECT: return "REJECT";
        case ActionType::LOG:    return "LOG";
        default:                 return "UNKNOWN";
    }
}

std::string FirewallRule::getConditionsSummary() const {
    if (conditions.empty()) {
        return "None";
    }
    std::string summary;
    for (size_t i = 0; i < conditions.size(); ++i) {
        summary += conditions[i]->getDescription();
        if (i + 1 < conditions.size()) {
            summary += ", ";
        }
    }
    return summary;
}
