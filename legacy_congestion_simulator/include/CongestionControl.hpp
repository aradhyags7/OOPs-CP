#ifndef CONGESTION_CONTROL_HPP
#define CONGESTION_CONTROL_HPP

#include <string>
#include <cstdint>

enum class TCPState {
    SLOW_START,
    CONGESTION_AVOIDANCE,
    FAST_RECOVERY
};

class CongestionControlAlgorithm {
protected:
    double cwnd{1.0};           // Congestion window (in MSS units)
    double ssthresh{64.0};       // Slow start threshold (in MSS units)
    uint32_t mss{1000};          // MSS in bytes
    int duplicateAcks{0};
    TCPState state{TCPState::SLOW_START};

public:
    explicit CongestionControlAlgorithm(double initialCwnd = 1.0, double initialSsthresh = 64.0, uint32_t segmentSize = 1000)
        : cwnd(initialCwnd), ssthresh(initialSsthresh), mss(segmentSize) {}

    virtual ~CongestionControlAlgorithm() = default;

    virtual void onAckReceived(double ackTime, double rtt, uint64_t ackSeq) = 0;
    virtual void onDuplicateAck(double ackTime) = 0;
    virtual void onTimeout(double currentTime) = 0;
    virtual std::string getAlgorithmName() const = 0;

    double getCwnd() const { return cwnd; }
    double getSsthresh() const { return ssthresh; }
    uint32_t getMSS() const { return mss; }
    int getDuplicateAcks() const { return duplicateAcks; }
    std::string getStateString() const {
        switch (state) {
            case TCPState::SLOW_START:           return "Slow Start";
            case TCPState::CONGESTION_AVOIDANCE: return "Congestion Avoidance";
            case TCPState::FAST_RECOVERY:        return "Fast Recovery";
            default:                             return "Unknown";
        }
    }

    void reset() {
        cwnd = 1.0;
        ssthresh = 64.0;
        duplicateAcks = 0;
        state = TCPState::SLOW_START;
    }
};

class TCPTahoe : public CongestionControlAlgorithm {
public:
    TCPTahoe(double initialCwnd = 1.0, double initialSsthresh = 64.0)
        : CongestionControlAlgorithm(initialCwnd, initialSsthresh) {}

    void onAckReceived(double ackTime, double rtt, uint64_t ackSeq) override;
    void onDuplicateAck(double ackTime) override;
    void onTimeout(double currentTime) override;
    std::string getAlgorithmName() const override { return "TCP Tahoe"; }
};

class TCPReno : public CongestionControlAlgorithm {
public:
    TCPReno(double initialCwnd = 1.0, double initialSsthresh = 64.0)
        : CongestionControlAlgorithm(initialCwnd, initialSsthresh) {}

    void onAckReceived(double ackTime, double rtt, uint64_t ackSeq) override;
    void onDuplicateAck(double ackTime) override;
    void onTimeout(double currentTime) override;
    std::string getAlgorithmName() const override { return "TCP Reno"; }
};

class TCPCubic : public CongestionControlAlgorithm {
private:
    double wMax{0.0};
    double epochStart{0.0};
    double K{0.0};
    double C{0.4};
    double beta{0.7}; // Window reduction factor on loss

public:
    TCPCubic(double initialCwnd = 1.0, double initialSsthresh = 64.0)
        : CongestionControlAlgorithm(initialCwnd, initialSsthresh) {}

    void onAckReceived(double ackTime, double rtt, uint64_t ackSeq) override;
    void onDuplicateAck(double ackTime) override;
    void onTimeout(double currentTime) override;
    std::string getAlgorithmName() const override { return "TCP Cubic"; }
};

class TCPBBR : public CongestionControlAlgorithm {
private:
    double btlBw{0.0};      // Estimated Bottleneck Bandwidth (pkts/sec)
    double rtProp{1.0};     // Estimated Min RTT (sec)
    double pacingGain{1.0};

public:
    TCPBBR(double initialCwnd = 1.0, double initialSsthresh = 64.0)
        : CongestionControlAlgorithm(initialCwnd, initialSsthresh) {}

    void onAckReceived(double ackTime, double rtt, uint64_t ackSeq) override;
    void onDuplicateAck(double ackTime) override;
    void onTimeout(double currentTime) override;
    std::string getAlgorithmName() const override { return "TCP BBR"; }
};

#endif // CONGESTION_CONTROL_HPP
