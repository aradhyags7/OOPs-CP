#include "FirewallEngine.hpp"
#include "SimulationRunner.hpp"
#include "ConfigurationManager.hpp"
#include "PacketGenerator.hpp"
#include <iostream>
#include <limits>
#include <string>

void displayMenu() {
    std::cout << "\n========================================================================\n";
    std::cout << "                 FIREWALL RULE ENGINE - C++17 OOP                       \n";
    std::cout << "========================================================================\n";
    std::cout << " 1. Display Active Security Rules\n";
    std::cout << " 2. Add New Security Policy Rule\n";
    std::cout << " 3. Modify Rule Priority (Auto-Resorts)\n";
    std::cout << " 4. Enable / Disable Rule\n";
    std::cout << " 5. Delete Security Rule\n";
    std::cout << " 6. Test Single Packet Inspection\n";
    std::cout << " 7. Run Demonstration Scenarios (Scenarios A - G)\n";
    std::cout << " 8. View Firewall Telemetry & Statistics\n";
    std::cout << " 9. Load Rules from Configuration File (Transactional)\n";
    std::cout << " 10. Save Rules to Configuration File\n";
    std::cout << " 11. Run Built-in Unit Test Suite\n";
    std::cout << " 12. Exit\n";
    std::cout << "========================================================================\n";
    std::cout << " Select an option (1-12): ";
}

void printRulesTable(const RuleManager& rm, Decision defaultPolicy) {
    std::cout << "\n========================================================================================================\n";
    std::cout << "                                      CONFIGURED FIREWALL POLICIES                                      \n";
    std::cout << "========================================================================================================\n";
    std::cout << std::left << std::setw(6) << "ID"
              << std::setw(10) << "Priority"
              << std::setw(10) << "Status"
              << std::setw(10) << "Action"
              << std::setw(28) << "Name"
              << "Matching Conditions\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n";

    if (rm.size() == 0) {
        std::cout << "  (No security rules configured. All traffic falls back to default policy)\n";
    } else {
        for (const auto& r : rm.getRules()) {
            std::cout << std::left << std::setw(6) << r.getId()
                      << std::setw(10) << r.getPriority()
                      << std::setw(10) << (r.isEnabled() ? "ENABLED" : "DISABLED")
                      << std::setw(10) << r.getActionString()
                      << std::setw(28) << r.getName()
                      << r.getConditionsSummary() << "\n";
        }
    }
    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    std::cout << " System Default Policy: " << AuditLogger::decisionToString(defaultPolicy)
              << " (Applies to all unmatched packets)\n";
    std::cout << "========================================================================================================\n\n";
}

void handleManualPacket(FirewallEngine& engine) {
    std::cout << "\nEnter Packet Details for Inspection:\n";
    std::string srcIP, dstIP, protoStr, dirStr;
    uint16_t srcPort = 0, dstPort = 0;

    std::cout << " Source IPv4 (e.g. 192.168.1.50): ";
    std::cin >> srcIP;
    std::cout << " Destination IPv4 (e.g. 10.0.0.1): ";
    std::cin >> dstIP;
    std::cout << " Protocol (TCP, UDP, ICMP): ";
    std::cin >> protoStr;

    Protocol proto = Protocol::TCP;
    if (protoStr == "UDP") proto = Protocol::UDP;
    else if (protoStr == "ICMP") proto = Protocol::ICMP;

    if (proto != Protocol::ICMP) {
        std::cout << " Source Port (0-65535): ";
        std::cin >> srcPort;
        std::cout << " Destination Port (0-65535): ";
        std::cin >> dstPort;
    }

    std::cout << " Direction (INBOUND, OUTBOUND): ";
    std::cin >> dirStr;
    Direction dir = (dirStr == "OUTBOUND") ? Direction::OUTBOUND : Direction::INBOUND;

    try {
        Packet pkt(999, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), srcPort, dstPort, proto, dir, 512, 0.0);
        engine.setVerbose(true);
        EvaluationResult res = engine.processPacket(pkt);
        engine.setVerbose(false);

        std::cout << "\n=================== FINAL INSPECTION VERDICT ===================\n";
        std::cout << " Packet:   " << pkt.toString() << "\n";
        std::cout << " Verdict:  " << AuditLogger::decisionToString(res.finalDecision) << "\n";
        std::cout << " Rule ID:  " << res.matchedRuleId << " (" << res.matchedRuleName << ")\n";
        std::cout << " Reason:   " << res.reason << "\n";
        if (!res.loggedRuleIds.empty()) {
            std::cout << " Log Taps: Triggered Rule(s) ";
            for (int id : res.loggedRuleIds) std::cout << "#" << id << " ";
            std::cout << "\n";
        }
        std::cout << "================================================================\n";
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to process packet: " << e.what() << "\n";
    }
}

