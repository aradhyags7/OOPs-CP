#ifndef AUDIT_LOGGER_HPP
#define AUDIT_LOGGER_HPP

#include "Packet.hpp"
#include "Statistics.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;

class AuditLogger {
private:
    std::ofstream logFile;
    std::ofstream csvFile;
    bool enableConsole{true};
    bool enableFile{true};
    bool enableCsv{true};
    uint64_t loggingErrorCount{0};

public:
    AuditLogger() = default;
    ~AuditLogger();

    bool initialize(const std::string& logFilename = "firewall_audit.log",
                    const std::string& csvFilename = "firewall_decisions.csv");

    void logTerminalDecision(const Packet& pkt, Decision dec, int ruleId,
                             const std::string& ruleName, const std::string& reason);

    void logNonTerminalTap(const Packet& pkt, int ruleId, const std::string& ruleName);

    void setConsoleOutput(bool enable) noexcept { enableConsole = enable; }
    void setFileOutput(bool enable) noexcept { enableFile = enable; }
    void setCsvOutput(bool enable) noexcept { enableCsv = enable; }

    uint64_t getErrorCount() const noexcept { return loggingErrorCount; }
    void close();

    static std::string getTimestampString();
    static std::string decisionToString(Decision dec);
};

#endif // AUDIT_LOGGER_HPP
