#ifndef FIREWALL_RULE_HPP
#define FIREWALL_RULE_HPP

#include "RuleCondition.hpp"
#include <vector>
#include <memory>
#include <string>

using namespace std;

enum class ActionType {
    ALLOW,
    BLOCK,
    REJECT,
    LOG
};

class FirewallRule {
private:
    int id{0};
    int priority{100};
    bool enabled{true};
    std::string name;
    ActionType actionType{ActionType::BLOCK};
    std::vector<std::unique_ptr<RuleCondition>> conditions;

public:
    FirewallRule(int rId, int rPriority, bool isEnabled, std::string rName, ActionType action);

    // Move-only semantics
    FirewallRule(FirewallRule&&) noexcept = default;
    FirewallRule& operator=(FirewallRule&&) noexcept = default;
    FirewallRule(const FirewallRule&) = delete;
    FirewallRule& operator=(const FirewallRule&) = delete;

    void addCondition(std::unique_ptr<RuleCondition> cond);
    bool matches(const Packet& pkt) const;

    int getId() const noexcept { return id; }
    int getPriority() const noexcept { return priority; }
    void setPriority(int newPriority) noexcept { priority = newPriority; }
    bool isEnabled() const noexcept { return enabled; }
    void setEnabled(bool state) noexcept { enabled = state; }
    const std::string& getName() const noexcept { return name; }
    void setName(const std::string& newName) { name = newName; }
    ActionType getActionType() const noexcept { return actionType; }
    void setActionType(ActionType act) noexcept { actionType = act; }

    const std::vector<std::unique_ptr<RuleCondition>>& getConditions() const noexcept {
        return conditions;
    }
    size_t getConditionCount() const noexcept { return conditions.size(); }

    std::string getActionString() const;
    std::string getConditionsSummary() const;
};

#endif // FIREWALL_RULE_HPP
