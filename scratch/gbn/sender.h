#pragma once

#include "ns3/applications-module.h"

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
    void ScheduleTx();
    /// Send a packet.
    void SendPacket();

    Ptr<Socket> m_socket;   //!< The transmission socket.
    Address m_peer;         //!< The destination address.
    uint32_t m_nPackets;    //!< The number of packets to send.
    DataRate m_dataRate;    //!< The data rate to use.
    EventId m_sendEvent;    //!< Send event.
    bool m_running;         //!< True if the application is running.
    uint32_t m_packetsSent; //!< The number of packets sent.

    uint32_t m_baseSequenceNumber;
    uint32_t m_nextSequenceNumber;
};
