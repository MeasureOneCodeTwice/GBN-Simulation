#include "./sender.h"
#include "./config.h"
#include "./utils/packet.h"
#include "./utils/logging.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

#include <memory.h>
#include <algorithm>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GbnSender");

void GbnSender::onTimeout(::SequenceNumber sequenceNumber) {
    NS_ASSERT(sequenceNumber == m_baseSequenceNumber);
    NS_ASSERT(m_nextSequenceNumber == getWindowEnd() + 1);
    NS_TIMESTAMP_LOG_INFO(
            "Packet with sequence number "
            << sequenceNumber.to_string()
            << " timed out"
    );
    m_timeoutQueue.clear();
    //only retransmit packets we've sent
    m_metrics.packetRetransmissions += SendPackets(m_baseSequenceNumber, m_nextSequenceNumber - 1);
    ++m_metrics.timeouts;
}

//based on `examples/tutorial/tutorial-app.cc`
GbnSender::GbnSender()
    : m_socket(nullptr),
      m_peer(),
      m_nPackets(0),
      m_nPacketsAcked(0),
      m_running(false),
      m_baseSequenceNumber(GbnConfig::initialSequenceNumber),
      m_nextSequenceNumber(GbnConfig::initialSequenceNumber),
      m_timeoutQueue(
              [this](::SequenceNumber num){ this->onTimeout(num); },
              GbnSenderConfig::retransmissionTimeout
      )
{
    NS_ASSERT(!::SequenceNumber(8).inWindow(9, 0));
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

    SendPackets(m_baseSequenceNumber, getWindowEnd());
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



int GbnSender::SendPackets(::SequenceNumber from, ::SequenceNumber to) {

    NS_TIMESTAMP_LOG_DEBUG(
        "Sending packets from "
        << from.to_string()
        << " to "
        << to.to_string();
    );

    ::SequenceNumber curr = from;
    while(curr != to + 1) {
        m_timeoutQueue.enqueue(curr);
        NS_TIMESTAMP_LOG_DEBUG(
                "Inserted packet with sequence number "
                << curr.to_string()
                << " into timeout queue"
        );
        SendPacket(curr);
        ++curr;
    }
    m_nextSequenceNumber = to + 1;
    uint32_t packetsSent = from.distance(to) + 1;
    m_metrics.totalPacketsTransmitted += packetsSent;
    return packetsSent;
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
    NS_ASSERT(m_socket->GetRxAvailable());

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
        m_nextSequenceNumber - 1
    );
    if(outOfRangeSequenceNumber) {
        NS_TIMESTAMP_LOG_DEBUG(
            "Dropping packet with sequence number outside of window ("
            << gbnPacket.sequenceNumber.to_string()
            << " is not in "
            << m_baseSequenceNumber.to_string()
            << " to "
            << (m_nextSequenceNumber - 1).to_string()
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
        << getWindowEnd().to_string()
        << "] (next: "
        << (m_nextSequenceNumber - 1).to_string()
        << ")";
    );

    ::SequenceNumber nextBase = gbnPacket.sequenceNumber + 1;
    if(nextBase == getWindowEnd() + 1) {
        nextBase = getWindowEnd();
    }
    m_nPacketsAcked += m_baseSequenceNumber.distance(nextBase);

    uint32_t numPacketsToDequeue = m_baseSequenceNumber.distance(gbnPacket.sequenceNumber + 1);
    for(uint32_t i = 0; i < numPacketsToDequeue; i++) {
        ::SequenceNumber removed = m_timeoutQueue.dequeue();
        NS_ASSERT(removed == m_baseSequenceNumber + i);
    }
    m_baseSequenceNumber = nextBase;


    NS_TIMESTAMP_LOG_DEBUG(
        "Window is now ["
        << m_baseSequenceNumber.to_string()
        << " -> "
        << getWindowEnd().to_string()
        << "] (next: "
        << (m_nextSequenceNumber - 1).to_string()
        << ")"
    );

    //Send packets now available due to window shift
    SendPackets(m_nextSequenceNumber, getWindowEnd());

    NS_TIMESTAMP_LOG_INFO("Sent " << m_nPacketsAcked << " of " << m_nPackets << " packets");
    if(allPacketsReceived()) {
        NS_LOG_INFO("Done.");
        return;
    }

    NS_ASSERT(m_nPacketsAcked <= m_nPackets);
}

::SequenceNumber GbnSender::getWindowEnd() const {
    NS_ASSERT(m_nPackets >= m_nPacketsAcked);

    uint32_t packetsLeft = m_nPackets - m_nPacketsAcked;
    uint32_t min = std::min(packetsLeft, GbnSenderConfig::windowSize - 1);
    return m_baseSequenceNumber + min;
}

bool GbnSender::allPacketsReceived() const {
    return m_nPackets == m_nPacketsAcked;
}
