#include "./packet.h"
#include <memory.h>
#include <arpa/inet.h>

GbnPacket GbnPacket::Parse(Ptr<Packet> rawPacket) {

    //check total packet length
    auto packetSize = rawPacket->GetSize();
    if(packetSize != GbnConfig::payloadSize && packetSize != GbnConfig::sequenceNumberSize) {
        throw std::runtime_error("Received packet that does not match payload size");
    }

    const auto sequenceNumberSize = GbnConfig::sequenceNumberSize;
    static_assert(sizeof(uint32_t) == sequenceNumberSize);

    //extract sequence number & check if its in range
    uint32_t sequenceNumber;
    rawPacket->CopyData((uint8_t*)&sequenceNumber, sequenceNumberSize);
    sequenceNumber = ntohl(sequenceNumber);

    if(sequenceNumber > GbnConfig::maxSequenceNumber) {
        std::string message =
        "Received with sequence number greater than maximum allowed sequence number (got "
        + std::to_string(sequenceNumber)
        + " expected at most "
        + std::to_string(GbnConfig::maxSequenceNumber)
        + ")";
        throw std::runtime_error(message);
    }
    rawPacket->RemoveAtStart(sequenceNumberSize);

    //copy payload if it exists
    GbnPacket packet(sequenceNumber);
    if(packetSize != 0) {
        packet.payload.emplace();
        rawPacket->CopyData(packet.payload->data(), packet.payload->size());
    }

    return packet;
}

Ptr<Packet> GbnPacket::Serialize() {
    uint8_t buffer[GbnConfig::payloadSize];

    uint32_t networkOrderSequenceNumber = htonl(this->sequenceNumber.getValue());
    memcpy(buffer, &networkOrderSequenceNumber, GbnConfig::sequenceNumberSize);
    std::size_t bufferContentLength = GbnConfig::sequenceNumberSize;
    if(this->payload) {
        size_t payloadSize = this->payload->size();
        memcpy(buffer + bufferContentLength, this->payload->data(), payloadSize);
        bufferContentLength += payloadSize;
    }

    return Create<Packet>(buffer, bufferContentLength);
}

GbnPacket::GbnPacket()
{
}

GbnPacket::GbnPacket(SequenceNumber sequenceNumber)
    : sequenceNumber(sequenceNumber)
{
}

GbnPacket::GbnPacket(SequenceNumber sequenceNumber, PacketPayload &&payload)
    : sequenceNumber(sequenceNumber),
      payload(payload)
{
}

