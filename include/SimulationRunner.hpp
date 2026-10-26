#ifndef SIMULATION_RUNNER_HPP
#define SIMULATION_RUNNER_HPP

#include "FirewallEngine.hpp"
#include <string>
#include <cstddef>

using namespace std;

class SimulationRunner {
private:
    FirewallEngine& engine;

public:
    explicit SimulationRunner(FirewallEngine& eng) : engine(eng) {}

    void runScenarioA_BasicFiltering();
    void runScenarioB_PortFiltering();
    void runScenarioC_RulePriority();
    void runScenarioD_SubnetFiltering();
    void runScenarioE_AuditLogging();
    void runScenarioF_ConfigurationPersistence();
    void runScenarioG_BatchBenchmark(size_t packetCount = 50000);

    static void printHeader(const std::string& title);
    static void printResultRow(const EvaluationResult& res, const Packet& pkt);
};

#endif // SIMULATION_RUNNER_HPP
