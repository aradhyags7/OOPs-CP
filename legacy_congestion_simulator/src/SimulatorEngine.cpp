#include "SimulatorEngine.hpp"
#include "Visualizer.hpp"
#include <iostream>
#include <memory>

void SimulatorEngine::reset() {
    currentTime = 0.0;
    scheduler.clear();
    network.clear();
    metrics.clear();
    dynamicThrottlingEnabled = false;
}

void SimulatorEngine::setupScenario(SimulationScenario scenario, const std::string& ccAlg, const std::string& aqmAlg) {
    reset();
    currentCcAlgorithm = ccAlg;
    currentAqmAlgorithm = aqmAlg;

    // Factory helper for Congestion Control
    auto createCC = [&](const std::string& algName) -> std::unique_ptr<CongestionControlAlgorithm> {
        if (algName == "Tahoe") return std::make_unique<TCPTahoe>();
        if (algName == "Cubic") return std::make_unique<TCPCubic>();
        if (algName == "BBR")   return std::make_unique<TCPBBR>();
        return std::make_unique<TCPReno>(); // Default Reno
    };

    // Factory helper for Queue Manager
    auto createQueue = [&](const std::string& qName, size_t cap = 20) -> std::unique_ptr<QueueManager> {
        if (qName == "RED") return std::make_unique<REDQueue>(cap, 5.0, 15.0, 0.1, 0.005);
        return std::make_unique<DropTailQueue>(cap);
    };

    switch (scenario) {
        case SimulationScenario::DUMBBELL_BOTTLENECK: {
            currentScenarioName = "Dumbbell Topology Bottleneck Congestion";

            // Create Nodes
            int h1 = network.addNode(std::make_unique<HostNode>(0, "Host_Src1", createCC(ccAlg), std::make_unique<CBRGenerator>(1, 0, 2, 12.0, 1000)));
            int h2 = network.addNode(std::make_unique<HostNode>(1, "Host_Src2", createCC(ccAlg), std::make_unique<CBRGenerator>(2, 1, 3, 12.0, 1000)));
            int h3 = network.addNode(std::make_unique<HostNode>(2, "Host_Dst1", createCC("Reno"), nullptr));
            int h4 = network.addNode(std::make_unique<HostNode>(3, "Host_Dst2", createCC("Reno"), nullptr));

            int r1 = network.addNode(std::make_unique<RouterNode>(4, "Router_R1"));
            int r2 = network.addNode(std::make_unique<RouterNode>(5, "Router_R2"));

            // Access links to Router R1 (100 Mbps, 2ms)
            network.addLink(std::make_unique<Link>(0, h1, r1, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(1, h2, r1, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));

            // BOTTLENECK LINK (R1 -> R2): 10 Mbps, 20ms delay
            bottleneckLinkId = network.addLink(std::make_unique<Link>(2, r1, r2, 10.0, 0.020, 0.0, createQueue(aqmAlg, 20)));

            // Egress links from Router R2 (100 Mbps, 2ms)
            network.addLink(std::make_unique<Link>(3, r2, h3, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(4, r2, h4, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));

            // Reverse return path links for ACKs
            network.addLink(std::make_unique<Link>(5, h3, r2, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(6, h4, r2, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(7, r2, r1, 100.0, 0.020, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(8, r1, h1, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(9, r1, h2, 100.0, 0.002, 0.0, createQueue("DropTail", 100)));

            break;
        }

        case SimulationScenario::TCP_ALGORITHM_SHOWDOWN: {
            currentScenarioName = "TCP Algorithm Showdown (" + ccAlg + ")";

            int src = network.addNode(std::make_unique<HostNode>(0, "SrcHost", createCC(ccAlg), std::make_unique<CBRGenerator>(1, 0, 1, 15.0, 1000)));
            int dst = network.addNode(std::make_unique<HostNode>(1, "DstHost", createCC("Reno"), nullptr));
            int r1  = network.addNode(std::make_unique<RouterNode>(2, "R1"));
            int r2  = network.addNode(std::make_unique<RouterNode>(3, "R2"));

            network.addLink(std::make_unique<Link>(0, src, r1, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));
            bottleneckLinkId = network.addLink(std::make_unique<Link>(1, r1, r2, 10.0, 0.015, 0.005 /* 0.5% loss */, createQueue(aqmAlg, 25)));
            network.addLink(std::make_unique<Link>(2, r2, dst, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));

            // Reverse links
            network.addLink(std::make_unique<Link>(3, dst, r2, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(4, r2, r1, 100.0, 0.015, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(5, r1, src, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));

            break;
        }

        case SimulationScenario::AQM_DROP_TAIL_VS_RED: {
            currentScenarioName = "AQM Policy Comparison (" + aqmAlg + " Queue)";

            int src = network.addNode(std::make_unique<HostNode>(0, "BurstSrc", createCC(ccAlg), std::make_unique<BurstGenerator>(1, 0, 1, 25.0, 1000, 0.1, 0.1)));
            int dst = network.addNode(std::make_unique<HostNode>(1, "SinkDst", createCC("Reno"), nullptr));
            int r1  = network.addNode(std::make_unique<RouterNode>(2, "R1"));
            int r2  = network.addNode(std::make_unique<RouterNode>(3, "R2"));

            network.addLink(std::make_unique<Link>(0, src, r1, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));
            bottleneckLinkId = network.addLink(std::make_unique<Link>(1, r1, r2, 8.0, 0.010, 0.0, createQueue(aqmAlg, 25)));
            network.addLink(std::make_unique<Link>(2, r2, dst, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));

            // Reverse links
            network.addLink(std::make_unique<Link>(3, dst, r2, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(4, r2, r1, 100.0, 0.010, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(5, r1, src, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));

            break;
        }

        case SimulationScenario::DYNAMIC_LINK_THROTTLING: {
            currentScenarioName = "Dynamic Link Bandwidth Throttling";
            dynamicThrottlingEnabled = true;

            int src = network.addNode(std::make_unique<HostNode>(0, "SrcHost", createCC(ccAlg), std::make_unique<CBRGenerator>(1, 0, 1, 15.0, 1000)));
            int dst = network.addNode(std::make_unique<HostNode>(1, "DstHost", createCC("Reno"), nullptr));
            int r1  = network.addNode(std::make_unique<RouterNode>(2, "R1"));
            int r2  = network.addNode(std::make_unique<RouterNode>(3, "R2"));

            network.addLink(std::make_unique<Link>(0, src, r1, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));
            bottleneckLinkId = network.addLink(std::make_unique<Link>(1, r1, r2, 10.0, 0.015, 0.0, createQueue(aqmAlg, 25)));
            network.addLink(std::make_unique<Link>(2, r2, dst, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));

            // Reverse links
            network.addLink(std::make_unique<Link>(3, dst, r2, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(4, r2, r1, 100.0, 0.015, 0.0, createQueue("DropTail", 100)));
            network.addLink(std::make_unique<Link>(5, r1, src, 100.0, 0.001, 0.0, createQueue("DropTail", 100)));

            break;
        }

        default:
            setupScenario(SimulationScenario::DUMBBELL_BOTTLENECK, ccAlg, aqmAlg);
            return;
    }

    // Update Dijkstra routing tables across all routers
    network.updateRoutingTables();

    // Schedule initial traffic generation events for source hosts
    for (const auto& node : network.getNodes()) {
        if (node->getType() == NodeType::HOST) {
            HostNode* host = static_cast<HostNode*>(node.get());
            if (host->getTrafficGenerator()) {
                double firstPktTime = host->getTrafficGenerator()->getNextPacketTime(0.0);
                scheduler.scheduleEvent(firstPktTime, EventType::PACKET_GENERATE, host->getId());
            }
        }
    }

    // Schedule initial metrics logging event
    scheduler.scheduleEvent(metricsLogInterval, EventType::PERIODIC_METRICS_LOG, 0);
}

void SimulatorEngine::run(double durationSec, bool liveVisual) {
    simulationDuration = durationSec;
    liveVisualizationEnabled = liveVisual;

    if (liveVisualizationEnabled) {
        Visualizer::printHeader("STARTING SIMULATION: " + currentScenarioName);
        Visualizer::printNetworkTopology(network);
    }

    while (scheduler.hasEvents() && currentTime <= simulationDuration) {
        Event evt = scheduler.popNextEvent();
        currentTime = evt.time;

        if (currentTime > simulationDuration) break;

        applyDynamicThrottlingCheck();
        processEvent(evt);
    }

    if (liveVisualizationEnabled) {
        SummaryStats stats = metrics.computeSummaryStats(simulationDuration);
        Visualizer::printSummaryReport(stats, currentScenarioName, currentCcAlgorithm, currentAqmAlgorithm);

        // Render ASCII CWND Chart
        std::vector<double> timeVec, cwndVec;
        for (const auto& s : metrics.getSamples()) {
            timeVec.push_back(s.timestamp);
            cwndVec.push_back(s.cwnd);
        }
        Visualizer::drawASCIIChart(timeVec, cwndVec, "CWND (MSS)", 10, 50);
    }

    // Export metrics to CSV
    metrics.exportToCSV("simulation_metrics.csv");
    metrics.exportCwndTraceToCSV("cwnd_trace.csv");
    std::cout << "[SimulatorEngine] Telemetry exported to 'simulation_metrics.csv' and 'cwnd_trace.csv'.\n";
}

void SimulatorEngine::applyDynamicThrottlingCheck() {
    if (!dynamicThrottlingEnabled || bottleneckLinkId == -1) return;

    Link* link = network.getLink(bottleneckLinkId);
    if (!link) return;

    if (currentTime >= throttleStartTime && currentTime < throttleEndTime) {
        if (link->getBandwidthMbps() != throttledBandwidth) {
            link->setBandwidthMbps(throttledBandwidth);
            if (liveVisualizationEnabled) {
                std::cout << "\n[EVENT " << currentTime << "s] Dynamic Link Throttled: Bandwidth dropped to "
                          << throttledBandwidth << " Mbps!\n";
            }
        }
    } else if (currentTime >= throttleEndTime) {
        if (link->getBandwidthMbps() != originalBandwidth) {
            link->setBandwidthMbps(originalBandwidth);
            if (liveVisualizationEnabled) {
                std::cout << "\n[EVENT " << currentTime << "s] Dynamic Link Recovered: Bandwidth restored to "
                          << originalBandwidth << " Mbps!\n";
            }
        }
    }
}

void SimulatorEngine::processEvent(const Event& evt) {
    switch (evt.type) {
        case EventType::PACKET_GENERATE:
            handlePacketGenerate(evt);
            break;
        case EventType::PACKET_ENQUEUE:
            handlePacketEnqueue(evt);
            break;
        case EventType::PACKET_TRANSMIT_START:
            handlePacketTransmitStart(evt);
            break;
        case EventType::PACKET_TRANSMIT_END:
            handlePacketTransmitEnd(evt);
            break;
        case EventType::PACKET_ARRIVE_NODE:
            handlePacketArriveNode(evt);
            break;
        case EventType::ACK_RECEIVE:
            handleAckReceive(evt);
            break;
        case EventType::TIMEOUT_RETRANSMISSION:
            handleTimeout(evt);
            break;
        case EventType::PERIODIC_METRICS_LOG:
            handlePeriodicMetricsLog(evt);
            break;
    }
}

void SimulatorEngine::handlePacketGenerate(const Event& evt) {
    HostNode* host = static_cast<HostNode*>(network.getNode(evt.nodeLinkOrFlowId));
    if (!host || !host->getTrafficGenerator()) return;

    // Check CWND limits if host is acting as TCP sender
    double cwnd = host->getCongestionControl() ? host->getCongestionControl()->getCwnd() : 100.0;
    uint64_t unacked = host->getPacketsSent() - host->getAcksReceived();

    if (unacked < static_cast<uint64_t>(cwnd)) {
        Packet pkt = host->getTrafficGenerator()->generateNextPacket(currentTime);
        host->incrementPacketsSent();

        // Route packet to first outgoing link
        Link* outLink = network.findLinkBetween(host->getId(), 4 /* Router R1 or R2 */);
        if (!outLink) {
            auto outLinks = network.getOutgoingLinks(host->getId());
            if (!outLinks.empty()) outLink = outLinks.front();
        }

        if (outLink) {
            scheduler.scheduleEvent(currentTime, EventType::PACKET_ENQUEUE, outLink->getId(), pkt);

            // Schedule retransmission timeout timer for this packet
            double rto = host->getRTO();
            scheduler.scheduleEvent(currentTime + rto, EventType::TIMEOUT_RETRANSMISSION, host->getId(), pkt);
        }
    }

    // Schedule next packet generation
    double nextTime = host->getTrafficGenerator()->getNextPacketTime(currentTime);
    if (nextTime <= simulationDuration) {
        scheduler.scheduleEvent(nextTime, EventType::PACKET_GENERATE, host->getId());
    }
}

void SimulatorEngine::handlePacketEnqueue(const Event& evt) {
    Link* link = network.getLink(evt.nodeLinkOrFlowId);
    if (!link) return;

    Packet pkt = evt.packet;
    bool enqueued = link->enqueuePacket(pkt, currentTime);

    if (enqueued && !link->isTransmitting()) {
        scheduler.scheduleEvent(currentTime, EventType::PACKET_TRANSMIT_START, link->getId());
    }
}

void SimulatorEngine::handlePacketTransmitStart(const Event& evt) {
    Link* link = network.getLink(evt.nodeLinkOrFlowId);
    if (!link || link->isQueueEmpty() || link->isTransmitting()) return;

    Packet pkt = link->dequeuePacket();
    double txDelay = link->getTransmissionDelaySec(pkt.sizeBytes);
    double propDelay = link->getPropagationDelaySec();

    link->setTransmitting(true, currentTime + txDelay);

    // Schedule transmit completion on link
    scheduler.scheduleEvent(currentTime + txDelay, EventType::PACKET_TRANSMIT_END, link->getId(), pkt);

    // Schedule arrival at destination node
    scheduler.scheduleEvent(currentTime + txDelay + propDelay, EventType::PACKET_ARRIVE_NODE, link->getToNodeId(), pkt);
}

void SimulatorEngine::handlePacketTransmitEnd(const Event& evt) {
    Link* link = network.getLink(evt.nodeLinkOrFlowId);
    if (!link) return;

    link->setTransmitting(false);
    link->addTransmittedBytes(evt.packet.sizeBytes);

    if (!link->isQueueEmpty()) {
        scheduler.scheduleEvent(currentTime, EventType::PACKET_TRANSMIT_START, link->getId());
    }
}

void SimulatorEngine::handlePacketArriveNode(const Event& evt) {
    Node* node = network.getNode(evt.targetNodeId);
    if (!node) return;

    Packet pkt = evt.packet;

    if (node->getType() == NodeType::ROUTER) {
        RouterNode* router = static_cast<RouterNode*>(node);
        int nextLink = router->getNextHopLink(pkt.destinationId);
        if (nextLink != -1) {
            scheduler.scheduleEvent(currentTime, EventType::PACKET_ENQUEUE, nextLink, pkt);
        }
    } else if (node->getType() == NodeType::HOST) {
        HostNode* host = static_cast<HostNode*>(node);
        bool generateAck = false;
        Packet ackPkt;

        host->processReceivedPacket(pkt, currentTime, generateAck, ackPkt);

        if (generateAck) {
            // Send ACK packet back to source
            Link* outLink = network.findLinkBetween(host->getId(), 5 /* Router R2 */);
            if (!outLink) {
                auto outLinks = network.getOutgoingLinks(host->getId());
                if (!outLinks.empty()) outLink = outLinks.front();
            }

            if (outLink) {
                scheduler.scheduleEvent(currentTime, EventType::PACKET_ENQUEUE, outLink->getId(), ackPkt);
            }
        } else if (pkt.type == PacketType::ACK) {
            // ACK reached originating host
            metrics.recordRTT(currentTime - pkt.originalPacketGenTime);
        }
    }
}

void SimulatorEngine::handleAckReceive(const Event& evt) {
    HostNode* host = static_cast<HostNode*>(network.getNode(evt.targetNodeId));
    if (!host) return;

    bool dummyAck = false;
    Packet outAck;
    host->processReceivedPacket(evt.packet, currentTime, dummyAck, outAck);
    metrics.recordRTT(currentTime - evt.packet.originalPacketGenTime);
}

void SimulatorEngine::handleTimeout(const Event& evt) {
    HostNode* host = static_cast<HostNode*>(network.getNode(evt.targetNodeId));
    if (!host) return;

    // Check if packet was already ACKed
    if (evt.packet.sequenceNum > 0) {
        host->handleTimeout(currentTime);
    }
}

void SimulatorEngine::handlePeriodicMetricsLog(const Event& evt) {
    (void)evt;

    // Find main source host (Host 0)
    HostNode* srcHost = static_cast<HostNode*>(network.getNode(0));
    Link* bLink = network.getLink((bottleneckLinkId != -1) ? bottleneckLinkId : 2);

    MetricSample sample;
    sample.timestamp = currentTime;
    sample.flowId = 1;

    if (srcHost && srcHost->getCongestionControl()) {
        sample.cwnd = srcHost->getCongestionControl()->getCwnd();
        sample.ssthresh = srcHost->getCongestionControl()->getSsthresh();
        sample.totalPacketsSent = srcHost->getPacketsSent();
        sample.totalPacketsDelivered = srcHost->getAcksReceived();
    }

    if (bLink) {
        sample.bufferOccupancyPercent = (static_cast<double>(bLink->getQueueSize()) / static_cast<double>(bLink->getQueueCapacity())) * 100.0;
        sample.totalPacketsDropped = bLink->getTotalPacketsDropped();

        double txTimeSec = metricsLogInterval;
        double bitsSent = static_cast<double>(bLink->getTotalBitsTransmitted());
        sample.throughputMbps = (bitsSent > 0) ? (bitsSent / (txTimeSec * 1e6)) : 0.0;
    }

    metrics.addSample(sample);

    if (liveVisualizationEnabled) {
        Visualizer::printLiveDashboard(currentTime, simulationDuration, sample.cwnd,
                                      sample.throughputMbps, bLink ? bLink->getQueueSize() : 0,
                                      bLink ? bLink->getQueueCapacity() : 0,
                                      sample.totalPacketsDropped,
                                      srcHost ? srcHost->getCongestionControl()->getStateString() : "Running");
    }

    // Schedule next periodic log
    if (currentTime + metricsLogInterval <= simulationDuration) {
        scheduler.scheduleEvent(currentTime + metricsLogInterval, EventType::PERIODIC_METRICS_LOG, 0);
    }
}
