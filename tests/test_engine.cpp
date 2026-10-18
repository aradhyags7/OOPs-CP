#include "Packet.hpp"
#include "RuleCondition.hpp"
#include "FirewallRule.hpp"
#include "RuleManager.hpp"
#include "FirewallEngine.hpp"
#include "ConfigurationManager.hpp"
#include <iostream>
#include <cassert>
#include <string>

int testsPassed = 0;
int testsTotal = 0;

void reportTest(const std::string& testId, const std::string& description, bool passed) {
    testsTotal++;
    if (passed) {
        testsPassed++;
        std::cout << " [PASS] " << testId << ": " << description << "\n";
    } else {
        std::cout << " [FAIL] " << testId << ": " << description << " *FAILED*\n";
    }
}

int main() {
    std::cout << "========================================================================\n";
    std::cout << "            FIREWALL RULE ENGINE - UNIT TEST VERIFICATION               \n";
    std::cout << "========================================================================\n";

    // TC-IP-01: Exact IPv4 Match
    try {
        IPCondition cond("192.168.1.1", IPField::SOURCE);
        Packet p(1, Packet::parseIPv4("192.168.1.1"), 0, 0, 0, Protocol::TCP, Direction::INBOUND);
        reportTest("TC-IP-01", "Exact IPv4 Match (192.168.1.1)", cond.matches(p));
    } catch (...) { reportTest("TC-IP-01", "Exact IPv4 Match", false); }

    // TC-IP-02: Exact IPv4 Mismatch
    try {
        IPCondition cond("192.168.1.1", IPField::SOURCE);
        Packet p(2, Packet::parseIPv4("192.168.1.2"), 0, 0, 0, Protocol::TCP, Direction::INBOUND);
        reportTest("TC-IP-02", "Exact IPv4 Mismatch (192.168.1.2 vs 192.168.1.1)", !cond.matches(p));
    } catch (...) { reportTest("TC-IP-02", "Exact IPv4 Mismatch", false); }

    // TC-IP-03: Invalid Octet Throws
    try {
        Packet::parseIPv4("192.168.1.300");
        reportTest("TC-IP-03", "Invalid Octet Exception Handling", false);
    } catch (const std::invalid_argument&) {
        reportTest("TC-IP-03", "Invalid Octet Exception Handling (192.168.1.300 throws)", true);
    }

    // TC-CIDR-01: /0 Matches All
    try {
        IPCondition cond("0.0.0.0/0", IPField::SOURCE);
        Packet p1(1, Packet::parseIPv4("1.2.3.4"), 0, 0, 0, Protocol::TCP, Direction::INBOUND);
        Packet p2(2, Packet::parseIPv4("255.255.255.255"), 0, 0, 0, Protocol::TCP, Direction::INBOUND);
        reportTest("TC-CIDR-01", "CIDR /0 Matches All Addresses", cond.matches(p1) && cond.matches(p2));
    } catch (...) { reportTest("TC-CIDR-01", "CIDR /0 Matches All", false); }

    // TC-CIDR-02: /32 Exact Match
    try {
        IPCondition cond("10.0.0.1/32", IPField::SOURCE);
        Packet p1(1, Packet::parseIPv4("10.0.0.1"), 0, 0, 0, Protocol::TCP, Direction::INBOUND);
        Packet p2(2, Packet::parseIPv4("10.0.0.2"), 0, 0, 0, Protocol::TCP, Direction::INBOUND);
        reportTest("TC-CIDR-02", "CIDR /32 Matches Exactly One Address", cond.matches(p1) && !cond.matches(p2));
    } catch (...) { reportTest("TC-CIDR-02", "CIDR /32 Matches Exactly One", false); }

    // TC-CIDR-03: Invalid Prefix Throws
    try {
        IPCondition::calculateMask(33);
        reportTest("TC-CIDR-03", "CIDR /33 Throws Exception", false);
    } catch (const std::invalid_argument&) {
        reportTest("TC-CIDR-03", "CIDR /33 Throws Exception (prefix > 32)", true);
    }

    // TC-PORT-01: Port Range Inclusion
    try {
        PortCondition cond(80, 443, PortField::DESTINATION);
        Packet p1(1, 0, 0, 1024, 80, Protocol::TCP, Direction::INBOUND);
        Packet p2(2, 0, 0, 1024, 443, Protocol::TCP, Direction::INBOUND);
        Packet p3(3, 0, 0, 1024, 8080, Protocol::TCP, Direction::INBOUND);
        reportTest("TC-PORT-01", "Port Range Inclusive Matching [80, 443]", cond.matches(p1) && cond.matches(p2) && !cond.matches(p3));
    } catch (...) { reportTest("TC-PORT-01", "Port Range Matching", false); }

    // TC-PORT-02: Inverted Port Range Throws
    try {
        PortCondition cond(8080, 80, PortField::DESTINATION);
        reportTest("TC-PORT-02", "Inverted Port Range (8080-80) Throws Exception", false);
    } catch (const std::invalid_argument&) {
        reportTest("TC-PORT-02", "Inverted Port Range (8080-80) Throws Exception", true);
    }

    // TC-ICMP-01: ICMP Disregards Ports
    try {
        PortCondition cond(80, PortField::DESTINATION);
        Packet p(1, 0, 0, 0, 0, Protocol::ICMP, Direction::INBOUND);
        reportTest("TC-ICMP-01", "ICMP Packets Return False on PortCondition", !cond.matches(p));
    } catch (...) { reportTest("TC-ICMP-01", "ICMP Port Condition", false); }

    // TC-RULE-01: Priority Ordering
    try {
        RuleManager rm;
        FirewallEngine engine(rm);

        FirewallRule r1(1, 10, true, "Allow HTTPS", ActionType::ALLOW);
        r1.addCondition(std::make_unique<PortCondition>(443, PortField::DESTINATION));
        rm.addRule(std::move(r1));

        FirewallRule r2(2, 20, true, "Block All TCP", ActionType::BLOCK);
        r2.addCondition(std::make_unique<ProtocolCondition>(ProtocolMatch::TCP));
        rm.addRule(std::move(r2));

        Packet p(1, 0, 0, 5000, 443, Protocol::TCP, Direction::INBOUND);
        EvaluationResult res = engine.processPacket(p);
        reportTest("TC-RULE-01", "Rule Priority (Priority 10 ALLOW beats Priority 20 BLOCK)", res.finalDecision == Decision::ALLOWED);
    } catch (...) { reportTest("TC-RULE-01", "Rule Priority", false); }

    // TC-RULE-02: Dynamic Priority Mutation & Re-Sorting
    try {
        RuleManager rm;
        FirewallEngine engine(rm);

        FirewallRule r1(1, 10, true, "Allow HTTPS", ActionType::ALLOW);
        r1.addCondition(std::make_unique<PortCondition>(443, PortField::DESTINATION));
        rm.addRule(std::move(r1));

        FirewallRule r2(2, 20, true, "Block All TCP", ActionType::BLOCK);
        r2.addCondition(std::make_unique<ProtocolCondition>(ProtocolMatch::TCP));
        rm.addRule(std::move(r2));

        rm.modifyRulePriority(2, 5); // Rule 2 now has Priority 5!

        Packet p(1, 0, 0, 5000, 443, Protocol::TCP, Direction::INBOUND);
        EvaluationResult res = engine.processPacket(p);
        reportTest("TC-RULE-02", "Priority Mutation & Auto Re-Sorting (Rule 2 elevated to Pri 5 -> BLOCK)", res.finalDecision == Decision::BLOCKED);
    } catch (...) { reportTest("TC-RULE-02", "Priority Mutation", false); }

    // TC-RULE-03: Disabled Rule Skipped
    try {
        RuleManager rm;
        FirewallEngine engine(rm);

        FirewallRule r1(1, 10, false, "Disabled Allow", ActionType::ALLOW); // Disabled
        r1.addCondition(std::make_unique<PortCondition>(443, PortField::DESTINATION));
        rm.addRule(std::move(r1));

        FirewallRule r2(2, 20, true, "Active Block", ActionType::BLOCK);
        r2.addCondition(std::make_unique<ProtocolCondition>(ProtocolMatch::TCP));
        rm.addRule(std::move(r2));

        Packet p(1, 0, 0, 5000, 443, Protocol::TCP, Direction::INBOUND);
        EvaluationResult res = engine.processPacket(p);
        reportTest("TC-RULE-03", "Disabled Rules Are Skipped During Evaluation", res.finalDecision == Decision::BLOCKED);
    } catch (...) { reportTest("TC-RULE-03", "Disabled Rule Skipped", false); }

    // TC-RULE-04: Empty Rule Rejection
    try {
        RuleManager rm;
        FirewallRule r(1, 10, true, "Empty Rule", ActionType::ALLOW);
        bool added = rm.addRule(std::move(r));
        reportTest("TC-RULE-04", "Rules with 0 Conditions Are Rejected on Add", !added);
    } catch (...) { reportTest("TC-RULE-04", "Empty Rule Rejection", false); }

    // TC-LOG-01: Non-Terminal LOG Tap Execution
    try {
        RuleManager rm;
        FirewallEngine engine(rm);

        FirewallRule r1(1, 10, true, "Audit Tap", ActionType::LOG);
        r1.addCondition(std::make_unique<PortCondition>(23, PortField::DESTINATION));
        rm.addRule(std::move(r1));

        FirewallRule r2(2, 20, true, "Terminal Reject", ActionType::REJECT);
        r2.addCondition(std::make_unique<PortCondition>(23, PortField::DESTINATION));
        rm.addRule(std::move(r2));

        Packet p(1, 0, 0, 5000, 23, Protocol::TCP, Direction::INBOUND);
        EvaluationResult res = engine.processPacket(p);
        reportTest("TC-LOG-01", "LOG Tap Recorded and Evaluation Continued to Terminal REJECT",
                   res.finalDecision == Decision::REJECTED && res.loggedRuleIds.size() == 1 && res.loggedRuleIds[0] == 1);
    } catch (...) { reportTest("TC-LOG-01", "LOG Tap Execution", false); }

    // TC-RULE-05: Dynamic Rule Mutation In-Place Modifies Verdict
    try {
        RuleManager rm;
        FirewallEngine engine(rm);

        // Initially Rule 1 permits TCP 80
        FirewallRule r1(1, 10, true, "Permit Web Traffic", ActionType::ALLOW);
        r1.addCondition(std::make_unique<PortCondition>(80, PortField::DESTINATION));
        rm.addRule(std::move(r1));

        Packet p(1, 0, 0, 5000, 80, Protocol::TCP, Direction::INBOUND);
        EvaluationResult initialRes = engine.processPacket(p);

        // Mutate Rule 1 to BLOCK
        FirewallRule updatedR1(1, 10, true, "Block Web Traffic", ActionType::BLOCK);
        updatedR1.addCondition(std::make_unique<PortCondition>(80, PortField::DESTINATION));
        bool updated = rm.updateRule(std::move(updatedR1));

        EvaluationResult postMutationRes = engine.processPacket(p);

        reportTest("TC-RULE-05", "Dynamic Rule Mutation In-Place Alters Packet Verdict (ALLOW -> BLOCK)",
                   updated && initialRes.finalDecision == Decision::ALLOWED && postMutationRes.finalDecision == Decision::BLOCKED);
    } catch (...) { reportTest("TC-RULE-05", "Dynamic Rule Mutation", false); }

    // TC-FALL-01: Default Deny Policy Fallback
    try {
        RuleManager rm;
        FirewallEngine engine(rm);
        engine.setDefaultPolicy(Decision::BLOCKED);

        Packet p(1, 0, 0, 5000, 9999, Protocol::TCP, Direction::INBOUND);
        EvaluationResult res = engine.processPacket(p);
        reportTest("TC-FALL-01", "Unmatched Packets Trigger Default Deny Policy",
                   res.finalDecision == Decision::BLOCKED && res.matchedRuleId == -1);
    } catch (...) { reportTest("TC-FALL-01", "Default Deny Fallback", false); }

    std::cout << "========================================================================\n";
    std::cout << " Test Results: " << testsPassed << " / " << testsTotal << " Passed ("
              << (testsPassed * 100 / testsTotal) << "% Success Rate)\n";
    std::cout << "========================================================================\n";

    return (testsPassed == testsTotal) ? 0 : 1;
}
