#include "./sender.h"
#include "./config.h"
#include "./packet.h"

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

    SendPackets(m_baseSequenceNumber);
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



void GbnSender::SendPackets(::SequenceNumber from) {

    uint32_t count = 0;
    ::SequenceNumber curr = from;
    ::SequenceNumber end = m_baseSequenceNumber + GbnSenderConfig::windowSize;
    std::string message =
        + " sending packets from "
        + curr.to_string()
        + " to "
        + end.to_string();
    NS_LOG_DEBUG(message);

    assert(curr != end);
    while(curr != end) {
        ScheduleTx(curr);
        ++curr;
        ++count;
    }
    m_nextSequenceNumber = curr - 1;
    m_metrics.totalPacketsTransmitted += count;
}

void
GbnSender::SendPacket(const ::SequenceNumber sequenceNumber)
{
    auto packet = GbnPacket(
            sequenceNumber,
            GbnSenderConfig::getPayloadContent()
    ).Serialize();

    std::string message =
        "Sending packet with sequence number "
        + sequenceNumber.to_string();
    NS_LOG_DEBUG(message);

    m_socket->Send(packet);
}

void GbnSender::OnRecv(Ptr<Socket> sock) {
    assert(m_socket->GetRxAvailable());

    Ptr<Packet> packet = sock->Recv();
    GbnPacket gbnPacket;
    try {
        gbnPacket = GbnPacket::Parse(packet);

    } catch(std::runtime_error& e) {
        std::string message = "Dropping packet due to parsing error: ";
        NS_LOG_INFO(message + e.what());
        return;
    }

    bool outOfRangeSequenceNumber = !gbnPacket.sequenceNumber.inWindow(
        m_baseSequenceNumber,
        m_nextSequenceNumber
    );
    if(outOfRangeSequenceNumber) {
        std::string message =
                "Dropping packet with sequence number outside of window ("
                + gbnPacket.sequenceNumber.to_string()
                + " is not in "
                + m_baseSequenceNumber.to_string()
                + " to "
                + m_nextSequenceNumber.to_string()
                + " mod "
                + std::to_string(GbnConfig::maxSequenceNumber)
                + ")";
        NS_LOG_INFO(message);
        return;
    }

    std::string message =
        "Received ack with sequence number "
        + gbnPacket.sequenceNumber.to_string()
        + ". Window: ["
        + m_baseSequenceNumber.to_string()
        + " -> "
        + (m_baseSequenceNumber + GbnSenderConfig::windowSize - 1).to_string()
        + "] (next: "
        + m_nextSequenceNumber.to_string()
        + ")";

    NS_LOG_DEBUG(message);

    m_baseSequenceNumber = gbnPacket.sequenceNumber + 1;
    SendPackets(m_nextSequenceNumber + 1);

    message =
        "Window is now ["
        + m_baseSequenceNumber.to_string()
        + " -> "
        + (m_baseSequenceNumber + GbnSenderConfig::windowSize - 1).to_string()
        + "] (next: "
        + m_nextSequenceNumber.to_string()
        + ")";

    NS_LOG_DEBUG(message);
}

void
GbnSender::ScheduleTx(const ::SequenceNumber sequenceNumber)
{
    if (m_running)
    {
        m_sendEvent = Simulator::Schedule(Seconds(0), &GbnSender::SendPacket, this, sequenceNumber);
    }
}
