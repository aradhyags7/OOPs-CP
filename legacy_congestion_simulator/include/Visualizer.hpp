#ifndef VISUALIZER_HPP
#define VISUALIZER_HPP

#include "Metrics.hpp"
#include "Network.hpp"
#include <vector>
#include <string>

class Visualizer {
public:
    static void printHeader(const std::string& title);
    static void printProgressBar(double current, double total, int width = 40);
    static void printLiveDashboard(double currentTime, double maxTime, double cwnd,
                                   double throughputMbps, size_t queueLen, size_t queueCap,
                                   uint64_t totalDrops, const std::string& status);

    static void printNetworkTopology(const Network& net);

    static void drawASCIIChart(const std::vector<double>& timeVec,
                               const std::vector<double>& valueVec,
                               const std::string& yAxisLabel,
                               int height = 12, int width = 60);

    static void printSummaryReport(const SummaryStats& stats,
                                   const std::string& scenarioName,
                                   const std::string& ccName,
                                   const std::string& aqmName);
};

#endif // VISUALIZER_HPP
