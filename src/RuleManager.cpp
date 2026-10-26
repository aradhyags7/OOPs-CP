#include "RuleManager.hpp"
#include <algorithm>
#include <iostream>

using namespace std;

bool RuleManager::addRule(FirewallRule rule) {
    if (findRuleById(rule.getId()) != nullptr) {
        std::cerr << "[RuleManager Error] Rule with ID " << rule.getId() << " already exists.\n";
        return false;
    }
    if (rule.getConditionCount() == 0) {
        std::cerr << "[RuleManager Error] Cannot add rule #" << rule.getId() << " with zero conditions.\n";
        return false;
    }
    rules.push_back(std::move(rule));
    sortRules();
    return true;
}

bool RuleManager::removeRule(int ruleId) {
    auto it = std::find_if(rules.begin(), rules.end(), [ruleId](const FirewallRule& r) {
        return r.getId() == ruleId;
    });
    if (it != rules.end()) {
        rules.erase(it);
        return true;
    }
    return false;
}

bool RuleManager::modifyRulePriority(int ruleId, int newPriority) {
    FirewallRule* rule = findRuleById(ruleId);
    if (!rule) {
        return false;
    }
    rule->setPriority(newPriority);
    sortRules();
    return true;
}

bool RuleManager::setRuleEnabled(int ruleId, bool enabled) {
    FirewallRule* rule = findRuleById(ruleId);
    if (!rule) {
        return false;
    }
    rule->setEnabled(enabled);
    return true;
}

bool RuleManager::updateRule(FirewallRule updatedRule) {
    auto it = std::find_if(rules.begin(), rules.end(), [id = updatedRule.getId()](const FirewallRule& r) {
        return r.getId() == id;
    });
    if (it == rules.end()) {
        std::cerr << "[RuleManager Error] Cannot update rule #" << updatedRule.getId() << ": Not found.\n";
        return false;
    }
    if (updatedRule.getConditionCount() == 0) {
        std::cerr << "[RuleManager Error] Cannot update rule #" << updatedRule.getId() << " with zero conditions.\n";
        return false;
    }
    *it = std::move(updatedRule);
    sortRules();
    return true;
}

FirewallRule* RuleManager::findRuleById(int ruleId) {
    for (auto& r : rules) {
        if (r.getId() == ruleId) {
            return &r;
        }
    }
    return nullptr;
}

const FirewallRule* RuleManager::findRuleById(int ruleId) const {
    for (const auto& r : rules) {
        if (r.getId() == ruleId) {
            return &r;
        }
    }
    return nullptr;
}

void RuleManager::sortRules() {
    std::stable_sort(rules.begin(), rules.end(), [](const FirewallRule& a, const FirewallRule& b) {
        return a.getPriority() < b.getPriority();
    });
}

void RuleManager::replaceAll(std::vector<FirewallRule> newRules) {
    rules = std::move(newRules);
    sortRules();
}
