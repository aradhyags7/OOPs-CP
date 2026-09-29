#include "CongestionControl.hpp"
#include <algorithm>
#include <cmath>

// ==================== TCP Tahoe Implementation ====================

void TCPTahoe::onAckReceived(double ackTime, double rtt, uint64_t ackSeq) {
    (void)ackTime;
    (void)rtt;
    (void)ackSeq;

    duplicateAcks = 0;
    if (cwnd < ssthresh) {
        // Slow Start Phase
        cwnd += 1.0;
        state = TCPState::SLOW_START;
    } else {
        // Congestion Avoidance Phase
        cwnd += 1.0 / cwnd;
        state = TCPState::CONGESTION_AVOIDANCE;
    }
}

void TCPTahoe::onDuplicateAck(double ackTime) {
    (void)ackTime;
    duplicateAcks++;
    if (duplicateAcks == 3) {
        // Fast Retransmit: Reset CWND to 1 MSS
        ssthresh = std::max(2.0, cwnd / 2.0);
        cwnd = 1.0;
        state = TCPState::SLOW_START;
        duplicateAcks = 0;
    }
}

void TCPTahoe::onTimeout(double currentTime) {
    (void)currentTime;
    ssthresh = std::max(2.0, cwnd / 2.0);
    cwnd = 1.0;
    state = TCPState::SLOW_START;
    duplicateAcks = 0;
}

// ==================== TCP Reno Implementation ====================

void TCPReno::onAckReceived(double ackTime, double rtt, uint64_t ackSeq) {
    (void)ackTime;
    (void)rtt;
    (void)ackSeq;

    if (state == TCPState::FAST_RECOVERY) {
        // Full ACK received: exit fast recovery
        cwnd = ssthresh;
        state = TCPState::CONGESTION_AVOIDANCE;
        duplicateAcks = 0;
        return;
    }

    duplicateAcks = 0;
    if (cwnd < ssthresh) {
        // Slow Start
        cwnd += 1.0;
        state = TCPState::SLOW_START;
    } else {
        // Congestion Avoidance
        cwnd += 1.0 / cwnd;
        state = TCPState::CONGESTION_AVOIDANCE;
    }
}

void TCPReno::onDuplicateAck(double ackTime) {
    (void)ackTime;
    duplicateAcks++;
    if (duplicateAcks == 3) {
        // Fast Recovery & Fast Retransmit
        ssthresh = std::max(2.0, cwnd / 2.0);
        cwnd = ssthresh + 3.0; // Window inflation
        state = TCPState::FAST_RECOVERY;
    } else if (duplicateAcks > 3 && state == TCPState::FAST_RECOVERY) {
        // Inflate CWND for each additional dup ACK
        cwnd += 1.0;
    }
}

void TCPReno::onTimeout(double currentTime) {
    (void)currentTime;
    ssthresh = std::max(2.0, cwnd / 2.0);
    cwnd = 1.0;
    state = TCPState::SLOW_START;
    duplicateAcks = 0;
}

// ==================== TCP Cubic Implementation ====================

void TCPCubic::onAckReceived(double ackTime, double rtt, uint64_t ackSeq) {
    (void)ackSeq;
    duplicateAcks = 0;

    if (cwnd < ssthresh) {
        cwnd += 1.0;
        state = TCPState::SLOW_START;
        return;
    }

    state = TCPState::CONGESTION_AVOIDANCE;
    double t = ackTime - epochStart;
    if (t < 0.0) t = 0.0;

    // Cubic window growth formula: W_cubic(t) = C*(t-K)^3 + W_max
    double targetCubic = C * std::pow(t - K, 3) + wMax;
    if (targetCubic < 1.0) targetCubic = 1.0;

    // TCP-friendly region approximation
    double safeRtt = std::max(0.001, rtt);
    double targetReno = wMax * beta + (3.0 * (1.0 - beta) / (1.0 + beta)) * (t / safeRtt);

    double target = std::max(targetCubic, targetReno);

    if (target > cwnd) {
        cwnd += (target - cwnd) / cwnd;
    } else {
        cwnd += 1.0 / cwnd;
    }
}

void TCPCubic::onDuplicateAck(double ackTime) {
    duplicateAcks++;
    if (duplicateAcks == 3) {
        epochStart = ackTime;
        wMax = cwnd;
        ssthresh = std::max(2.0, cwnd * beta);
        cwnd = ssthresh;
        K = std::cbrt(wMax * (1.0 - beta) / C);
        state = TCPState::CONGESTION_AVOIDANCE;
    }
}

void TCPCubic::onTimeout(double currentTime) {
    epochStart = currentTime;
    wMax = cwnd;
    ssthresh = std::max(2.0, cwnd * beta);
    cwnd = 1.0;
    K = std::cbrt(wMax * (1.0 - beta) / C);
    state = TCPState::SLOW_START;
    duplicateAcks = 0;
}

// ==================== TCP BBR Implementation ====================

void TCPBBR::onAckReceived(double ackTime, double rtt, uint64_t ackSeq) {
    (void)ackTime;
    (void)ackSeq;
    duplicateAcks = 0;

    double sampleBw = 1.0 / std::max(0.001, rtt); // Estimated packets per second
    btlBw = std::max(btlBw, sampleBw);
    if (rtProp == 1.0 || rtt < rtProp) {
        rtProp = std::max(0.001, rtt);
    }

    // Bandwidth Delay Product (BDP) estimation
    double bdp = btlBw * rtProp;
    double targetCwnd = bdp * pacingGain + 4.0;

    if (targetCwnd > cwnd) {
        cwnd += std::min(1.0, (targetCwnd - cwnd) * 0.1);
    } else if (targetCwnd < cwnd) {
        cwnd -= std::min(0.5, (cwnd - targetCwnd) * 0.05);
    }

    cwnd = std::max(2.0, cwnd);
    if (cwnd < ssthresh) {
        state = TCPState::SLOW_START;
    } else {
        state = TCPState::CONGESTION_AVOIDANCE;
    }
}

void TCPBBR::onDuplicateAck(double ackTime) {
    (void)ackTime;
    duplicateAcks++;
    if (duplicateAcks >= 3) {
        // BBR adjusts pacing rate and target CWND rather than drastic drops
        cwnd = std::max(2.0, cwnd * 0.85);
    }
}

void TCPBBR::onTimeout(double currentTime) {
    (void)currentTime;
    cwnd = std::max(2.0, cwnd / 2.0);
    duplicateAcks = 0;
    state = TCPState::SLOW_START;
}
