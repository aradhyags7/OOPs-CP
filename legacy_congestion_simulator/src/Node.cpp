#include "Node.hpp"
#include <cmath>
#include <algorithm>

HostNode::HostNode(int nodeId, const std::string& nodeName,
                   std::unique_ptr<CongestionControlAlgorithm> cc,
                   std::unique_ptr<TrafficGenerator> gen)
    : Node(nodeId, nodeName, NodeType::HOST),
      congestionControl(std::move(cc)), trafficGen(std::move(gen)) {
    if (!congestionControl) {
        congestionControl = std::make_unique<TCPReno>();
    }
}

Packet HostNode::processReceivedPacket(const Packet& pkt, double currentTime, bool& generateAckOut, Packet& ackPktOut) {
    generateAckOut = false;
    packetsReceived++;

    if (pkt.type == PacketType::DATA) {
        // Data packet received: emit ACK back to source
        generateAckOut = true;
        acksSent++;
        ackPktOut = Packet(0, id, pkt.sourceId, pkt.flowId, 64 /* 64-byte ACK */,
                           0, PacketType::ACK, currentTime);
        ackPktOut.ackSequenceNum = pkt.sequenceNum;
        ackPktOut.originalPacketGenTime = pkt.generationTime;
        return pkt;
    } else if (pkt.type == PacketType::ACK) {
        // ACK received: update Congestion Window and RTT
        acksReceived++;
        lastAckedSequenceNum = pkt.ackSequenceNum;

        double sampleRTT = currentTime - pkt.originalPacketGenTime;
        if (sampleRTT > 0.0) {
            updateRTT(sampleRTT);
        }

        if (congestionControl) {
            congestionControl->onAckReceived(currentTime, sampleRTT, pkt.ackSequenceNum);
        }
    }

    return pkt;
}

void HostNode::updateRTT(double sampleRTT) {
    // Karn / Jacobson RTT Estimation Algorithm
    double err = sampleRTT - srtt;
    srtt += alpha * err;
    rttvar += beta * (std::abs(err) - rttvar);
    rto = srtt + 4.0 * rttvar;

    // Bound RTO between 0.1s and 3.0s
    if (rto < 0.1) rto = 0.1;
    if (rto > 3.0) rto = 3.0;
}

void HostNode::handleTimeout(double currentTime) {
    retransmissions++;
    // Exponential RTO backoff
    rto = std::min(6.0, rto * 2.0);

    if (congestionControl) {
        congestionControl->onTimeout(currentTime);
    }
}
