#pragma once

#include <stdint.h>
#include <cstddef>
#include <string>
#include <array>
#include <memory.h>


class GbnConfig {
    protected:
        static const int studentId = 7966258;

    public:
        static const uint32_t port = 30;
        static const std::size_t sequenceNumberSize = 4;
        static constexpr uint32_t initialSequenceNumber = (GbnConfig::studentId / 10) % 10; //Second right-most digit of student ID;
        static const uint32_t maxSequenceNumber = 15;
        static const std::size_t payloadSize = 1000; //(application data) + 4 bytes (sequence number);
};

typedef std::array<uint8_t, GbnConfig::payloadSize - GbnConfig::sequenceNumberSize> PacketPayload;
class GbnSenderConfig : GbnConfig {
    private:
        static constexpr std::string studentName = "Logan";

    public:
        static constexpr std::string retransmissionTimeout = "100ms";
        static constexpr int windowSize = 5 + (GbnSenderConfig::studentId % 5);
        static const int numPackets;

        static constexpr PacketPayload getPayloadContent() {
            const std::size_t nameLength = GbnSenderConfig::studentName.length();
            const std::size_t payloadContentSize = GbnConfig::payloadSize - GbnConfig::sequenceNumberSize;
            static_assert(nameLength <= payloadContentSize);

            PacketPayload array; array.fill('0');
            memcpy(array.data(), GbnSenderConfig::studentName.data(), nameLength);
            return array;
        }
};
// static_assert(GbnSenderConfig::getPayloadContent().size() + GbnConfig::sequenceNumberSize == GbnConfig::payloadSize);
static_assert(GbnSenderConfig::windowSize * 2 <= GbnConfig::maxSequenceNumber + 1);
