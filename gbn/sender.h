#pragma once

#include "ns3/applications-module.h"

#include "./utils/sequence-number.h"
#include "./utils/timed-queue.h"

using namespace ns3;

//based on `examples/tutorial/tutorial-app.h`
class GbnSender : public Application {
  public:
    GbnSender();
    ~GbnSender() override;

    static TypeId GetTypeId();

    /**
     * Setup the socket.
     * @param address The destination address.
     * @param nPackets The number of packets to transmit.
     */
    void Setup(Ipv4Address address, uint32_t nPackets);

    struct Metrics {
        uint32_t totalPacketsTransmitted;
        uint32_t packetRetransmissions;
        uint32_t timeouts;
        DataRate goodput;
        Time endTime;
    };
    Metrics getMetrics() const;


  private:
    void StartApplication() override;
    void StopApplication() override;

    void OnRecv(Ptr<Socket> sock);

    /// Send a packet.
    void SendPacket(::SequenceNumber);
    // Sends all packets in range and returns number of packets sent.
    // including from and to
    int SendPackets(::SequenceNumber from, ::SequenceNumber to);

    Ptr<Socket> m_socket;         //!< The transmission socket.
    Address m_peer;               //!< The destination address.
    uint32_t m_nPackets;          //!< The number of packets to send
    uint32_t m_nPacketsAcked;     //!< The number of packets ACK'd by the recipient
    bool m_running;               //!< True if the application is running.
                                  //
    struct RawMetrics {
        uint32_t totalPacketsTransmitted;
        uint32_t packetRetransmissions;
        uint32_t timeouts;
        Time startTime;
        Time lastAckedPacketTime;
    };
    RawMetrics m_metrics;

    ::SequenceNumber m_baseSequenceNumber; //!< Oldest un-acked packet
    ::SequenceNumber m_nextSequenceNumber; //!< Smallest unused sequence number
    ::SequenceNumber getWindowEnd() const; //!< The last sequence number in the window
    bool allPacketsReceived() const;

    TimedQueue<::SequenceNumber> m_timeoutQueue;
    void onTimeout(::SequenceNumber);

};
