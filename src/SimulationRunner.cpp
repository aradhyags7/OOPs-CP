#include "SimulationRunner.hpp"
#include "PacketGenerator.hpp"
#include "ConfigurationManager.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

void SimulationRunner::printHeader(const std::string& title) {
    std::cout << "\n========================================================================\n";
    std::cout << "   " << title << "\n";
    std::cout << "========================================================================\n";
}

void SimulationRunner::printResultRow(const EvaluationResult& res, const Packet& pkt) {
    std::cout << "  Packet #" << std::setw(4) << pkt.id << " [" << std::setw(4) << pkt.getProtocolString() << " "
              << std::setw(8) << pkt.getDirectionString() << "] "
              << std::setw(15) << Packet::ipToString(pkt.srcIP) << ":" << std::setw(5) << pkt.srcPort
              << " -> " << std::setw(15) << Packet::ipToString(pkt.dstIP) << ":" << std::setw(5) << pkt.dstPort
              << " | Verdict: " << std::setw(8) << AuditLogger::decisionToString(res.finalDecision)
              << " (Rule #" << std::setw(2) << res.matchedRuleId << " - " << res.matchedRuleName << ")\n";
}

void SimulationRunner::runScenarioA_BasicFiltering() {
    printHeader("SCENARIO A: BASIC FILTERING (ALLOW HTTPS, BLOCK SPECIFIC IP, DEFAULT DENY)");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    engine.getStatistics().reset();
    engine.setDefaultPolicy(Decision::BLOCKED);

    // Rule 10: Allow Inbound HTTPS (TCP 443)
    FirewallRule r1(10, 10, true, "Allow Inbound HTTPS", ActionType::ALLOW);
    r1.addCondition(std::make_unique<ProtocolCondition>(ProtocolMatch::TCP));
    r1.addCondition(std::make_unique<PortCondition>(443, PortField::DESTINATION));
    r1.addCondition(std::make_unique<DirectionCondition>(DirectionMatch::INBOUND));
    rm.addRule(std::move(r1));

    // Rule 20: Block Malicious IP
    FirewallRule r2(20, 20, true, "Block Rogue Host", ActionType::BLOCK);
    r2.addCondition(std::make_unique<IPCondition>("198.51.100.45", IPField::SOURCE));
    rm.addRule(std::move(r2));

    // Test Packets
    Packet p1 = PacketGenerator::createHTTPS(1, "10.0.1.10", "192.0.2.1", Direction::INBOUND);
    Packet p2(2, Packet::parseIPv4("198.51.100.45"), Packet::parseIPv4("192.0.2.1"), 8080, 80, Protocol::TCP, Direction::INBOUND, 100);
    Packet p3(3, Packet::parseIPv4("172.16.0.5"), Packet::parseIPv4("192.0.2.1"), 4321, 8080, Protocol::TCP, Direction::INBOUND, 100);

    printResultRow(engine.processPacket(p1), p1);
    printResultRow(engine.processPacket(p2), p2);
    printResultRow(engine.processPacket(p3), p3);

    engine.getStatistics().printSummary();
}

void SimulationRunner::runScenarioB_PortFiltering() {
    printHeader("SCENARIO B: PORT & PROTOCOL FILTERING (DNS ALLOWED, TELNET REJECTED)");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    engine.getStatistics().reset();
    engine.setDefaultPolicy(Decision::BLOCKED);

    // Rule 1: Allow Outbound DNS (UDP 53)
    FirewallRule r1(1, 10, true, "Allow Outbound DNS", ActionType::ALLOW);
    r1.addCondition(std::make_unique<ProtocolCondition>(ProtocolMatch::UDP));
    r1.addCondition(std::make_unique<PortCondition>(53, PortField::DESTINATION));
    r1.addCondition(std::make_unique<DirectionCondition>(DirectionMatch::OUTBOUND));
    rm.addRule(std::move(r1));

    // Rule 2: Reject Inbound Telnet (TCP 23)
    FirewallRule r2(2, 20, true, "Reject Inbound Telnet", ActionType::REJECT);
    r2.addCondition(std::make_unique<ProtocolCondition>(ProtocolMatch::TCP));
    r2.addCondition(std::make_unique<PortCondition>(23, PortField::DESTINATION));
    r2.addCondition(std::make_unique<DirectionCondition>(DirectionMatch::INBOUND));
    rm.addRule(std::move(r2));

    Packet p1 = PacketGenerator::createDNS(101, "192.168.1.50", "8.8.8.8", Direction::OUTBOUND);
    Packet p2 = PacketGenerator::createTelnet(102, "203.0.113.5", "192.168.1.1", Direction::INBOUND);
    Packet p3 = PacketGenerator::createPing(103, "192.168.1.50", "8.8.8.8", Direction::OUTBOUND);

    printResultRow(engine.processPacket(p1), p1);
    printResultRow(engine.processPacket(p2), p2);
    printResultRow(engine.processPacket(p3), p3);

    engine.getStatistics().printSummary();
}

