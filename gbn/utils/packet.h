#pragma once

#include "ns3/applications-module.h"
#include "../config.h"
#include "./sequence-number.h"

using namespace ns3;


struct GbnPacket {
    SequenceNumber sequenceNumber;
    std::optional<PacketPayload> payload;

    GbnPacket();
    GbnPacket(SequenceNumber);
    GbnPacket(SequenceNumber, PacketPayload &&payload);

    Ptr<Packet> Serialize();
    //damages the given packet
    static GbnPacket Parse(Ptr<Packet>);
};
