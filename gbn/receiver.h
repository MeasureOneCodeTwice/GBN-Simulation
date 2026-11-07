#pragma once

#include "ns3/applications-module.h"

#include "./utils/sequence-number.h"

using namespace ns3;

//based on `examples/tutorial/tutorial-app.h`
class GbnReceiver : public Application {
  public:
    GbnReceiver();
    ~GbnReceiver() override;

    static TypeId GetTypeId();

    void Setup(Ipv4Address bindAddress);

  private:
    void OnRecv(Ptr<Socket>);
    void StartApplication() override;
    void StopApplication() override;

    /// Send a packet.
    void SendPacket();

    Ptr<Socket> m_socket;      //!< The transmission socket.
    bool m_running;            //!< True if the application is running.
    Ipv4Address m_bindAddress; //!< Address to bind socket to
    ::SequenceNumber m_sequenceNumber; //!< Sequence number of next expected packet.
};

