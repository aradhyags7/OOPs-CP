#ifndef SIMULATOR_ENGINE_HPP
#define SIMULATOR_ENGINE_HPP

#include "Network.hpp"
#include "EventScheduler.hpp"
#include "Metrics.hpp"
#include <string>

enum class SimulationScenario {
    DUMBBELL_BOTTLENECK = 1,
    TCP_ALGORITHM_SHOWDOWN = 2,
    AQM_DROP_TAIL_VS_RED = 3,
    DYNAMIC_LINK_THROTTLING = 4,
    CUSTOM_TOPOLOGY = 5
};

class SimulatorEngine {
private:
    Network network;
    EventScheduler scheduler;
    MetricsCollector metrics;

    double currentTime{0.0};
    double simulationDuration{10.0};
    double metricsLogInterval{0.1}; // Log metrics every 100ms

    bool liveVisualizationEnabled{true};
    std::string currentScenarioName{"Custom Simulation"};
    std::string currentCcAlgorithm{"TCP Reno"};
    std::string currentAqmAlgorithm{"DropTail"};

    // Dynamic link throttling settings
    bool dynamicThrottlingEnabled{false};
    double throttleStartTime{3.0};
    double throttleEndTime{7.0};
    int bottleneckLinkId{-1};
    double originalBandwidth{10.0};
    double throttledBandwidth{2.0};

public:
    SimulatorEngine() = default;

    void setupScenario(SimulationScenario scenario, const std::string& ccAlg = "Reno", const std::string& aqmAlg = "DropTail");
    void setupCustomTopology();

    void run(double durationSec, bool liveVisual = true);

    const MetricsCollector& getMetrics() const { return metrics; }
    const Network& getNetwork() const { return network; }
    std::string getScenarioName() const { return currentScenarioName; }

private:
    void reset();

    // Event Handler Routines
    void processEvent(const Event& evt);

    void handlePacketGenerate(const Event& evt);
    void handlePacketEnqueue(const Event& evt);
    void handlePacketTransmitStart(const Event& evt);
    void handlePacketTransmitEnd(const Event& evt);
    void handlePacketArriveNode(const Event& evt);
    void handleAckReceive(const Event& evt);
    void handleTimeout(const Event& evt);
    void handlePeriodicMetricsLog(const Event& evt);

    void applyDynamicThrottlingCheck();
};

#endif // SIMULATOR_ENGINE_HPP