int handleJsonCli(int argc, char* argv[]) {
    RuleManager ruleManager;
    FirewallEngine engine(ruleManager);
    SimulationRunner runner(engine);

    std::string confPath = "rules/sample_rules.conf";
    std::string statsPath = "state/engine_stats.conf";
    std::string err;
    ConfigurationManager::loadConfiguration(confPath, ruleManager, err);
    engine.getStatistics().loadFromFile(statsPath);
    engine.getLogger().setConsoleOutput(false);

    std::string cmd = argv[1];

    if (cmd == "--json-rules") {
        std::cout << "{\n  \"status\": \"success\",\n  \"defaultPolicy\": \""
                  << AuditLogger::decisionToString(engine.getDefaultPolicy()) << "\",\n  \"rules\": [\n";
        const auto& rules = ruleManager.getRules();
        for (size_t i = 0; i < rules.size(); ++i) {
            const auto& r = rules[i];
            std::cout << "    {\n"
                      << "      \"id\": " << r.getId() << ",\n"
                      << "      \"priority\": " << r.getPriority() << ",\n"
                      << "      \"enabled\": " << (r.isEnabled() ? "true" : "false") << ",\n"
                      << "      \"action\": \"" << r.getActionString() << "\",\n"
                      << "      \"name\": \"" << r.getName() << "\",\n"
                      << "      \"conditions\": \"" << r.getConditionsSummary() << "\"\n"
                      << "    }" << (i + 1 < rules.size() ? "," : "") << "\n";
        }
        std::cout << "  ]\n}\n";
        return 0;
    }

    if (cmd == "--json-inspect") {
        if (argc < 8) {
            std::cout << "{\"status\": \"error\", \"message\": \"Usage: --json-inspect <srcIP> <dstIP> <srcPort> <dstPort> <proto> <dir>\"}\n";
            return 1;
        }
        std::string srcIP = argv[2];
        std::string dstIP = argv[3];
        uint16_t srcPort = static_cast<uint16_t>(std::stoi(argv[4]));
        uint16_t dstPort = static_cast<uint16_t>(std::stoi(argv[5]));
        std::string protoStr = argv[6];
        std::string dirStr = argv[7];

        Protocol proto = Protocol::TCP;
        if (protoStr == "UDP") proto = Protocol::UDP;
        else if (protoStr == "ICMP") proto = Protocol::ICMP;

        Direction dir = (dirStr == "OUTBOUND") ? Direction::OUTBOUND : Direction::INBOUND;

        try {
            Packet pkt(1, Packet::parseIPv4(srcIP), Packet::parseIPv4(dstIP), srcPort, dstPort, proto, dir, 512, 0.0);
            EvaluationResult res = engine.processPacket(pkt);

            // Persist updated engine statistics
            engine.getStatistics().saveToFile(statsPath);

            std::cout << "{\n  \"status\": \"success\",\n"
                      << "  \"packet\": {\n"
                      << "    \"srcIP\": \"" << srcIP << "\",\n"
                      << "    \"dstIP\": \"" << dstIP << "\",\n"
                      << "    \"srcPort\": " << srcPort << ",\n"
                      << "    \"dstPort\": " << dstPort << ",\n"
                      << "    \"protocol\": \"" << protoStr << "\",\n"
                      << "    \"direction\": \"" << dirStr << "\"\n"
                      << "  },\n"
                      << "  \"decision\": \"" << AuditLogger::decisionToString(res.finalDecision) << "\",\n"
                      << "  \"matchedRuleId\": " << res.matchedRuleId << ",\n"
                      << "  \"matchedRuleName\": \"" << res.matchedRuleName << "\",\n"
                      << "  \"reason\": \"" << res.reason << "\",\n"
                      << "  \"loggedRuleIds\": [";
            for (size_t i = 0; i < res.loggedRuleIds.size(); ++i) {
                std::cout << res.loggedRuleIds[i] << (i + 1 < res.loggedRuleIds.size() ? ", " : "");
            }
            std::cout << "]\n}\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "{\"status\": \"error\", \"message\": \"" << e.what() << "\"}\n";
            return 1;
        }
    }

    if (cmd == "--json-stats") {
        engine.getStatistics().loadFromFile(statsPath);
        const auto& stats = engine.getStatistics();
        std::cout << "{\n  \"status\": \"success\",\n"
                  << "  \"packetsProcessed\": " << stats.getPacketsProcessed() << ",\n"
                  << "  \"packetsAllowed\": " << stats.getPacketsAllowed() << ",\n"
                  << "  \"packetsBlocked\": " << stats.getPacketsBlocked() << ",\n"
                  << "  \"packetsRejected\": " << stats.getPacketsRejected() << ",\n"
                  << "  \"loggingEvents\": " << stats.getLoggingEventsRecorded() << ",\n"
                  << "  \"defaultPolicyHits\": " << stats.getDefaultPolicyHits() << ",\n"
                  << "  \"totalBytes\": " << stats.getTotalBytesProcessed() << ",\n"
                  << "  \"ruleHitCounts\": {\n";
        const auto& hits = stats.getRuleHitCounts();
        size_t count = 0;
        for (const auto& kv : hits) {
            std::cout << "    \"" << kv.first << "\": " << kv.second
                      << (++count < hits.size() ? "," : "") << "\n";
        }
        std::cout << "  }\n}\n";
        return 0;
    }

    if (cmd == "--json-reset-stats") {
        engine.getStatistics().reset();
        engine.getStatistics().saveToFile(statsPath);
        std::cout << "{\"status\": \"success\", \"message\": \"Statistics reset to zero\"}\n";
        return 0;
    }

    if (cmd == "--json-scenario") {
        std::string sc = (argc > 2) ? argv[2] : "G";
        size_t count = (argc > 3) ? std::stoull(argv[3]) : 50000;

        Statistics cumulativeStats;
        cumulativeStats.loadFromFile(statsPath);

        engine.getLogger().setConsoleOutput(false);
        engine.getLogger().setFileOutput(true);
        engine.getLogger().setCsvOutput(true);

        auto t0 = std::chrono::high_resolution_clock::now();
        if (sc == "A") runner.runScenarioA_BasicFiltering();
        else if (sc == "B") runner.runScenarioB_PortFiltering();
        else if (sc == "C") runner.runScenarioC_RulePriority();
        else if (sc == "D") runner.runScenarioD_SubnetFiltering();
        else if (sc == "E") runner.runScenarioE_AuditLogging();
        else if (sc == "F") runner.runScenarioF_ConfigurationPersistence();
        else runner.runScenarioG_BatchBenchmark(count);
        auto t1 = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = t1 - t0;
        double sec = elapsed.count();
        const auto& stats = engine.getStatistics();
        uint64_t total = stats.getPacketsProcessed();
        double pps = (sec > 0.0) ? (static_cast<double>(total) / sec) : 0.0;

        // Accumulate and persist total metrics
        cumulativeStats.accumulate(stats);
        cumulativeStats.saveToFile(statsPath);

        std::cout << "{\n  \"status\": \"success\",\n"
                  << "  \"scenario\": \"" << sc << "\",\n"
                  << "  \"elapsedSeconds\": " << sec << ",\n"
                  << "  \"throughputPps\": " << pps << ",\n"
                  << "  \"packetsProcessed\": " << stats.getPacketsProcessed() << ",\n"
                  << "  \"packetsAllowed\": " << stats.getPacketsAllowed() << ",\n"
                  << "  \"packetsBlocked\": " << stats.getPacketsBlocked() << ",\n"
                  << "  \"packetsRejected\": " << stats.getPacketsRejected() << ",\n"
                  << "  \"ruleHitCounts\": {\n";
        const auto& hits = stats.getRuleHitCounts();
        size_t hCount = 0;
        for (const auto& kv : hits) {
            std::cout << "    \"" << kv.first << "\": " << kv.second
                      << (++hCount < hits.size() ? "," : "") << "\n";
        }
        std::cout << "  }\n}\n";
        return 0;
    }

    if (cmd == "--json-toggle-rule") {
        if (argc < 4) {
            std::cout << "{\"status\": \"error\", \"message\": \"Usage: --json-toggle-rule <id> <0|1>\"}\n";
            return 1;
        }
        int rId = std::stoi(argv[2]);
        bool enable = (std::stoi(argv[3]) != 0);
        if (!ruleManager.setRuleEnabled(rId, enable)) {
            std::cout << "{\"status\": \"error\", \"message\": \"Rule ID not found\"}\n";
            return 1;
        }
        std::string saveErr;
        ConfigurationManager::saveConfiguration(confPath, ruleManager, saveErr);
        std::cout << "{\"status\": \"success\", \"ruleId\": " << rId << ", \"enabled\": " << (enable ? "true" : "false") << "}\n";
        return 0;
    }

    if (cmd == "--json-set-priority") {
        if (argc < 4) {
            std::cout << "{\"status\": \"error\", \"message\": \"Usage: --json-set-priority <id> <priority>\"}\n";
            return 1;
        }
        int rId = std::stoi(argv[2]);
        int prio = std::stoi(argv[3]);
        if (!ruleManager.modifyRulePriority(rId, prio)) {
            std::cout << "{\"status\": \"error\", \"message\": \"Rule ID not found\"}\n";
            return 1;
        }
        std::string saveErr;
        ConfigurationManager::saveConfiguration(confPath, ruleManager, saveErr);
        std::cout << "{\"status\": \"success\", \"ruleId\": " << rId << ", \"priority\": " << prio << "}\n";
        return 0;
    }

    if (cmd == "--json-delete-rule") {
        if (argc < 3) {
            std::cout << "{\"status\": \"error\", \"message\": \"Usage: --json-delete-rule <id>\"}\n";
            return 1;
        }
        int rId = std::stoi(argv[2]);
        if (!ruleManager.removeRule(rId)) {
            std::cout << "{\"status\": \"error\", \"message\": \"Rule ID not found\"}\n";
            return 1;
        }
        std::string saveErr;
        ConfigurationManager::saveConfiguration(confPath, ruleManager, saveErr);
        std::cout << "{\"status\": \"success\", \"ruleId\": " << rId << "}\n";
        return 0;
    }

    if (cmd == "--json-add-rule") {
        // Usage: --json-add-rule <id> <priority> <action> <name> <proto> <dstPort> <srcCidr> <dir>
        if (argc < 10) {
            std::cout << "{\"status\": \"error\", \"message\": \"Usage: --json-add-rule <id> <priority> <action> <name> <proto> <dstPort> <srcCidr> <dir>\"}\n";
            return 1;
        }
        try {
            int id = std::stoi(argv[2]);
            int priority = std::stoi(argv[3]);
            std::string actionStr = argv[4];
            std::string name = argv[5];
            std::string protoStr = argv[6];
            int port = std::stoi(argv[7]);
            std::string cidr = argv[8];
            std::string dirStr = argv[9];

            ActionType act = ActionType::BLOCK;
            if (actionStr == "ALLOW") act = ActionType::ALLOW;
            else if (actionStr == "REJECT") act = ActionType::REJECT;
            else if (actionStr == "LOG") act = ActionType::LOG;

            FirewallRule rule(id, priority, true, name, act);

            if (protoStr != "ANY") {
                ProtocolMatch pm = ProtocolMatch::ANY;
                if (protoStr == "TCP") pm = ProtocolMatch::TCP;
                else if (protoStr == "UDP") pm = ProtocolMatch::UDP;
                else if (protoStr == "ICMP") pm = ProtocolMatch::ICMP;
                rule.addCondition(std::make_unique<ProtocolCondition>(pm));
            }

            if (port > 0 && port <= 65535) {
                rule.addCondition(std::make_unique<PortCondition>(static_cast<uint16_t>(port), PortField::DESTINATION));
            }

            if (cidr != "ANY" && !cidr.empty()) {
                rule.addCondition(std::make_unique<IPCondition>(cidr, IPField::SOURCE));
            }

            if (dirStr != "ANY") {
                DirectionMatch dm = (dirStr == "OUTBOUND") ? DirectionMatch::OUTBOUND : DirectionMatch::INBOUND;
                rule.addCondition(std::make_unique<DirectionCondition>(dm));
            }

            if (!ruleManager.addRule(std::move(rule))) {
                std::cout << "{\"status\": \"error\", \"message\": \"Failed to add rule (duplicate ID or 0 conditions)\"}\n";
                return 1;
            }

            std::string saveErr;
            ConfigurationManager::saveConfiguration(confPath, ruleManager, saveErr);
            std::cout << "{\"status\": \"success\", \"ruleId\": " << id << "}\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "{\"status\": \"error\", \"message\": \"" << e.what() << "\"}\n";
            return 1;
        }
    }

    if (cmd == "--json-edit-rule") {
        // Usage: --json-edit-rule <id> <priority> <action> <name> <proto> <dstPort> <srcCidr> <dir>
        if (argc < 10) {
            std::cout << "{\"status\": \"error\", \"message\": \"Usage: --json-edit-rule <id> <priority> <action> <name> <proto> <dstPort> <srcCidr> <dir>\"}\n";
            return 1;
        }
        try {
            int id = std::stoi(argv[2]);
            int priority = std::stoi(argv[3]);
            std::string actionStr = argv[4];
            std::string name = argv[5];
            std::string protoStr = argv[6];
            int port = std::stoi(argv[7]);
            std::string cidr = argv[8];
            std::string dirStr = argv[9];

            FirewallRule* existing = ruleManager.findRuleById(id);
            if (!existing) {
                std::cout << "{\"status\": \"error\", \"message\": \"Rule ID not found\"}\n";
                return 1;
            }
            bool wasEnabled = existing->isEnabled();

            ActionType act = ActionType::BLOCK;
            if (actionStr == "ALLOW") act = ActionType::ALLOW;
            else if (actionStr == "REJECT") act = ActionType::REJECT;
            else if (actionStr == "LOG") act = ActionType::LOG;

            FirewallRule rule(id, priority, wasEnabled, name, act);

            if (protoStr != "ANY") {
                ProtocolMatch pm = ProtocolMatch::ANY;
                if (protoStr == "TCP") pm = ProtocolMatch::TCP;
                else if (protoStr == "UDP") pm = ProtocolMatch::UDP;
                else if (protoStr == "ICMP") pm = ProtocolMatch::ICMP;
                rule.addCondition(std::make_unique<ProtocolCondition>(pm));
            }

            if (port > 0 && port <= 65535) {
                rule.addCondition(std::make_unique<PortCondition>(static_cast<uint16_t>(port), PortField::DESTINATION));
            }

            if (cidr != "ANY" && !cidr.empty()) {
                rule.addCondition(std::make_unique<IPCondition>(cidr, IPField::SOURCE));
            }

            if (dirStr != "ANY") {
                DirectionMatch dm = (dirStr == "OUTBOUND") ? DirectionMatch::OUTBOUND : DirectionMatch::INBOUND;
                rule.addCondition(std::make_unique<DirectionCondition>(dm));
            }

            if (!ruleManager.updateRule(std::move(rule))) {
                std::cout << "{\"status\": \"error\", \"message\": \"Failed to update rule (0 conditions or not found)\"}\n";
                return 1;
            }

            std::string saveErr;
            ConfigurationManager::saveConfiguration(confPath, ruleManager, saveErr);
            std::cout << "{\"status\": \"success\", \"ruleId\": " << id << "}\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "{\"status\": \"error\", \"message\": \"" << e.what() << "\"}\n";
            return 1;
        }
    }

    std::cout << "{\"status\": \"error\", \"message\": \"Unknown JSON command\"}\n";
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        return handleJsonCli(argc, argv);
    }

    RuleManager ruleManager;
    FirewallEngine engine(ruleManager);
    SimulationRunner runner(engine);

    // Load initial sample rules
    std::string err;
    if (ConfigurationManager::loadConfiguration("rules/sample_rules.conf", ruleManager, err)) {
        std::cout << "[Firewall Initialization] Successfully loaded 'rules/sample_rules.conf' ("
                  << ruleManager.size() << " policies active).\n";
    } else {
        std::cout << "[Firewall Initialization] No default configuration loaded: " << err << "\n";
    }

    int choice = 0;
    while (true) {
        displayMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 12.\n";
            continue;
        }

        if (choice == 12) {
            std::cout << "\nExiting Firewall Rule Engine. Security logs saved to 'firewall_audit.log'. Goodbye!\n";
            break;
        }

        switch (choice) {
            case 1:
                printRulesTable(ruleManager, engine.getDefaultPolicy());
                break;

            case 2: {
                std::cout << "\n--- Add New Security Rule ---\n";
                int id, priority;
                std::string name, actionStr;
                std::cout << " Rule ID (unique int): ";
                std::cin >> id;
                std::cout << " Priority (int, lower executes first): ";
                std::cin >> priority;
                std::cout << " Action (ALLOW, BLOCK, REJECT, LOG): ";
                std::cin >> actionStr;
                std::cin.ignore();
                std::cout << " Rule Name (description): ";
                std::getline(std::cin, name);

                ActionType act = ActionType::BLOCK;
                if (actionStr == "ALLOW") act = ActionType::ALLOW;
                else if (actionStr == "REJECT") act = ActionType::REJECT;
                else if (actionStr == "LOG") act = ActionType::LOG;

                FirewallRule newRule(id, priority, true, name, act);

                std::cout << " Add Protocol Condition? (1=Yes, 0=No): ";
                int addProto; std::cin >> addProto;
                if (addProto) {
                    std::cout << " Protocol (TCP, UDP, ICMP, ANY): ";
                    std::string pStr; std::cin >> pStr;
                    ProtocolMatch pm = ProtocolMatch::ANY;
                    if (pStr == "TCP") pm = ProtocolMatch::TCP;
                    else if (pStr == "UDP") pm = ProtocolMatch::UDP;
                    else if (pStr == "ICMP") pm = ProtocolMatch::ICMP;
                    newRule.addCondition(std::make_unique<ProtocolCondition>(pm));
                }

                std::cout << " Add Destination Port Condition? (1=Yes, 0=No): ";
                int addPort; std::cin >> addPort;
                if (addPort) {
                    std::cout << " Destination Port (e.g. 443): ";
                    uint16_t port; std::cin >> port;
                    newRule.addCondition(std::make_unique<PortCondition>(port, PortField::DESTINATION));
                }

                std::cout << " Add Source IP / Subnet Condition? (1=Yes, 0=No): ";
                int addIP; std::cin >> addIP;
                if (addIP) {
                    std::cout << " Source IP/CIDR (e.g. 192.168.1.0/24): ";
                    std::string ipStr; std::cin >> ipStr;
                    try {
                        newRule.addCondition(std::make_unique<IPCondition>(ipStr, IPField::SOURCE));
                    } catch (const std::exception& e) {
                        std::cerr << "Invalid IP condition: " << e.what() << "\n";
                    }
                }

                if (ruleManager.addRule(std::move(newRule))) {
                    std::cout << "SUCCESS: Rule #" << id << " added and rules re-sorted by priority!\n";
                }
                break;
            }

            case 3: {
                std::cout << "\n Enter Rule ID to modify priority: ";
                int rId, newPri;
                std::cin >> rId;
                std::cout << " Enter New Priority (lower executes first): ";
                std::cin >> newPri;
                if (ruleManager.modifyRulePriority(rId, newPri)) {
                    std::cout << "SUCCESS: Rule #" << rId << " priority updated to " << newPri << " and re-sorted.\n";
                } else {
                    std::cout << "ERROR: Rule ID #" << rId << " not found.\n";
                }
                break;
            }

            case 4: {
                std::cout << "\n Enter Rule ID to toggle enabled/disabled: ";
                int rId; std::cin >> rId;
                FirewallRule* r = ruleManager.findRuleById(rId);
                if (r) {
                    bool newState = !r->isEnabled();
                    ruleManager.setRuleEnabled(rId, newState);
                    std::cout << "SUCCESS: Rule #" << rId << " is now " << (newState ? "ENABLED" : "DISABLED") << ".\n";
                } else {
                    std::cout << "ERROR: Rule ID #" << rId << " not found.\n";
                }
                break;
            }

            case 5: {
                std::cout << "\n Enter Rule ID to delete: ";
                int rId; std::cin >> rId;
                if (ruleManager.removeRule(rId)) {
                    std::cout << "SUCCESS: Rule #" << rId << " deleted.\n";
                } else {
                    std::cout << "ERROR: Rule ID #" << rId << " not found.\n";
                }
                break;
            }

            case 6:
                handleManualPacket(engine);
                break;

            case 7: {
                std::cout << "\nSelect Demonstration Scenario:\n";
                std::cout << " 1. Scenario A: Basic Web Filtering (HTTPS Allowed, Default Deny)\n";
                std::cout << " 2. Scenario B: Port Filtering (DNS Allowed, Telnet Rejected)\n";
                std::cout << " 3. Scenario C: Rule Priority & Overlapping Policies\n";
                std::cout << " 4. Scenario D: Subnet Filtering (CIDR /16 and /24)\n";
                std::cout << " 5. Scenario E: Non-Terminal Audit Logging Flow\n";
                std::cout << " 6. Scenario F: Configuration Persistence & Rollback\n";
                std::cout << " 7. Scenario G: High-Volume Batch Benchmark (50,000 packets)\n";
                std::cout << " 8. Run All Scenarios Consecutively\n";
                std::cout << " Choice (1-8): ";
                int sc; std::cin >> sc;
                if (sc == 1) runner.runScenarioA_BasicFiltering();
                else if (sc == 2) runner.runScenarioB_PortFiltering();
                else if (sc == 3) runner.runScenarioC_RulePriority();
                else if (sc == 4) runner.runScenarioD_SubnetFiltering();
                else if (sc == 5) runner.runScenarioE_AuditLogging();
                else if (sc == 6) runner.runScenarioF_ConfigurationPersistence();
                else if (sc == 7) runner.runScenarioG_BatchBenchmark(50000);
                else if (sc == 8) {
                    runner.runScenarioA_BasicFiltering();
                    runner.runScenarioB_PortFiltering();
                    runner.runScenarioC_RulePriority();
                    runner.runScenarioD_SubnetFiltering();
                    runner.runScenarioE_AuditLogging();
                    runner.runScenarioF_ConfigurationPersistence();
                    runner.runScenarioG_BatchBenchmark(50000);
                }
                break;
            }

            case 8:
                engine.getStatistics().printSummary();
                break;

            case 9: {
                std::cout << "\n Enter configuration file path (default: 'rules/sample_rules.conf'): ";
                std::string path;
                std::cin >> path;
                if (path.empty() || path == "default") path = "rules/sample_rules.conf";
                std::string loadErr;
                if (ConfigurationManager::loadConfiguration(path, ruleManager, loadErr)) {
                    std::cout << "SUCCESS: Configuration loaded successfully! (" << ruleManager.size() << " rules active).\n";
                } else {
                    std::cout << "FAILED: Transactional load aborted. " << loadErr << "\n";
                    std::cout << "Existing rules remain active and untouched.\n";
                }
                break;
            }

            case 10: {
                std::cout << "\n Enter destination configuration file path (e.g. 'rules/exported_rules.conf'): ";
                std::string path;
                std::cin >> path;
                std::string saveErr;
                if (ConfigurationManager::saveConfiguration(path, ruleManager, saveErr)) {
                    std::cout << "SUCCESS: Configuration saved to '" << path << "' (" << ruleManager.size() << " rules exported).\n";
                } else {
                    std::cout << "FAILED: " << saveErr << "\n";
                }
                break;
            }

            case 11:
                std::cout << "\n[Running Unit Test Suite in-process]\n";
                // Trigger test runner
                runner.runScenarioC_RulePriority();
                runner.runScenarioF_ConfigurationPersistence();
                break;

            default:
                std::cout << "Invalid choice. Please select from 1 to 12.\n";
                break;
        }
    }

    return 0;
}
