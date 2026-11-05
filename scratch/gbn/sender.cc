#include "./sender.h"
#include "./config.h"
#include "./utils/packet.h"
#include "./utils/logging.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

#include <memory.h>
#include <assert.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GbnSender");

void GbnSender::onTimeout(::SequenceNumber sequenceNumber) {
    assert(sequenceNumber == m_baseSequenceNumber);
    this->SendPackets(this->m_baseSequenceNumber, GbnSenderConfig::windowSize);
}

//based on `examples/tutorial/tutorial-app.cc`
GbnSender::GbnSender()
    : m_socket(nullptr),
      m_peer(),
      m_nPackets(0),
      m_running(false),
      m_baseSequenceNumber(GbnConfig::initialSequenceNumber),
      m_nextSequenceNumber(GbnConfig::initialSequenceNumber),
      m_timeoutQueue(
              [this](::SequenceNumber num){ this->onTimeout(num); },
              GbnSenderConfig::retransmissionTimeout
      )
{
    assert(!::SequenceNumber(8).InWindow(9, 0));
}

GbnSender::~GbnSender()
{
    m_socket = nullptr;
}

/* static */
TypeId
GbnSender::GetTypeId()
{
    static TypeId tid = TypeId("GbnSender")
                            .SetParent<Application>()
                            .SetGroupName("GbnSender")
                            .AddConstructor<GbnSender>();
    return tid;
}

void
GbnSender::Setup(Ipv4Address address, uint32_t nPackets) {
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_peer = InetSocketAddress(address, ::GbnConfig::port);
    m_nPackets = nPackets;
}

void
GbnSender::StartApplication()
{
    m_running = true;
    m_socket->Bind();
    m_socket->Connect(m_peer);

    m_socket->SetRecvCallback(
        MakeCallback(&GbnSender::OnRecv, this)
    );

    SendPackets(m_baseSequenceNumber, GbnSenderConfig::windowSize);
}

void
GbnSender::StopApplication()
{
    m_running = false;
    if (m_socket)
    {
        m_socket->Close();
    }
}



void GbnSender::SendPackets(::SequenceNumber from, uint32_t numPackets) {

    NS_TIMESTAMP_LOG_DEBUG(
        "Sending packets from "
        << from.to_string()
        << " to "
        << (from + numPackets).to_string();
    );

    for(uint32_t i = 0; i < numPackets; i++) {
        SendPacket(from + i);
    }
    m_nextSequenceNumber = from + numPackets - 1;
    m_metrics.totalPacketsTransmitted += numPackets;
}

void
GbnSender::SendPacket(const ::SequenceNumber sequenceNumber)
{
    auto packet = GbnPacket(
            sequenceNumber,
            GbnSenderConfig::getPayloadContent()
    ).Serialize();

    NS_TIMESTAMP_LOG_DEBUG(
        "Sending packet with sequence number "
        << sequenceNumber.to_string()
    );

    m_socket->Send(packet);
}

void GbnSender::OnRecv(Ptr<Socket> sock) {
    if(!m_running) return;
    assert(m_socket->GetRxAvailable());

    Ptr<Packet> packet = sock->Recv();
    GbnPacket gbnPacket;
    try {
        gbnPacket = GbnPacket::Parse(packet);

    } catch(std::runtime_error& e) {
        std::string message = "Dropping packet due to parsing error: ";
        NS_TIMESTAMP_LOG_INFO(message + e.what());
        return;
    }

    bool outOfRangeSequenceNumber = !gbnPacket.sequenceNumber.inWindow(
        m_baseSequenceNumber,
        m_nextSequenceNumber
    );
    if(outOfRangeSequenceNumber) {
        NS_TIMESTAMP_LOG_DEBUG(
            "Dropping packet with sequence number outside of window ("
            << gbnPacket.sequenceNumber.to_string()
            << " is not in "
            << m_baseSequenceNumber.to_string()
            << " to "
            << m_nextSequenceNumber.to_string()
            << " mod "
            << std::to_string(GbnConfig::maxSequenceNumber)
            << ")"
        );
        return;
    }

    NS_TIMESTAMP_LOG_DEBUG(
        "Received ack with sequence number "
        << gbnPacket.sequenceNumber.to_string()
        << ". Window: ["
        << m_baseSequenceNumber.to_string()
        << " -> "
        << (m_baseSequenceNumber + GbnSenderConfig::windowSize - 1).to_string()
        << "] (next: "
        << m_nextSequenceNumber.to_string()
        << ")";
    );

    m_baseSequenceNumber = gbnPacket.sequenceNumber + 1;

    ::SequenceNumber txNext = m_nextSequenceNumber + 1;
    //Send now available due to window shift
    uint32_t packetsToSend = txNext.distance(m_baseSequenceNumber + GbnSenderConfig::windowSize);
    //Respect application packet limit.
    packetsToSend = std::min(packetsToSend, m_nPackets - m_nPacketsSent);

    SendPackets(txNext, packetsToSend);
    m_nPacketsSent += packetsToSend;
    assert(m_nPacketsSent < m_nPackets);

    NS_TIMESTAMP_LOG_DEBUG(
        "Window is now ["
        << m_baseSequenceNumber.to_string()
        << " -> "
        << (m_baseSequenceNumber + GbnSenderConfig::windowSize - 1).to_string()
        << "] (next: "
        << m_nextSequenceNumber.to_string()
        << ")"
    );
}
