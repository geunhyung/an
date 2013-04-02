/*  Advanced Networking Assignment: Lab1
 *  lab1-task1-template.cc
 *
 *  Michiel Appelman  
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

// Define your NS_LOG identifier here
NS_LOG_COMPONENT_DEFINE("lab1-task1-appelman");

int
main(int argc, char *argv[]) {
    // Global configuration
    uint32_t MAX_RWIN = 16384;
    std::string DELAY = "64ms";
    
    CommandLine cmd;
    cmd.AddValue ("delay", "Time of delay on the linke: <num>ms", DELAY);
    cmd.Parse (argc, argv);

    // Simulation parameters
    Config::SetDefault ("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(MAX_RWIN));

    NS_LOG_INFO("Create nodes.");
    // Create network toplogy using NodeContainer class
    NodeContainer nodes;
    nodes.Create (2);

    // Create channels with specified data rate and delay, without IP addresses first
    NS_LOG_INFO("Create channel.");
    
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("3.2Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue (DELAY));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    // Install Internet Stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Add IP addresses
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    // Create TCP or UDP applications installed on nodes.
    NS_LOG_INFO("Create TCP Server.");
    uint16_t port = 50000;
    Address sinkLocalAddress(InetSocketAddress (Ipv4Address::GetAny (), port));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install (nodes.Get (0));
    sinkApp.Start (Seconds (1.0));
    sinkApp.Stop (Seconds (60.0));
    
    NS_LOG_INFO("Create TCP Client.");
    
    ApplicationContainer clientApp;
    OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
    Ipv4Address serverAddress = interfaces.GetAddress (0);
    AddressValue remoteAddress (InetSocketAddress (serverAddress, port));
    onOffHelper.SetAttribute ("Remote", remoteAddress);
    clientApp.Add (onOffHelper.Install (nodes.Get (1)));
    
    clientApp.Start (Seconds (1.0));
    clientApp.Stop (Seconds (50.0));

    
    // Enable ascii tracing or pcap tracing
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("tcp-task1.tr"));
    pointToPoint.EnablePcapAll("tcp-task1");

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    // Set simulation timeout and run.
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(60.0));
    Simulator::Run();

    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
            std::cout << "Flow " << i->first << ", delay: " << DELAY << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 50.0 / 1024 / 1024  << " Mbps\n";
    }

    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}

