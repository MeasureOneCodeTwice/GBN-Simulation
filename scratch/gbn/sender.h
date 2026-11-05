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

  private:
    void StartApplication() override;
    void StopApplication() override;

    void OnRecv(Ptr<Socket> sock);

    /// Send a packet.
    void SendPacket(::SequenceNumber);
    // Sends all packets in the window from a given point
    void SendPackets(::SequenceNumber from, uint32_t numPackets);

    Ptr<Socket> m_socket;     //!< The transmission socket.
    Address m_peer;           //!< The destination address.
    uint32_t m_nPackets;      //!< The number of packets to send
    uint32_t m_nPacketsSent;  //!< The number of packets to send
    bool m_running;           //!< True if the application is running.

    ::SequenceNumber m_baseSequenceNumber; //!< Oldest un-acked packet
    ::SequenceNumber m_nextSequenceNumber; //!< Smallest unused sequence number

    TimedQueue<::SequenceNumber> m_timeoutQueue;
    void onTimeout(::SequenceNumber);

    struct Metrics {
        uint32_t totalPacketsTransmitted;
        uint32_t packetRetransmissions;
        uint32_t timeouts;
        Time completionTime;
    };
    Metrics m_metrics;
};
