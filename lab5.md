Lab 5: MPLS Treasure Hunt
==========================

**What are the IP addressing details of this topology?**

 ![lab5-topo.png](Lab 5 Topology)

**Which IGP is used?**

 OSPF.

**Describe in detail its configuration and its functioning. Use show commands to explain the set-up.**

When we look at the config we see one large backbone area, with no stub areas. Which is pretty similar on all routers:

    ospf {
        area 0.0.0.0 {
            interface ge-0/0/4.56 {
                interface-type p2p;
                hello-interval 1;
                dead-interval 4;
            }
            interface ge-0/0/4.67 {
                interface-type p2p;
                hello-interval 1;
                dead-interval 4;
            }
            interface lo0.0;
        }
    }

Except for R8, which has an extra policy and also a metric configured on neighbour R5 (also present from R5, by the way). 

    export flikkerinospf;
    area 0.0.0.0 {
        interface ge-0/0/4.58 {
            interface-type p2p;
            metric 1000;
            hello-interval 1;
            dead-interval 4;
        }
    }


**Question 2: Which routes are present on the routers (which routing table)?**

On R8 we see the following IP routes:

    inet.0: 15 destinations, 15 routes (15 active, 0 holddown, 0 hidden)
    + = Active Route, - = Last Active, * = Both
    
    5.5.5.5/32         *[OSPF/10] 00:49:51, metric 3
                        > to 192.168.78.1 via ge-0/0/4.78
    6.6.6.6/32         *[OSPF/10] 00:49:51, metric 2
                        > to 192.168.78.1 via ge-0/0/4.78
    7.7.7.7/32         *[OSPF/10] 00:49:51, metric 1
                        > to 192.168.78.1 via ge-0/0/4.78
    8.8.8.8/32         *[Direct/0] 00:50:01
                        > via lo0.0
    10.10.1.0/24       *[Direct/0] 00:50:21
                        > via ge-0/0/0.0
    10.10.1.8/32       *[Local/0] 00:50:35
                          Local via ge-0/0/0.0
    10.10.10.0/24      *[Static/5] 00:50:21
                        > to 10.10.1.254 via ge-0/0/0.0
    145.125.0.0/16     *[Static/5] 00:50:11
                          Discard
    192.168.56.0/24    *[OSPF/10] 00:49:51, metric 3
                        > to 192.168.78.1 via ge-0/0/4.78
    192.168.58.0/24    *[Direct/0] 00:50:01
                        > via ge-0/0/4.58
    192.168.58.2/32    *[Local/0] 00:50:01
                          Local via ge-0/0/4.58
    192.168.67.0/24    *[OSPF/10] 00:49:51, metric 2
                        > to 192.168.78.1 via ge-0/0/4.78
    192.168.78.0/24    *[Direct/0] 00:50:01
                        > via ge-0/0/4.78
    192.168.78.2/32    *[Local/0] 00:50:01
                          Local via ge-0/0/4.78
    224.0.0.5/32       *[OSPF/10] 00:50:11, metric 1
                          MultiRecv

Which are pretty much expected when you look at the topology. Except for the explicit discard which is configured below:

    routing-options {
        static {
            route 10.10.10.0/24 {
                next-hop 10.10.1.254;
                no-readvertise;
            }
            route 145.125.0.0/16 discard;
        }
        router-id 8.8.8.8;
    }

Thanks to the earlier mentioned policy `flikkerinospf` this is redistributed across the whole area:

    policy-options {
	    policy-statement flikkerinospf {
	        term 1 {
	            from protocol static;
	            then accept;
	        }
	    }
	}

Causing all traffic for that network to be routed to R8 and then discarded.

With regards to the LDP configuration and connectivity, we can see on R8:

    inet.3: 3 destinations, 3 routes (3 active, 0 holddown, 0 hidden)
    + = Active Route, - = Last Active, * = Both
    
    5.5.5.5/32         *[LDP/9] 00:49:47, metric 1
                        > to 192.168.78.1 via ge-0/0/4.78, Push 299792

