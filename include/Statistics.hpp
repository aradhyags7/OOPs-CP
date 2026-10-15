#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

enum class Decision {
    ALLOWED,
    BLOCKED,
    REJECTED
};

class Statistics {
private:
    uint64_t packetsProcessed{0};
    uint64_t packetsAllowed{0};
    uint64_t packetsBlocked{0};
    uint64_t packetsRejected{0};
    uint64_t loggingEventsRecorded{0};
    uint64_t defaultPolicyHits{0};
    uint64_t totalBytesProcessed{0};
    uint64_t totalBytesAllowed{0};
    uint64_t totalBytesBlocked{0};
    std::unordered_map<int, uint64_t> ruleHitCounts;

public:
    Statistics() = default;

    void recordDecision(Decision dec, int ruleId, uint32_t bytes);
    void recordLogEvent(int ruleId);
    void reset();

    uint64_t getPacketsProcessed() const noexcept { return packetsProcessed; }
    uint64_t getPacketsAllowed() const noexcept { return packetsAllowed; }
    uint64_t getPacketsBlocked() const noexcept { return packetsBlocked; }
    uint64_t getPacketsRejected() const noexcept { return packetsRejected; }
    uint64_t getLoggingEventsRecorded() const noexcept { return loggingEventsRecorded; }
    uint64_t getDefaultPolicyHits() const noexcept { return defaultPolicyHits; }
    uint64_t getTotalBytesProcessed() const noexcept { return totalBytesProcessed; }
    uint64_t getTotalBytesAllowed() const noexcept { return totalBytesAllowed; }
    uint64_t getTotalBytesBlocked() const noexcept { return totalBytesBlocked; }

    const std::unordered_map<int, uint64_t>& getRuleHitCounts() const noexcept {
        return ruleHitCounts;
    }

    bool saveToFile(const std::string& filePath) const;
    bool loadFromFile(const std::string& filePath);
    void accumulate(const Statistics& other);

    void printSummary() const;
};

#endif // STATISTICS_HPP
