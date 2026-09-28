#ifndef METRICS_HPP
#define METRICS_HPP

#include <vector>
#include <string>
#include <cstdint>

struct MetricSample {
    double timestamp{0.0};
    int flowId{0};
    double cwnd{0.0};
    double ssthresh{0.0};
    double throughputMbps{0.0};
    double avgRttMs{0.0};
    double jitterMs{0.0};
    uint64_t totalPacketsSent{0};
    uint64_t totalPacketsDelivered{0};
    uint64_t totalPacketsDropped{0};
    double bufferOccupancyPercent{0.0};
    double linkUtilizationPercent{0.0};
};

struct SummaryStats {
    double durationSec{0.0};
    uint64_t totalPacketsSent{0};
    uint64_t totalPacketsDelivered{0};
    uint64_t totalPacketsDropped{0};
    double packetLossRatePercent{0.0};
    double avgThroughputMbps{0.0};
    double peakThroughputMbps{0.0};
    double avgRttMs{0.0};
    double minRttMs{0.0};
    double maxRttMs{0.0};
    double avgJitterMs{0.0};
    double peakCwnd{0.0};
    double avgBufferOccupancyPercent{0.0};
};

class MetricsCollector {
private:
    std::vector<MetricSample> samples;
    std::vector<double> rttHistoryMs;
    double lastRttMs{0.0};

public:
    MetricsCollector() = default;

    void addSample(const MetricSample& sample);
    void recordRTT(double rttSec);

    const std::vector<MetricSample>& getSamples() const { return samples; }

    SummaryStats computeSummaryStats(double totalDurationSec) const;

    bool exportToCSV(const std::string& filename) const;
    bool exportCwndTraceToCSV(const std::string& filename) const;

    void clear() {
        samples.clear();
        rttHistoryMs.clear();
        lastRttMs = 0.0;
    }
};

#endif // METRICS_HPP
