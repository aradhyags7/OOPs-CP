#include "Statistics.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

using namespace std;

void Statistics::recordDecision(Decision dec, int ruleId, uint32_t bytes) {
    packetsProcessed++;
    totalBytesProcessed += bytes;

    switch (dec) {
        case Decision::ALLOWED:
            packetsAllowed++;
            totalBytesAllowed += bytes;
            break;
        case Decision::BLOCKED:
            packetsBlocked++;
            totalBytesBlocked += bytes;
            break;
        case Decision::REJECTED:
            packetsRejected++;
            totalBytesBlocked += bytes;
            break;
    }

    if (ruleId != -1) {
        ruleHitCounts[ruleId]++;
    } else {
        defaultPolicyHits++;
    }
}

void Statistics::recordLogEvent(int ruleId) {
    loggingEventsRecorded++;
    ruleHitCounts[ruleId]++;
}

void Statistics::reset() {
    packetsProcessed = 0;
    packetsAllowed = 0;
    packetsBlocked = 0;
    packetsRejected = 0;
    loggingEventsRecorded = 0;
    defaultPolicyHits = 0;
    totalBytesProcessed = 0;
    totalBytesAllowed = 0;
    totalBytesBlocked = 0;
    ruleHitCounts.clear();
}

void Statistics::printSummary() const {
    std::cout << "\n========================================================================\n";
    std::cout << "                 FIREWALL TELEMETRY & SECURITY METRICS                  \n";
    std::cout << "========================================================================\n";
    std::cout << " Total Packets Processed: " << std::setw(10) << packetsProcessed << " pkts\n";
    std::cout << " Packets Allowed:         " << std::setw(10) << packetsAllowed;
    if (packetsProcessed > 0) {
        double pct = (static_cast<double>(packetsAllowed) / static_cast<double>(packetsProcessed)) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(2) << pct << " %)";
    }
    std::cout << "\n";

    std::cout << " Packets Blocked (Drop):  " << std::setw(10) << packetsBlocked;
    if (packetsProcessed > 0) {
        double pct = (static_cast<double>(packetsBlocked) / static_cast<double>(packetsProcessed)) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(2) << pct << " %)";
    }
    std::cout << "\n";

    std::cout << " Packets Rejected:        " << std::setw(10) << packetsRejected;
    if (packetsProcessed > 0) {
        double pct = (static_cast<double>(packetsRejected) / static_cast<double>(packetsProcessed)) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(2) << pct << " %)";
    }
    std::cout << "\n";

    std::cout << " Default Policy Fallbacks:" << std::setw(10) << defaultPolicyHits << " pkts\n";
    std::cout << " Non-Terminal Log Events: " << std::setw(10) << loggingEventsRecorded << " events\n";
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << " Total Bytes Inspected:   " << std::setw(10) << totalBytesProcessed << " bytes\n";
    std::cout << " Total Bytes Allowed:     " << std::setw(10) << totalBytesAllowed << " bytes\n";
    std::cout << " Total Bytes Blocked:     " << std::setw(10) << totalBytesBlocked << " bytes\n";
    std::cout << "========================================================================\n\n";
}

void Statistics::accumulate(const Statistics& other) {
    packetsProcessed += other.packetsProcessed;
    packetsAllowed += other.packetsAllowed;
    packetsBlocked += other.packetsBlocked;
    packetsRejected += other.packetsRejected;
    loggingEventsRecorded += other.loggingEventsRecorded;
    defaultPolicyHits += other.defaultPolicyHits;
    totalBytesProcessed += other.totalBytesProcessed;
    totalBytesAllowed += other.totalBytesAllowed;
    totalBytesBlocked += other.totalBytesBlocked;
    for (const auto& kv : other.ruleHitCounts) {
        ruleHitCounts[kv.first] += kv.second;
    }
}

bool Statistics::saveToFile(const std::string& filePath) const {
    try {
        std::filesystem::path p(filePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        file << "PACKETS_PROCESSED " << packetsProcessed << "\n";
        file << "PACKETS_ALLOWED " << packetsAllowed << "\n";
        file << "PACKETS_BLOCKED " << packetsBlocked << "\n";
        file << "PACKETS_REJECTED " << packetsRejected << "\n";
        file << "LOGGING_EVENTS " << loggingEventsRecorded << "\n";
        file << "DEFAULT_HITS " << defaultPolicyHits << "\n";
        file << "TOTAL_BYTES_PROCESSED " << totalBytesProcessed << "\n";
        file << "TOTAL_BYTES_ALLOWED " << totalBytesAllowed << "\n";
        file << "TOTAL_BYTES_BLOCKED " << totalBytesBlocked << "\n";
        file << "RULE_HITS\n";
        for (const auto& kv : ruleHitCounts) {
            file << kv.first << " " << kv.second << "\n";
        }
        file << "END_RULE_HITS\n";
        return true;
    } catch (...) {
        return false;
    }
}

bool Statistics::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    reset();
    std::string key;
    bool inRuleHits = false;

    while (file >> key) {
        if (key == "RULE_HITS") {
            inRuleHits = true;
            continue;
        }
        if (key == "END_RULE_HITS") {
            inRuleHits = false;
            continue;
        }

        if (inRuleHits) {
            try {
                int rId = std::stoi(key);
                uint64_t count = 0;
                if (file >> count) {
                    ruleHitCounts[rId] = count;
                }
            } catch (...) {}
            continue;
        }

        if (key == "PACKETS_PROCESSED") file >> packetsProcessed;
        else if (key == "PACKETS_ALLOWED") file >> packetsAllowed;
        else if (key == "PACKETS_BLOCKED") file >> packetsBlocked;
        else if (key == "PACKETS_REJECTED") file >> packetsRejected;
        else if (key == "LOGGING_EVENTS") file >> loggingEventsRecorded;
        else if (key == "DEFAULT_HITS") file >> defaultPolicyHits;
        else if (key == "TOTAL_BYTES_PROCESSED") file >> totalBytesProcessed;
        else if (key == "TOTAL_BYTES_ALLOWED") file >> totalBytesAllowed;
        else if (key == "TOTAL_BYTES_BLOCKED") file >> totalBytesBlocked;
    }
    return true;
}
