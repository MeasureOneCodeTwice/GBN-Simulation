//Based on the example `examples/tutorial/first.cc`

/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "./config.h"
#include "./gbn/sender.h"
#include "./gbn/receiver.h"

#include <filesystem>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GbnNetwork");


void setPacketErrorRates(NetDeviceContainer);
void printMetrics(GbnSender::Metrics);

int
main(int argc, char* argv[])
{
    ::SimulationConfig::applyCliArguments(argc, argv);

    Time::SetResolution(Time::NS);
    ns3::RngSeedManager::SetSeed(SimulationConfig::randomSeed);

    NodeContainer nodes;
    //Sender: nodes[0], Receiver: nodes[1]
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(::SimulationConfig::linkBandwidth ));
    pointToPoint.SetChannelAttribute("Delay", StringValue(::SimulationConfig::propagationDelay ));


    NetDeviceContainer devices = pointToPoint.Install(nodes);
    //enable PCAP
    std::filesystem::create_directory(SimulationConfig::pcapOutputDirectory);
    pointToPoint.EnablePcapAll(SimulationConfig::pcapOutputDirectory + SimulationConfig::pcapOutputBaseFileName);
    setPacketErrorRates(devices);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);


    LogComponentEnable("GbnReceiver", LOG_LEVEL_INFO);
    LogComponentEnable("GbnSender", LOG_LEVEL_INFO);

    Ptr<GbnSender> sender = CreateObject<GbnSender>();
    nodes.Get(0)->AddApplication(sender);
    sender->Setup(
        interfaces.GetAddress(1),
        ::SimulationConfig::numberOfDataPackets
    );

    Ptr<GbnReceiver> receiver = CreateObject<GbnReceiver>();
    nodes.Get(1)->AddApplication(receiver);
    receiver->Setup(interfaces.GetAddress(1));

    Simulator::Stop(::SimulationConfig::maximumSimulationDuration);
    Simulator::Run();
    Simulator::Destroy();

    printMetrics(sender->getMetrics());
    return 0;
}

void setPacketErrorRates(NetDeviceContainer devices) {

    Ptr<RateErrorModel> dataErrorModel = CreateObject<RateErrorModel>();
    dataErrorModel->SetUnit(RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
    dataErrorModel->SetRate(::SimulationConfig::getDataPacketLossProbability());
    //set recipient error rate
    devices.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue(dataErrorModel));

    Ptr<RateErrorModel> ackErrorModel = CreateObject<RateErrorModel>();
    ackErrorModel->SetUnit(RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
    ackErrorModel->SetRate(::SimulationConfig::getAckPacketLossProbability());
    //set sender recv error rate
    devices.Get(0)->SetAttribute ("ReceiveErrorModel", PointerValue(ackErrorModel));
}

void printMetrics(GbnSender::Metrics metrics) {
    auto goodputPercentage = ((float)metrics.goodput.GetBitRate() / DataRate(SimulationConfig::linkBandwidth).GetBitRate()) * 100;
    std::cout << std::endl;
    std::cout << "----------- METRICS -----------" << std::endl;
    std::cout << " Total Packets Transmitted: "    << metrics.totalPacketsTransmitted << std::endl;
    std::cout << " Packet Retransmissions: "       << metrics.packetRetransmissions   << std::endl;
    std::cout << " Timeouts: "                     << metrics.timeouts                << std::endl;
    std::cout << " End time: "                     << metrics.endTime.As(Time::S)     << std::endl;
    std::cout << " Goodput: "                      << metrics.goodput                 << std::endl;
    std::cout << " Goodput Percentage: "           << goodputPercentage               << std::endl;
    std::cout << "-------------------------------" << std::endl;
}
