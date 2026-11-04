#include "./receiver.h"
#include "./packet.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include "ns3/applications-module.h"

using namespace ns3;

GbnReceiver::GbnReceiver()
    : m_socket(nullptr),
      m_running(false),
      m_sendEvent()
{
}

GbnReceiver::~GbnReceiver()
{
    m_socket = nullptr;
}

/* static */
TypeId
GbnReceiver::GetTypeId()
{
    static TypeId tid = TypeId("GbnReceiver")
                            .SetParent<Application>()
                            .SetGroupName("GbnReceiver")
                            .AddConstructor<GbnReceiver>();
    return tid;
}

void
GbnReceiver::Setup(Ipv4Address bindAddress)
{
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_bindAddress = bindAddress;
}

void
GbnReceiver::OnRecv(Ptr<Socket> socket) {
    Ptr<Packet> rawPacket = socket->Recv();
    auto packet = ::GbnPacket::Parse(rawPacket);
    std::cout
        << "Received message with sequence number "
        << packet.sequenceNumber
        << std::endl;

    packet.payload.reset();
    std::cout
        << "ACKing"
        << std::endl;

    socket->Send(packet.Serialize());
}

void
GbnReceiver::StartApplication()
{
    m_running = true;
    int status = m_socket->Bind(
        InetSocketAddress(m_bindAddress, ::GbnConfig::port)
    );
    if(status == -1) {
        NS_FATAL_ERROR("Failed to bind socket");
    }

    m_socket->SetRecvCallback(
        MakeCallback(&GbnReceiver::OnRecv, this)
    );
}

void
GbnReceiver::StopApplication()
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

void
GbnReceiver::SendPacket()
{
    Ptr<Packet> packet = Create<Packet>(0);
    m_socket->Send(packet);
}
