#pragma once

#include <string>
#include "ns3/core-module.h"

class GbnConfig {
    private:
        static const int studentId = 7966258;
        static constexpr std::string studentName = "Logan";

    public:
        static constexpr int initialSequenceNumber = (GbnConfig::studentId / 10) % 10; //Second right-most digit of student ID;
        static constexpr int windowSize = 5 + (studentId % 5);
        static const int retransmissionTimeoutMs = 100;

        static const std::size_t sequenceNumberSize = 4;
        static const std::size_t payloadSize = 1000; //(application data) + 4 bytes (sequence number);

        static constexpr std::string getPayloadContent() {
            const std::size_t nameLength = GbnConfig::studentName.length();
            const std::size_t payloadContentSize = GbnConfig::payloadSize - GbnConfig::sequenceNumberSize;
            static_assert(nameLength <= payloadContentSize);

            const std::size_t zeroesToPad = payloadContentSize - nameLength;
            std::string payloadContent(zeroesToPad, '0');
            payloadContent.insert(0, GbnConfig::studentName);
            return payloadContent;
        }
};
static_assert(GbnConfig::getPayloadContent().length() + GbnConfig::sequenceNumberSize == GbnConfig::payloadSize);


class SimulationConfig {

    public:
        static const ns3::Time maximumSimulationDuration;

        static constexpr std::string linkBandwidth = "10Mbps";
        static constexpr std::string propagationDelay = "20ms";
        static const int numberOfDataPackets = 100;

    // CLI configurable
    private:
        static float DataPacketLossProbability;
        static constexpr float DefaultDataPacketLossProbability = 0.1;

        static float AckPacketLossProbability;
        static constexpr float DefaultAckPacketLossProbability = 0.1;

    public:

        static float getDataPacketLossProbability() {
            return SimulationConfig::DataPacketLossProbability;
        }

        static float getAckPacketLossProbability() {
            return SimulationConfig::AckPacketLossProbability;
        }

        static void applyCliArguments(int argc, char* argv[]) {
            ns3::CommandLine cmd;
            cmd.AddValue("DataPacketLossProbability", "The probability of data packet loss", SimulationConfig::DataPacketLossProbability);
            cmd.AddValue("AckPacketLossProbability", "The probability of ack packet loss", SimulationConfig::AckPacketLossProbability);
            cmd.Parse(argc, argv);
        }



        // int byteOrder = Network byte order;
        // int randomnessControl = RngSeedManager;
        // int performanceMetrics = Delivered packets, transmissions, retransmissions, timeouts, completion time, goodput;
};

float SimulationConfig::DataPacketLossProbability = SimulationConfig::DefaultDataPacketLossProbability;
float SimulationConfig::AckPacketLossProbability = SimulationConfig::DefaultAckPacketLossProbability;
const ns3::Time SimulationConfig::maximumSimulationDuration("120s");

