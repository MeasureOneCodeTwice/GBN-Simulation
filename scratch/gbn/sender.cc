#include "./sender.h"
#include "./config.h"
#include "./packet.h"
#include "./utils.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

#include <memory.h>
#include <assert.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GbnSender");

//based on `examples/tutorial/tutorial-app.cc`
GbnSender::GbnSender()
    : m_socket(nullptr),
      m_peer(),
      m_nPackets(0),
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0),
      m_baseSequenceNumber(GbnConfig::initialSequenceNumber),
      m_nextSequenceNumber(GbnConfig::initialSequenceNumber)
{
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
GbnSender::Setup(Ipv4Address address,
                 uint32_t nPackets,
                 DataRate dataRate
)
{
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_peer = InetSocketAddress(address, ::GbnConfig::port);
    m_nPackets = nPackets;
    m_dataRate = dataRate;
}

void
GbnSender::StartApplication()
{
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind();
    m_socket->Connect(m_peer);

    m_socket->SetRecvCallback(
        MakeCallback(&GbnSender::OnRecv, this)
    );

    SendPackets();
}

void
GbnSender::StopApplication()
{
    m_running = false;

    if (m_sendEvent.IsPending())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

int GbnSender::SendPackets() {

    uint32_t packetsToSend = GbnSenderConfig::windowSize - (m_nextSequenceNumber - m_baseSequenceNumber);
    assert(packetsToSend > 0);
    for(uint32_t i = 0; i < packetsToSend; i++) {
        ScheduleTx();
    }

    return packetsToSend;
}

void
GbnSender::SendPacket()
{
    auto packet = GbnPacket(
            m_nextSequenceNumber,
            GbnSenderConfig::getPayloadContent()
    ).Serialize();

    std::string message =
        "Sending packet with sequence number "
        + std::to_string(m_nextSequenceNumber);
    NS_LOG_INFO(message);

    m_socket->Send(packet);
    m_metrics.totalPacketsTransmitted++;

    incrementSequenceNumber(m_nextSequenceNumber);
}

void GbnSender::OnRecv(Ptr<Socket> sock) {
    assert(m_socket->GetRxAvailable());

    Ptr<Packet> packet = sock->RecvFrom(this->m_peer);
    GbnPacket gbnPacket;
    try {
        gbnPacket = GbnPacket::Parse(packet);

    } catch(std::runtime_error& e) {
        std::string message = "Dropping packet due to parsing error: ";
        NS_LOG_ERROR(message + e.what());
        return;
    }

    std::cout << "Ack sequence number " << gbnPacket.sequenceNumber << " recvd" << std::endl;
}

void
GbnSender::ScheduleTx()
{
    if (m_running)
    {
        Time tNext(Seconds(GbnConfig::payloadSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &GbnSender::SendPacket, this);
    }
}
