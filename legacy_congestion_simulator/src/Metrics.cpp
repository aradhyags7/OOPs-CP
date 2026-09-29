#include "Metrics.hpp"
#include <fstream>
#include <iomanip>
#include <cmath>

void MetricsCollector::addSample(const MetricSample& sample) {
    samples.push_back(sample);
}

void MetricsCollector::recordRTT(double rttSec) {
    double rttMs = rttSec * 1000.0;
    rttHistoryMs.push_back(rttMs);
    lastRttMs = rttMs;
}

SummaryStats MetricsCollector::computeSummaryStats(double totalDurationSec) const {
    SummaryStats stats;
    stats.durationSec = totalDurationSec;

    if (samples.empty()) return stats;

    double sumThroughput = 0.0;
    double maxThroughput = 0.0;
    double maxCwnd = 0.0;
    double sumBufferOccupancy = 0.0;

    for (const auto& s : samples) {
        sumThroughput += s.throughputMbps;
        if (s.throughputMbps > maxThroughput) maxThroughput = s.throughputMbps;
        if (s.cwnd > maxCwnd) maxCwnd = s.cwnd;
        sumBufferOccupancy += s.bufferOccupancyPercent;

        stats.totalPacketsSent = s.totalPacketsSent;
        stats.totalPacketsDelivered = s.totalPacketsDelivered;
        stats.totalPacketsDropped = s.totalPacketsDropped;
    }

    size_t nSamples = samples.size();
    stats.avgThroughputMbps = sumThroughput / static_cast<double>(nSamples);
    stats.peakThroughputMbps = maxThroughput;
    stats.peakCwnd = maxCwnd;
    stats.avgBufferOccupancyPercent = sumBufferOccupancy / static_cast<double>(nSamples);

    if (stats.totalPacketsSent > 0) {
        stats.packetLossRatePercent = (static_cast<double>(stats.totalPacketsDropped) /
                                       static_cast<double>(stats.totalPacketsSent)) * 100.0;
    }

    if (!rttHistoryMs.empty()) {
        double sumRtt = 0.0;
        double minRtt = rttHistoryMs.front();
        double maxRtt = rttHistoryMs.front();
        double sumJitter = 0.0;

        for (size_t i = 0; i < rttHistoryMs.size(); ++i) {
            double rtt = rttHistoryMs[i];
            sumRtt += rtt;
            if (rtt < minRtt) minRtt = rtt;
            if (rtt > maxRtt) maxRtt = rtt;

            if (i > 0) {
                sumJitter += std::abs(rttHistoryMs[i] - rttHistoryMs[i - 1]);
            }
        }

        stats.avgRttMs = sumRtt / static_cast<double>(rttHistoryMs.size());
        stats.minRttMs = minRtt;
        stats.maxRttMs = maxRtt;
        stats.avgJitterMs = (rttHistoryMs.size() > 1) ? (sumJitter / static_cast<double>(rttHistoryMs.size() - 1)) : 0.0;
    }

    return stats;
}

bool MetricsCollector::exportToCSV(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return false;

    outFile << "Timestamp(s),FlowID,CWND(MSS),Ssthresh(MSS),Throughput(Mbps),AvgRTT(ms),Jitter(ms),"
            << "PacketsSent,PacketsDelivered,PacketsDropped,BufferOccupancy(%),LinkUtil(%)\n";

    for (const auto& s : samples) {
        outFile << std::fixed << std::setprecision(4)
                << s.timestamp << ","
                << s.flowId << ","
                << s.cwnd << ","
                << s.ssthresh << ","
                << s.throughputMbps << ","
                << s.avgRttMs << ","
                << s.jitterMs << ","
                << s.totalPacketsSent << ","
                << s.totalPacketsDelivered << ","
                << s.totalPacketsDropped << ","
                << s.bufferOccupancyPercent << ","
                << s.linkUtilizationPercent << "\n";
    }

    outFile.close();
    return true;
}

bool MetricsCollector::exportCwndTraceToCSV(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return false;

    outFile << "Timestamp(s),CWND(MSS),Ssthresh(MSS)\n";
    for (const auto& s : samples) {
        outFile << std::fixed << std::setprecision(4)
                << s.timestamp << ","
                << s.cwnd << ","
                << s.ssthresh << "\n";
    }

    outFile.close();
    return true;
}
