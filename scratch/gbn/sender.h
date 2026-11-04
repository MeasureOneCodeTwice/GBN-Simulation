#pragma once

#include "ns3/applications-module.h"

#include "./sequence-number.h"

using namespace ns3;

//based on `examples/tutorial/tutorial-app.h`
class GbnSender : public Application {
  public:
    GbnSender();
    ~GbnSender() override;

    static TypeId GetTypeId();

    /**
     * Setup the socket.
     * @param socket The socket.
     * @param address The destination address.
     * @param packetSize The packet size to transmit.
     * @param nPackets The number of packets to transmit.
     * @param dataRate the data rate to use.
     */
    void Setup(Ipv4Address address,
               uint32_t nPackets,
               DataRate dataRate
    );

  private:
    void StartApplication() override;
    void StopApplication() override;

    void OnRecv(Ptr<Socket> sock);

    /// Schedule a new transmission.
    void ScheduleTx(::SequenceNumber);
    /// Send a packet.
    void SendPacket(::SequenceNumber);
    // Sends all packets in the window from a given point
    void SendPackets(::SequenceNumber from);

    Ptr<Socket> m_socket;   //!< The transmission socket.
    Address m_peer;         //!< The destination address.
    uint32_t m_nPackets;    //!< The number of packets to send.
    DataRate m_dataRate;    //!< The data rate to use.
    EventId m_sendEvent;    //!< Send event.
    bool m_running;         //!< True if the application is running.
    uint32_t m_packetsSent; //!< The number of packets sent.

    ::SequenceNumber m_baseSequenceNumber; //!< Oldest un-acked packet
    ::SequenceNumber m_nextSequenceNumber; //!< Smallest unused sequence number

    struct Metrics {
        uint32_t totalPacketsTransmitted;
        uint32_t packetRetransmissions;
        uint32_t timeouts;
        Time completionTime;
    };
    Metrics m_metrics;
};
