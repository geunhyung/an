/*  Advanced Networking Assignment: Lab1
 *  lab1-task2-template.cc
 *
 *  Created on: Feb 25, 2013
 *      Author: canhnt <t.c.ngo@uva.nl>
 *
 *  ns-3 simulation source code template
 *  Copyright (C) 2013 Canh Ngo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

class MyApp: public Application {
public:

    MyApp();
    virtual ~MyApp();

    void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize,
            DataRate dataRate);
    void ChangeRate(DataRate newrate);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void ScheduleTx(void);
    void SendPacket(void);

    Ptr<Socket> m_socket;
    Address m_peer;
    uint32_t m_packetSize;
//  uint32_t m_nPackets;
    DataRate m_dataRate;
    EventId m_sendEvent;
    bool m_running;
    uint32_t m_packetsSent;
};

MyApp::MyApp() :
        m_socket(0), m_peer(), m_packetSize(0), m_dataRate(0), m_sendEvent(), m_running(
                false), m_packetsSent(0) {
}

MyApp::~MyApp() {
    m_socket = 0;
}

void MyApp::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize,
        DataRate dataRate) {
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_dataRate = dataRate;
}

void MyApp::StartApplication(void) {
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind();
    m_socket->Connect(m_peer);
    SendPacket();
}

void MyApp::StopApplication(void) {
    m_running = false;

    if (m_sendEvent.IsRunning()) {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket) {
        m_socket->Close();
    }
}

void MyApp::SendPacket(void) {
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

//  if (++m_packetsSent < m_nPackets)
//    {
    ScheduleTx();
//    }
}

void MyApp::ScheduleTx(void) {
    if (m_running) {
        Time tNext(
                Seconds(
                        m_packetSize * 8
                                / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &MyApp::SendPacket, this);
    }
}

void MyApp::ChangeRate(DataRate newrate) {
    m_dataRate = newrate;
    return;
}

static void CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
        std::cout << Simulator::Now ().GetSeconds () << "," << newCwnd << "\n";
}

// Define your NS_LOG identifier here
NS_LOG_COMPONENT_DEFINE("lab1-task2b-appelman");

int main(int argc, char *argv[]) {
    // Global configuration
    std::string DELAY = "16ms";

    CommandLine cmd;
    cmd.AddValue ("delay", "Time of delay on the link: <num>ms", DELAY);
    cmd.Parse (argc, argv);

    // Simulation parameters
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
    Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (4));

    // Create channels with specified data rate and delay, without IP addresses first
    NS_LOG_INFO("Create channel.");

    PointToPointHelper pointToPointBB;
    pointToPointBB.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPointBB.SetChannelAttribute ("Delay", StringValue (DELAY));

    PointToPointHelper pointToPointLAN;
    pointToPointLAN.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    pointToPointLAN.SetChannelAttribute ("Delay", StringValue ("1ms"));

    NS_LOG_INFO("Create nodes.");
    // Create dumbbell thingy
    PointToPointDumbbellHelper net (2, pointToPointLAN, 2, pointToPointLAN, pointToPointBB);

    // Install Internet Stack
    InternetStackHelper stack;
    net.InstallStack(stack);

    // Add IP addresses
    NS_LOG_INFO("Assign IP Addresses.");
    net.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                             Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                             Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

    // Create TCP server
    NS_LOG_INFO("Create TCP Server.");

    uint16_t port = 50000;
    Address sinkLocalAddress(InetSocketAddress (net.GetRightIpv4Address(0), port));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install (net.GetRight (0));

    sinkApp.Start (Seconds (0.0));
    sinkApp.Stop  (Seconds (50.0));


    // Create a TCP client socket
    NS_LOG_INFO("Create TCP Client Application.");

    Ptr<Node>   clientNode   = net.GetLeft(0);
    Ptr<Socket> clientSocket = Socket::CreateSocket(clientNode, TcpSocketFactory::GetTypeId());
    Ptr<MyApp>  clientApp    = CreateObject<MyApp>();

    // Set variables
    Ipv4Address clientAddress = net.GetRightIpv4Address(0);
    AddressValue localAddress (InetSocketAddress (clientAddress, port));
    uint32_t packetSize = 1380;

    // Define Callback
	clientSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));

    // Bind socket to the application and connect to server
    // with sending date rate and packet size.
    clientApp->Setup(clientSocket, sinkLocalAddress, packetSize, DataRate("8Mbps"));

    // Install the application to the client node
    clientNode->AddApplication(clientApp);

    // Set start and stop running time of the app.
    clientApp->SetStartTime(Seconds(0.0));
    clientApp->SetStopTime(Seconds(50));

    // Setup UDP Server
    NS_LOG_INFO("Add UDP Server");

    uint16_t UDPport = 60000;
    Address sinkAddressUDP (InetSocketAddress (net.GetRightIpv4Address(1), UDPport));
    PacketSinkHelper packetSinkHelperUDP ("ns3::UdpSocketFactory", sinkAddressUDP);
    ApplicationContainer sinkAppUDP = packetSinkHelperUDP.Install (net.GetRight (1));

    sinkAppUDP.Start (Seconds (0.));
    sinkAppUDP.Stop (Seconds (50.));

    // Add UDP Client
    NS_LOG_INFO("Add UDP Client");

    Ptr<Socket> clientSocketUDP = Socket::CreateSocket(net.GetLeft (1), UdpSocketFactory::GetTypeId());
    Ptr<MyApp>  clientAppUDP    = CreateObject<MyApp>();
    clientAppUDP->Setup(clientSocketUDP, sinkAddressUDP, 1460, DataRate("5Mbps"));
    net.GetLeft (1)->AddApplication(clientAppUDP);

    clientAppUDP->SetStartTime(Seconds(10.));
    clientAppUDP->SetStopTime(Seconds(30.));


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Enable ascii tracing or pcap tracing
    AsciiTraceHelper ascii;
    pointToPointBB.EnableAsciiAll(ascii.CreateFileStream("tcp-task2.tr"));
    pointToPointBB.EnablePcapAll("tcp-task2");

    // Set simulation timeout and run.
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(50.));
    Simulator::Schedule(Seconds(20.), &MyApp::ChangeRate, clientAppUDP, DataRate("10Mbps"));
    Simulator::Run();

    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}
