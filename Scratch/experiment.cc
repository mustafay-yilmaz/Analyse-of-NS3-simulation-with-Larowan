#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/periodic-sender.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include <algorithm>
#include <ctime>

#include "ns3/building-allocator.h"
#include "ns3/building-penetration-loss.h"
#include "ns3/buildings-helper.h"
#include "ns3/callback.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/double.h"
#include "ns3/forwarder-helper.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/network-server-helper.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-phy.h"

#include <iostream>
//------------------------------------------------------

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");


auto packetsSent = 0;

auto packetsReceived = 0;

auto pdr = 0.0;

double averageDelay = 0.0;



// İlgili bilgileri içeren yapı
struct PacketInfo {
    ns3::Ptr<const ns3::Packet> packet;
    ns3::Time packetSentTime;
    ns3::Time packetReceivedTime;
};

std::vector<PacketInfo> receivedPackets;



/**
 * Record the beginning of a transmission by an end device.
 *
 * \param packet A pointer to the packet sent.
 * \param senderNodeId Node id of the sender end device.
 */
void OnTransmissionCallback(ns3::Ptr<const ns3::Packet> packet, uint32_t senderNodeId)
{
    PacketInfo info;
    info.packet = packet;
    info.packetSentTime = ns3::Simulator::Now();
    receivedPackets.push_back(info);
    packetsSent++;
}

/**
 * Record the correct reception of a packet by a gateway.
 *
 * \param packet A pointer to the packet received.
 * \param receiverNodeId Node id of the receiver gateway.
 */
void OnPacketReceptionCallback(ns3::Ptr<const ns3::Packet> packet, uint32_t receiverNodeId)
{
    for (auto& info : receivedPackets) {
        if (info.packet == packet) {
            info.packetReceivedTime = ns3::Simulator::Now();
            packetsReceived++;
            break;
        }
    }
}


void PrintPacketStatistics()
{
    // Calculate Packet Delivery Ratio (PDR)
    float pdr = (static_cast<float>(packetsReceived) / packetsSent) * 100;

    // Print the statistics
    std::cout << "\n\nGönderilen Toplam Paket Sayısı: " << packetsSent << std::endl;
    std::cout << "Ulaşan Toplam Paket Sayısı: " << packetsReceived << std::endl;
    std::cout << "PDR: " << pdr << "%" << std::endl;
}

void PrintPacketsDelay()
{
    int i = 0;
    double totalDelay = 0.0;
    std::cout << "\n\nPackets Delay:" << std::endl;
    for (const auto& packetInfo  : receivedPackets)
    {
            auto delay = (packetInfo.packetReceivedTime-packetInfo.packetSentTime).GetSeconds();
            // Gönderim zamanını ve alınma zamanını yazdır            
             std::cout << "Packet " << ++i <<" Delay Time: " <<  delay << std::endl;

            totalDelay+=delay;       
    }
    averageDelay = totalDelay/packetsReceived;

    std::cout << "\nAverage Delay: " <<  averageDelay << std::endl;
}

void RemoveUnreceivedPackets()
{
    auto it = std::remove_if(receivedPackets.begin(), receivedPackets.end(), [](const PacketInfo& info) {
        return info.packetReceivedTime == ns3::Seconds(0);
    });

    receivedPackets.erase(it, receivedPackets.end());
}


int
main(int argc, char* argv[])
{
    uint32_t nDevices;
    uint32_t Period;

    std::string OutputFolder;

     CommandLine cmd;
    cmd.AddValue("nDevices", "Number of devices in the simulation", nDevices);
    cmd.AddValue("OutputFolder", "Number of devices in the simulation", OutputFolder);
    cmd.AddValue("Period", "Period Time", Period);

    cmd.Parse(argc, argv);
   
    /************************
     *  Create the channel  *
     ************************/

    NS_LOG_INFO("Creating the channel...");

    // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    /************************
     *  Create the helpers  *
     ************************/

    NS_LOG_INFO("Setting up helpers...");

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(4000, 4000, 15));
    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();

    // Create the LoraHelper
    LoraHelper helper = LoraHelper();

    NetworkServerHelper nsHelper = NetworkServerHelper();

    // Create the ForwarderHelper
    ForwarderHelper forHelper = ForwarderHelper();


    /************************
     *  Create End Devices  *
     ************************/

    NS_LOG_INFO("Creating the end device...");
    std::string traceFile="/home/ali/Masaüstü/sumoDemo/ns2mobility-az.tcl";
    int nodeNum = nDevices;

    Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
    NodeContainer endDevices;
    endDevices.Create (nodeNum);
    ns2.Install (); // configure movements for each node, while reading trace file

    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, endDevices);
    
    /*********************
     *  Create Gateways  *
     *********************/

    NS_LOG_INFO("Creating the gateway...");
    NodeContainer gateways;
    gateways.Create(1);

    mobility.Install(gateways);

    // Create a netdevice for each gateway
    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);


    //// Install applications in end devices
    int appPeriodSeconds = Period; // One packet every 20 minute
    PeriodicSenderHelper appHelper = PeriodicSenderHelper();
    appHelper.SetPeriod(Seconds(appPeriodSeconds));
    ApplicationContainer appContainer = appHelper.Install(endDevices);


    // Install trace sources
    for (auto node = gateways.Begin(); node != gateways.End(); node++)
    {
        (*node)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy()->TraceConnectWithoutContext(
            "ReceivedPacket",
            MakeCallback(OnPacketReceptionCallback));
    }

    // Install trace sources
    for (auto node = endDevices.Begin(); node != endDevices.End(); node++)
    {
        (*node)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy()->TraceConnectWithoutContext(
            "StartSending",
            MakeCallback(OnTransmissionCallback));
    }

    LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);


    /****************
     *  Simulation  *
     ****************/

    Simulator::Stop(Seconds(8000));

    Simulator::Run();
    
    /////////////////////////////
    // Print results to stdout //
    /////////////////////////////
    NS_LOG_INFO("Computing performance metrics...");
  
        RemoveUnreceivedPackets();
        PrintPacketsDelay();
        PrintPacketStatistics();

    
    Simulator::Destroy();

    return 0;
}
