#ifndef RULE_MANAGER_HPP
#define RULE_MANAGER_HPP

#include "FirewallRule.hpp"
#include <vector>

using namespace std;

class RuleManager {
private:
    std::vector<FirewallRule> rules;

public:
    RuleManager() = default;

    bool addRule(FirewallRule rule);
    bool removeRule(int ruleId);
    bool modifyRulePriority(int ruleId, int newPriority);
    bool setRuleEnabled(int ruleId, bool enabled);
    bool updateRule(FirewallRule updatedRule);

    FirewallRule* findRuleById(int ruleId);
    const FirewallRule* findRuleById(int ruleId) const;

    void sortRules();
    void clear() noexcept { rules.clear(); }
    size_t size() const noexcept { return rules.size(); }

    const std::vector<FirewallRule>& getRules() const noexcept { return rules; }
    std::vector<FirewallRule>& getRules() noexcept { return rules; }

    void replaceAll(std::vector<FirewallRule> newRules);
};

#endif // RULE_MANAGER_HPP