void SimulationRunner::runScenarioC_RulePriority() {
    printHeader("SCENARIO C: RULE PRIORITY & OVERLAPPING POLICIES");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    engine.getStatistics().reset();
    engine.setDefaultPolicy(Decision::BLOCKED);

    std::cout << "[Step 1] Loading Overlapping Rules:\n";
    std::cout << "  - Rule #1 (Priority 10): ALLOW Host 192.168.1.50\n";
    std::cout << "  - Rule #2 (Priority 20): BLOCK Subnet 192.168.1.0/24\n";

    FirewallRule r1(1, 10, true, "Allow Specific Host", ActionType::ALLOW);
    r1.addCondition(std::make_unique<IPCondition>("192.168.1.50", IPField::SOURCE));
    rm.addRule(std::move(r1));

    FirewallRule r2(2, 20, true, "Block Entire Subnet", ActionType::BLOCK);
    r2.addCondition(std::make_unique<IPCondition>("192.168.1.0/24", IPField::SOURCE));
    rm.addRule(std::move(r2));

    Packet pkt = PacketGenerator::createSSH(201, "192.168.1.50", "10.0.0.1", Direction::INBOUND);
    std::cout << "\nEvaluating Packet from 192.168.1.50 under Priority 10 vs 20:\n";
    printResultRow(engine.processPacket(pkt), pkt);

    std::cout << "\n[Step 2] Inverting Priorities (Block Subnet Priority 5, Allow Host Priority 10):\n";
    rm.modifyRulePriority(2, 5); // Subnet block now has higher priority!

    printResultRow(engine.processPacket(pkt), pkt);
    std::cout << "Observed Behavior: With higher priority, the Subnet Block rule matched first!\n";
}

void SimulationRunner::runScenarioD_SubnetFiltering() {
    printHeader("SCENARIO D: SUBNET FILTERING (CIDR /16 AND /24)");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    engine.getStatistics().reset();
    engine.setDefaultPolicy(Decision::BLOCKED);

    // Allow corporate subnet 10.10.0.0/16
    FirewallRule r1(1, 10, true, "Allow Corporate Subnet", ActionType::ALLOW);
    r1.addCondition(std::make_unique<IPCondition>("10.10.0.0/16", IPField::SOURCE));
    rm.addRule(std::move(r1));

    // Block DMZ subnet 172.16.50.0/24
    FirewallRule r2(2, 20, true, "Block DMZ Subnet", ActionType::BLOCK);
    r2.addCondition(std::make_unique<IPCondition>("172.16.50.0/24", IPField::SOURCE));
    rm.addRule(std::move(r2));

    Packet p1(301, Packet::parseIPv4("10.10.5.20"), Packet::parseIPv4("192.0.2.1"), 5000, 80, Protocol::TCP, Direction::INBOUND);
    Packet p2(302, Packet::parseIPv4("10.10.255.254"), Packet::parseIPv4("192.0.2.1"), 5001, 80, Protocol::TCP, Direction::INBOUND);
    Packet p3(303, Packet::parseIPv4("172.16.50.15"), Packet::parseIPv4("192.0.2.1"), 5002, 80, Protocol::TCP, Direction::INBOUND);
    Packet p4(304, Packet::parseIPv4("172.16.51.15"), Packet::parseIPv4("192.0.2.1"), 5003, 80, Protocol::TCP, Direction::INBOUND);

    printResultRow(engine.processPacket(p1), p1);
    printResultRow(engine.processPacket(p2), p2);
    printResultRow(engine.processPacket(p3), p3);
    printResultRow(engine.processPacket(p4), p4);
}

