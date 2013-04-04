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

// Define your NS_LOG identifier here
NS_LOG_COMPONENT_DEFINE("lab1-task2-appelman");

int main(int argc, char *argv[]) {
    // Global configuration
    std::string DELAY = "16ms";
    
    CommandLine cmd;
    cmd.AddValue ("delay", "Time of delay on the link: <num>ms", DELAY);
    cmd.Parse (argc, argv);
    
    // Simulation parameters
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpTahoe")); 
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
    // Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (40));

    NS_LOG_INFO("Create nodes.");
    // Create network toplogy using NodeContainer class
    NodeContainer nodes;
    nodes.Create (2);

    // Create channels with specified data rate and delay, without IP addresses first
    NS_LOG_INFO("Create channel.");
    
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
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
    sinkApp.Start (Seconds (0.5));
    sinkApp.Stop (Seconds (50.0));


    NS_LOG_INFO("Create TCP Client Application.");
    
    // Create a TCP client socket
    Ptr<Node> clientNode = nodes.Get(1);
    Ptr<Socket> clientSocket = Socket::CreateSocket(clientNode,
    TcpSocketFactory::GetTypeId());
    
    // Create an application
    Ptr<MyApp> clientApp = CreateObject<MyApp>();

    // Set variables
    Ipv4Address clientAddress = interfaces.GetAddress (1);
    AddressValue localAddress (InetSocketAddress (clientAddress, port));
    Ipv4Address serverAddress = interfaces.GetAddress (0);
    Address remoteAddress (InetSocketAddress (serverAddress, port));
    uint32_t packetSize = 1400;
    
    // Bind socket to the application and connect to server
    // with sending date rate and packet size.
    clientApp->Setup(clientSocket, remoteAddress, packetSize, DataRate("5Mbps"));
    
    // Install the application to the client node
    clientNode->AddApplication(clientApp);
    
    // Set start and stop running time of the app.
    clientApp->SetStartTime(Seconds(1.0));
    clientApp->SetStopTime(Seconds(50));

    // Enable ascii tracing or pcap tracing
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("tcp-task2.tr"));
    pointToPoint.EnablePcapAll("tcp-task2");

    // Set simulation timeout and run.
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(50.));
    Simulator::Run();

    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}

