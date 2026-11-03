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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GbnNetwork");

void setPacketErrorRates(NetDeviceContainer);

int
main(int argc, char* argv[])
{
    ::SimulationConfig::applyCliArguments(argc, argv);
    std::cout << ::SimulationConfig::getDataPacketLossProbability() << std::endl;


    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    //Sender: nodes[0], Receiver: nodes[1]
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(::SimulationConfig::linkBandwidth ));
    pointToPoint.SetChannelAttribute("Delay", StringValue(::SimulationConfig::propagationDelay ));


    NetDeviceContainer devices = pointToPoint.Install(nodes);
    setPacketErrorRates(devices);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(10));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2));
    clientApps.Stop(Seconds(10));

    Simulator::Stop(::SimulationConfig::maximumSimulationDuration);
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

void setPacketErrorRates(NetDeviceContainer devices) {

    Ptr<RateErrorModel> dataErrorModel = CreateObject<RateErrorModel>();
    dataErrorModel->SetUnit(RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
    dataErrorModel->SetRate(::SimulationConfig::getDataPacketLossProbability());
    devices.Get(0)->SetAttribute ("ReceiveErrorModel", PointerValue(dataErrorModel));

    Ptr<RateErrorModel> ackErrorModel = CreateObject<RateErrorModel>();
    ackErrorModel->SetUnit(RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
    ackErrorModel->SetRate(::SimulationConfig::getAckPacketLossProbability());
    devices.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue(ackErrorModel));
}