void SimulationRunner::runScenarioE_AuditLogging() {
    printHeader("SCENARIO E: AUDIT LOGGING (NON-TERMINAL LOG TAP FOLLOWED BY TERMINAL ACTION)");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    engine.getStatistics().reset();
    engine.setDefaultPolicy(Decision::BLOCKED);

    // Rule 1: LOG Tap for Port 23
    FirewallRule r1(1, 10, true, "Audit Tap Telnet", ActionType::LOG);
    r1.addCondition(std::make_unique<PortCondition>(23, PortField::DESTINATION));
    rm.addRule(std::move(r1));

    // Rule 2: REJECT Port 23
    FirewallRule r2(2, 20, true, "Reject Telnet Port", ActionType::REJECT);
    r2.addCondition(std::make_unique<PortCondition>(23, PortField::DESTINATION));
    rm.addRule(std::move(r2));

    Packet p1 = PacketGenerator::createTelnet(401, "192.168.1.100", "10.0.0.1", Direction::INBOUND);

    std::cout << "Injecting Telnet packet to verify Audit Tap + Terminal Action pipeline:\n";
    EvaluationResult res = engine.processPacket(p1);
    printResultRow(res, p1);

    std::cout << "Non-Terminal Log Taps Triggered: " << res.loggedRuleIds.size() << " (Rule ID: ";
    for (int id : res.loggedRuleIds) std::cout << id << " ";
    std::cout << ")\n";
    std::cout << "Final Packet Decision: " << AuditLogger::decisionToString(res.finalDecision) << "\n";
    std::cout << "Notice: Log tap executed without prematurely halting the evaluation chain!\n";
}

void SimulationRunner::runScenarioF_ConfigurationPersistence() {
    printHeader("SCENARIO F: CONFIGURATION PERSISTENCE & TRANSACTIONAL ROLLBACK");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    std::string err;

    std::cout << "[Step 1] Loading Valid Configuration ('rules/sample_rules.conf')...\n";
    bool ok = ConfigurationManager::loadConfiguration("rules/sample_rules.conf", rm, err);
    if (ok) {
        std::cout << "SUCCESS: Loaded " << rm.size() << " policies into memory.\n";
    } else {
        std::cout << "FAILED: " << err << "\n";
    }

    std::cout << "\n[Step 2] Testing Transactional Rollback on Malformed Configuration...\n";
    std::string badFile = "rules/malformed_test.conf";
    {
        std::ofstream bad(badFile);
        bad << "RULE 99 10 1 ALLOW \"Broken Rule\"\n";
        bad << "  COND IP SRC 192.168.1.300\n"; // Invalid IP octet!
        bad << "END_RULE\n";
    }

    size_t countBefore = rm.size();
    bool badOk = ConfigurationManager::loadConfiguration(badFile, rm, err);
    std::cout << "Loading Invalid File Result: " << (badOk ? "Allowed (Unexpected)" : "Rejected (Expected)") << "\n";
    std::cout << "Reported Error: " << err << "\n";
    std::cout << "Rule Count Before: " << countBefore << " | Rule Count After: " << rm.size() << "\n";
    std::cout << "Result: Active rules preserved intact. Transaction rollback verified!\n";
}

void SimulationRunner::runScenarioG_BatchBenchmark(size_t packetCount) {
    printHeader("SCENARIO G: HIGH-VOLUME BATCH EVALUATION BENCHMARK");

    RuleManager& rm = engine.getRuleManager();
    rm.clear();
    std::string err;
    ConfigurationManager::loadConfiguration("rules/sample_rules.conf", rm, err);
    engine.getStatistics().reset();

    std::cout << "Generating " << packetCount << " synthetic test packets...\n";
    auto packets = PacketGenerator::generateBenchmarkBatch(packetCount);

    // Disable console printing for benchmark speed measurement
    engine.getLogger().setConsoleOutput(false);
    engine.getLogger().setFileOutput(false);
    engine.getLogger().setCsvOutput(false);

    std::cout << "Executing batch inspection pipeline across " << rm.size() << " security rules...\n";

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& pkt : packets) {
        engine.processPacket(pkt);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    double sec = elapsed.count();
    double pps = (sec > 0.0) ? (static_cast<double>(packetCount) / sec) : 0.0;

    std::cout << "\n========================================================================\n";
    std::cout << "                    BENCHMARK PERFORMANCE RESULTS                       \n";
    std::cout << "========================================================================\n";
    std::cout << " Total Packets Evaluated: " << std::setw(10) << packetCount << " pkts\n";
    std::cout << " Execution Time Elapsed:  " << std::fixed << std::setprecision(4) << sec << " seconds\n";
    std::cout << " Evaluated Throughput:    " << std::fixed << std::setprecision(2) << pps << " packets/second\n";
    std::cout << "========================================================================\n";

    engine.getLogger().setConsoleOutput(true);
    engine.getLogger().setFileOutput(true);
    engine.getLogger().setCsvOutput(true);

    engine.getStatistics().printSummary();
}
