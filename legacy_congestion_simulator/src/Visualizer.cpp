#include "Visualizer.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

void Visualizer::printHeader(const std::string& title) {
    std::cout << "\n========================================================================\n";
    std::cout << "   " << title << "\n";
    std::cout << "========================================================================\n\n";
}

void Visualizer::printProgressBar(double current, double total, int width) {
    if (total <= 0.0) return;
    double ratio = current / total;
    if (ratio > 1.0) ratio = 1.0;
    int filled = static_cast<int>(ratio * width);

    std::cout << "\rProgress: [";
    for (int i = 0; i < width; ++i) {
        if (i < filled) std::cout << "=";
        else if (i == filled) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << (ratio * 100.0) << "% " << std::flush;
}

void Visualizer::printLiveDashboard(double currentTime, double maxTime, double cwnd,
                                     double throughputMbps, size_t queueLen, size_t queueCap,
                                     uint64_t totalDrops, const std::string& status) {
    std::cout << "\r[Time: " << std::fixed << std::setprecision(2) << currentTime << "/" << maxTime << "s] "
              << "| CWND: " << std::setprecision(1) << std::setw(5) << cwnd << " MSS "
              << "| Rate: " << std::setprecision(2) << std::setw(6) << throughputMbps << " Mbps "
              << "| Queue: " << std::setw(2) << queueLen << "/" << std::setw(2) << queueCap << " "
              << "| Drops: " << std::setw(4) << totalDrops << " "
              << "| " << status << "      " << std::flush;
}

void Visualizer::printNetworkTopology(const Network& net) {
    std::cout << "---------------------- Network Topology Graph ----------------------\n";
    for (const auto& node : net.getNodes()) {
        std::cout << " [" << node->getTypeString() << " " << node->getId() << ": " << node->getName() << "]\n";
        auto outLinks = const_cast<Network&>(net).getOutgoingLinks(node->getId());
        for (const auto* link : outLinks) {
            const Node* dest = net.getNode(link->getToNodeId());
            std::string destName = dest ? dest->getName() : "Unknown";
            std::cout << "    |---> Link " << link->getId() << " -> " << destName
                      << " (BW: " << link->getBandwidthMbps() << " Mbps, Delay: "
                      << (link->getPropagationDelaySec() * 1000.0) << " ms, Queue: "
                      << link->getQueueType() << " [" << link->getQueueCapacity() << " pkts])\n";
        }
    }
    std::cout << "--------------------------------------------------------------------\n\n";
}

void Visualizer::drawASCIIChart(const std::vector<double>& timeVec,
                                const std::vector<double>& valueVec,
                                const std::string& yAxisLabel,
                                int height, int width) {
    if (timeVec.empty() || valueVec.empty() || timeVec.size() != valueVec.size()) {
        std::cout << "[Visualizer] No metric data available to draw chart.\n";
        return;
    }

    double minVal = *std::min_element(valueVec.begin(), valueVec.end());
    double maxVal = *std::max_element(valueVec.begin(), valueVec.end());
    if (minVal == maxVal) {
        maxVal = minVal + 1.0;
    }

    double minTime = timeVec.front();
    double maxTime = timeVec.back();
    if (minTime == maxTime) {
        maxTime = minTime + 1.0;
    }

    // Grid matrix
    std::vector<std::string> grid(height, std::string(width, ' '));

    // Plot data points into ASCII grid
    for (size_t i = 0; i < timeVec.size(); ++i) {
        double t = timeVec[i];
        double v = valueVec[i];

        int col = static_cast<int>(((t - minTime) / (maxTime - minTime)) * (width - 1));
        int row = height - 1 - static_cast<int>(((v - minVal) / (maxVal - minVal)) * (height - 1));

        if (col >= 0 && col < width && row >= 0 && row < height) {
            grid[row][col] = '*';
        }
    }

    std::cout << "\n=================== " << yAxisLabel << " Timeline Graph ===================\n";
    for (int r = 0; r < height; ++r) {
        double currentYVal = maxVal - (static_cast<double>(r) / (height - 1)) * (maxVal - minVal);
        std::cout << std::setw(8) << std::fixed << std::setprecision(1) << currentYVal << " | " << grid[r] << "\n";
    }

    std::cout << "         +" << std::string(width, '-') << "\n";
    std::cout << "Time(s): " << std::setw(5) << minTime << std::string(width / 2 - 5, ' ')
              << std::setw(5) << (minTime + maxTime) / 2.0 << std::string(width / 2 - 5, ' ')
              << std::setw(5) << maxTime << "\n";
    std::cout << "====================================================================\n\n";
}

void Visualizer::printSummaryReport(const SummaryStats& stats,
                                    const std::string& scenarioName,
                                    const std::string& ccName,
                                    const std::string& aqmName) {
    std::cout << "\n========================================================================\n";
    std::cout << "              SIMULATION RESULTS & PERFORMANCE SUMMARY                  \n";
    std::cout << "========================================================================\n";
    std::cout << " Scenario:               " << scenarioName << "\n";
    std::cout << " Congestion Algorithm:   " << ccName << "\n";
    std::cout << " Queue Management (AQM): " << aqmName << "\n";
    std::cout << " Simulation Duration:    " << std::fixed << std::setprecision(2) << stats.durationSec << " seconds\n";
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << " Total Packets Sent:      " << stats.totalPacketsSent << " pkts\n";
    std::cout << " Total Packets Delivered: " << stats.totalPacketsDelivered << " pkts\n";
    std::cout << " Total Packets Dropped:   " << stats.totalPacketsDropped << " pkts\n";
    std::cout << " Packet Loss Rate:        " << std::setprecision(2) << stats.packetLossRatePercent << " %\n";
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << " Average Throughput:      " << std::setprecision(3) << stats.avgThroughputMbps << " Mbps\n";
    std::cout << " Peak Throughput:         " << std::setprecision(3) << stats.peakThroughputMbps << " Mbps\n";
    std::cout << " Peak CWND Reached:       " << std::setprecision(1) << stats.peakCwnd << " MSS\n";
    std::cout << " Average Buffer Load:     " << std::setprecision(2) << stats.avgBufferOccupancyPercent << " %\n";
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << " Min Round Trip Time:     " << std::setprecision(2) << stats.minRttMs << " ms\n";
    std::cout << " Avg Round Trip Time:     " << std::setprecision(2) << stats.avgRttMs << " ms\n";
    std::cout << " Max Round Trip Time:     " << std::setprecision(2) << stats.maxRttMs << " ms\n";
    std::cout << " Average Network Jitter:  " << std::setprecision(2) << stats.avgJitterMs << " ms\n";
    std::cout << "========================================================================\n\n";
}
