#include "AuditLogger.hpp"
#include <ctime>

AuditLogger::~AuditLogger() {
    close();
}

std::string AuditLogger::getTimestampString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
    return std::string(buf);
}

std::string AuditLogger::decisionToString(Decision dec) {
    switch (dec) {
        case Decision::ALLOWED:  return "ALLOWED";
        case Decision::BLOCKED:  return "BLOCKED";
        case Decision::REJECTED: return "REJECTED";
        default:                 return "UNKNOWN";
    }
}

bool AuditLogger::initialize(const std::string& logFilename, const std::string& csvFilename) {
    if (enableFile) {
        logFile.open(logFilename, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "[AuditLogger Warning] Could not open log file: " << logFilename << "\n";
            loggingErrorCount++;
        }
    }

    if (enableCsv) {
        bool writeHeader = false;
        std::ifstream test(csvFilename);
        if (!test.is_open()) {
            writeHeader = true;
        }
        test.close();

        csvFile.open(csvFilename, std::ios::out | std::ios::app);
        if (!csvFile.is_open()) {
            std::cerr << "[AuditLogger Warning] Could not open CSV file: " << csvFilename << "\n";
            loggingErrorCount++;
        } else if (writeHeader) {
            csvFile << "Timestamp,PacketID,Protocol,Direction,SrcIP,SrcPort,DstIP,DstPort,PayloadBytes,Decision,RuleID,Reason\n";
        }
    }

    return (loggingErrorCount == 0);
}

void AuditLogger::logTerminalDecision(const Packet& pkt, Decision dec, int ruleId,
                                     const std::string& ruleName, const std::string& reason) {
    std::string ts = getTimestampString();
    std::string decStr = decisionToString(dec);

    if (enableConsole) {
        std::cout << "[" << ts << "] [" << std::setw(8) << decStr << "] "
                  << pkt.toString() << " | Rule: #" << ruleId << " (" << ruleName << ") | Reason: " << reason << "\n";
    }

    if (enableFile && logFile.is_open()) {
        logFile << "[" << ts << "] [VERDICT: " << decStr << "] "
                << "PktID=" << pkt.id << " Proto=" << pkt.getProtocolString()
                << " Dir=" << pkt.getDirectionString()
                << " Src=" << Packet::ipToString(pkt.srcIP) << ":" << pkt.srcPort
                << " Dst=" << Packet::ipToString(pkt.dstIP) << ":" << pkt.dstPort
                << " Bytes=" << pkt.payloadSize
                << " RuleID=" << ruleId << " RuleName=\"" << ruleName << "\""
                << " Reason=\"" << reason << "\"\n";
        if (!logFile.good()) {
            loggingErrorCount++;
        }
    }

    if (enableCsv && csvFile.is_open()) {
        csvFile << ts << ","
                << pkt.id << ","
                << pkt.getProtocolString() << ","
                << pkt.getDirectionString() << ","
                << Packet::ipToString(pkt.srcIP) << ","
                << pkt.srcPort << ","
                << Packet::ipToString(pkt.dstIP) << ","
                << pkt.dstPort << ","
                << pkt.payloadSize << ","
                << decStr << ","
                << ruleId << ",\""
                << reason << "\"\n";
        if (!csvFile.good()) {
            loggingErrorCount++;
        }
    }
}

void AuditLogger::logNonTerminalTap(const Packet& pkt, int ruleId, const std::string& ruleName) {
    std::string ts = getTimestampString();

    if (enableConsole) {
        std::cout << "[" << ts << "] [AUDIT-TAP ] "
                  << pkt.toString() << " | Triggered by Rule #" << ruleId << " (" << ruleName << ")\n";
    }

    if (enableFile && logFile.is_open()) {
        logFile << "[" << ts << "] [AUDIT-TAP: LOG] "
                << "PktID=" << pkt.id << " Proto=" << pkt.getProtocolString()
                << " Src=" << Packet::ipToString(pkt.srcIP) << ":" << pkt.srcPort
                << " Dst=" << Packet::ipToString(pkt.dstIP) << ":" << pkt.dstPort
                << " TriggeredByRuleID=" << ruleId << " RuleName=\"" << ruleName << "\"\n";
        if (!logFile.good()) {
            loggingErrorCount++;
        }
    }
}

void AuditLogger::close() {
    if (logFile.is_open()) {
        logFile.flush();
        logFile.close();
    }
    if (csvFile.is_open()) {
        csvFile.flush();
        csvFile.close();
    }
}
