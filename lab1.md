Lab 1: TCP Performance Monitoring
=================================

Installation tutorial
---------------------

Following the ns-3 installation tutorial:

    $ wget https://www.nsnam.org/release/ns-allinone-3.16.tar.bz2
    $ bunzip2 ns-allinone-3.16.tar.bz2 
	$ tar xf ns-allinone-3.16.tar 
	$ cd ns-allinone-3.16/
	$ vi README 
	$ ./build.py --enable-examples --enable-tests
	'build' finished successfully (1.133s)

	Modules built:
	antenna                   aodv                      applications             
	bridge                    buildings                 config-store             
	core                      csma                      csma-layout              
	dsdv                      dsr                       emu                      
	energy                    flow-monitor              internet                 
	lte                       mesh                      mobility                 
	mpi                       netanim (no Python)       network                  
	nix-vector-routing        olsr                      point-to-point           
	point-to-point-layout     propagation               spectrum                 
	stats                     tap-bridge                test (no Python)         
	tools                     topology-read             uan                      
	virtual-net-device        wifi                      wimax                    

	Modules not built (see ns-3 tutorial for explanation):
	brite                     click                     openflow                 
	visualizer
	$ cd ns-3.16/
	$ ./test.py -c core
	$ ...
	145 of 148 tests passed (145 passed, 3 skipped, 0 failed, 0 crashed, 0 valgrind errors)
	$ ./waf --run hello-simulator
	Waf: Entering directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
	Waf: Leaving directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
	'build' finished successfully (2.794s)
	Hello Simulator

Apparently it works!

**Study how to:**

create nodes

    NodeContainer nodes;
    nodes.Create (2);

setup network topology

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    
    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

assign IP addresses

    InternetStackHelper stack;
    stack.Install (nodes);
    
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

install UDP applications.

    UdpEchoServerHelper echoServer (9);
    
    ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));
    
    UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
    
    ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

**Study how to create UDP application using OnOffHelper class:**

I couldn't find anything on this in the `$NS3 HOME/examples/udp- client-server/udp-client-server.cc` file. Cahn admitted that it was his mistake.


**Open `$NS3 HOME/examples/tcp/star.cc` to study how to use OnOffHelper class to create TCP application server and clients.**

    // Create on/off TCP socket
    OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

    // Create container 'spokeApps'
    ApplicationContainer spokeApps;

    // For every spoke node ..
    for (uint32_t i = 0; i < star.SpokeCount (); ++i)
    {   
	  // set the Server IP and port ..
      AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port));
      // set the remote address for the TCP on/off socket ..
      onOffHelper.SetAttribute ("Remote", remoteAddress);
      // install the on/off helper for the node i in the container
      spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
    }   
    // Start all the applications installed in the spokeApps container
    spokeApps.Start (Seconds (1.0));
    spokeApps.Stop (Seconds (10.0));

**Create TCP socket and monitor congestion window: in `$NS3 HOME/examples/tutorial/fifth.cc`**

Task 1: Throughput of TCP
-------------------------

    mike@alpha:~/ns-allinone-3.16/ns-3.16$ ./waf --run "lab1"
    Waf: Entering directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
    Waf: Leaving directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
    'build' finished successfully (1.542s)
    Flow 1, delay: 64ms (10.1.1.2 -> 10.1.1.1)
      Tx Bytes:   1616680
      Rx Bytes:   1616680
      Throughput: 0.246686 Mbps
    Flow 2, delay: 64ms (10.1.1.1 -> 10.1.1.2)
      Tx Bytes:   58320
      Rx Bytes:   58320
      Throughput: 0.00889893 Mbps
    mike@alpha:~/ns-allinone-3.16/ns-3.16$ ./waf --run "lab1 --delay=96ms"
    Waf: Entering directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
    Waf: Leaving directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
    'build' finished successfully (1.553s)
    Flow 1, delay: 96ms (10.1.1.2 -> 10.1.1.1)
      Tx Bytes:   1682744
      Rx Bytes:   1682744
      Throughput: 0.256766 Mbps
    Flow 2, delay: 96ms (10.1.1.1 -> 10.1.1.2)
      Tx Bytes:   61200
      Rx Bytes:   61200
      Throughput: 0.00933838 Mbps
    mike@alpha:~/ns-allinone-3.16/ns-3.16$ ./waf --run "lab1 --delay=128ms"
    Waf: Entering directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
    Waf: Leaving directory `/home/mike/ns-allinone-3.16/ns-3.16/build'
    'build' finished successfully (1.552s)
    Flow 1, delay: 128ms (10.1.1.2 -> 10.1.1.1)
      Tx Bytes:   1678000
      Rx Bytes:   1678000
      Throughput: 0.256042 Mbps
    Flow 2, delay: 128ms (10.1.1.1 -> 10.1.1.2)
      Tx Bytes:   62000
      Rx Bytes:   62000
      Throughput: 0.00946045 Mbps


Task 2: Monitoring TCP congestion window
----------------------------------------