This means that R8 has heard from R7 that to reach R5 (5.5.5.5) it can use the 299792 label and R7 will forward to R5. Although this is a longer path (R8-R7-R6-R5), the path checks out because the IGP (OSPF) considers the R5-R8 link longer because it has a metric of 1000.

When we take a look at R7:

     inet.3: 3 destinations, 3 routes (3 active, 0 holddown, 0 hidden)
     + = Active Route, - = Last Active, * = Both
     
     5.5.5.5/32         *[LDP/9] 01:06:32, metric 1
                         > to 192.168.67.1 via ge-0/0/4.67, Push 299776

We see it will reach R5 by forwarding it to R6 and push label 299776. Which will be useful to R8 because we also see on R7:

    mpls.0: 10 destinations, 10 routes (10 active, 0 holddown, 0 hidden)
    + = Active Route, - = Last Active, * = Both
    
    ...
    299792             *[LDP/9] 01:06:32, metric 1
                        > to 192.168.67.1 via ge-0/0/4.67, Swap 299776

Here it says that R7 will swap the incoming 299792 label with the 299776 label.


**Question 3: What are the MPLS labels used? Which LSPs are configured between the routers? Explain the labels used by each LSP.**

There is only one LSP configured in the whole setup and that is the LSP `toR8`. This LSP has an ingress at R5 and an egress at R8, with R6 and R7 acting as LSRs.

    student@srx5> show mpls lsp
	Ingress LSP: 1 sessions
	To              From            State Rt P     ActivePath       LSPname
	8.8.8.8         5.5.5.5         Up     0 *                      toR8
	Total 1 displayed, Up 1, Down 0

	student@srx6> show mpls lsp
	Transit LSP: 1 sessions
	To              From            State   Rt Style Labelin Labelout LSPname
	8.8.8.8         5.5.5.5         Up       0  1 FF  299824   299824 toR8
	Total 1 displayed, Up 1, Down 0

	student@srx7> show mpls lsp
	Transit LSP: 1 sessions
	To              From            State   Rt Style Labelin Labelout LSPname
	8.8.8.8         5.5.5.5         Up       0  1 FF  299824        3 toR8
	Total 1 displayed, Up 1, Down 0

	student@srx8> show mpls lsp
	Egress LSP: 1 sessions
	To              From            State   Rt Style Labelin Labelout LSPname
	8.8.8.8         5.5.5.5         Up       0  1 FF       3        - toR8
	Total 1 displayed, Up 1, Down 0


In the above output you can also see the used labels for traffic forwarding between every MPLS router in the LSP path. The label `3` is the *Explicit NULL* label which will cause R7 to already pop the label for R8 so it can start processing the packet immediately.

**Question 4: RSVP is also configured, which tunnel is built?**

The LSP which has been defined is also configured as an RSVP tunnel because of the `traffic-engineering bgp-igp` statement. This causes the `inet.3` table to be incorporated into the `inet.0` table, allowing for the use of the lower level information from MPLS and RSVP by the upper layer IGP of R5. Which finally means that the following route will be installed:

    145.125.0.0/32     *[RSVP/7/1] 01:52:25, metric 3
	                    > to 192.168.56.2 via ge-0/0/4.56, label-switched-path toR8

This process can be found on the [Juniper Tech Publications](http://www.juniper.net/techpubs/en_US/junos9.5/information-products/topic-collections/config-guide-mpls-applications/mpls-configuring-traffic-engineering-for-lsps.html#id-30128)

**Why is this tunnel preferred above the LSP built by LDP? What do you need to change this preference?**

RSVP has a default administrative distance of 7 which is lower than LDP (which has 9) and OSPF internal (which has 10). You can change that distance yourself to a higher value if you want. See also this [list of AD's](http://switchingandrouting.wordpress.com/2011/07/10/administrative-distance-ad-values-in-cisco-juniper/).
