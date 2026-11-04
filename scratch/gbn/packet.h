#include "ns3/applications-module.h"
#include "./config.h"

using namespace ns3;


struct GbnPacket {
    uint32_t sequenceNumber;
    std::optional<PacketPayload> payload;

    GbnPacket();
    GbnPacket(uint32_t sequenceNumber);
    GbnPacket(uint32_t sequenceNumber, PacketPayload &&payload);

    Ptr<Packet> Serialize();
    //damages the given packet
    static GbnPacket Parse(Ptr<Packet>);
};
