#include "./receiver.h"
#include "./utils/packet.h"
#include "./utils/logging.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GbnReceiver");

GbnReceiver::GbnReceiver()
    : m_socket(nullptr),
      m_running(false),
      m_sequenceNumber(GbnConfig::initialSequenceNumber)
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
    Address senderAddr;
    Ptr<Packet> rawPacket = socket->RecvFrom(senderAddr);

    GbnPacket packet;
    try {
        packet = GbnPacket::Parse(rawPacket);
    } catch(std::runtime_error& e) {
        std::string message = "Dropping packet due to parsing error: ";
        NS_TIMESTAMP_LOG_INFO(message + e.what());
        return;
    }

    if(packet.sequenceNumber == m_sequenceNumber) {
        std::string message =
            "Successfuly received data packet with sequence number "
            + packet.sequenceNumber.to_string();
        NS_TIMESTAMP_LOG_INFO(message);

        ++m_sequenceNumber;
    } else {
        std::string message =
            "Received packet with out of order sequence number "
            + packet.sequenceNumber.to_string()
            + " expected sequence number "
            + m_sequenceNumber.to_string();

        NS_TIMESTAMP_LOG_DEBUG(message);
    }

    //send ACK
    //clear payload
    packet.payload.reset();
    packet.sequenceNumber = m_sequenceNumber - 1;
    socket->SendTo(packet.Serialize(), 0, senderAddr);
}

void
GbnReceiver::StartApplication()
{
    m_running = true;
    int status = m_socket->Bind(
        InetSocketAddress(m_bindAddress, GbnConfig::port)
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
