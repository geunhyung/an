Lab 4: Optical Networking
=========================

Task 1: Start an OpenNSA Domain
-------------------------------

    # apt-get update
    # apt-get install python-pip python-dev git
    # pip install twisted
    # pip install https://github.com/downloads/htj/suds-htj/suds-0.4.1-htj.tar.gz
    # git clone https://github.com/jeroenh/OpenNSA.git
    # cd OpenNSA
    # python setup.py install
    # wget -O - http://staff.science.uva.nl/~vdham/opennsa-confs.tgz | tar xzf - 
    # cd opennsa-confs

I added the edited files to this assignment.

    # ./onsa reserve -t ../opennsa-confs/test-topology.owl -n Michiel -s Michiel:A3 -d Michiel:A4
	Requester URL: http://localhost:7080/NSI/services/ConnectionService
	Site starting on 7080
	Connection ID: e6e37bbc-a8f7-11e2-834a-080027449efb
	Global ID: conn-1789
	Reservation created at <NetworkServiceAgent Michiel>
	(TCP Port 7080 Closed)

Task 2: Connect to Partner domain
---------------------------------

 1. What information do you need to exchange to make the connection?

We need at least the IP and the STPs:

    <!-- urn:ogf:network:nsnetwork:Razvan -->
    <owl:NamedIndividual rdf:about="urn:ogf:network:nsnetwork:Razvan">
        <rdf:type rdf:resource="http://www.glif.is/working-groups/tech/dtox#NSNetwork"/>
        <rdfs:label xml:lang="en">Razvan</rdfs:label>
        <hasSTP rdf:resource="urn:ogf:network:stp:Razvan:A1"/>
        <hasSTP rdf:resource="urn:ogf:network:stp:Razvan:A2"/>
        <hasSTP rdf:resource="urn:ogf:network:stp:Razvan:A3"/>
        <hasSTP rdf:resource="urn:ogf:network:stp:Razvan:A4"/>
        <managedBy rdf:resource="urn:ogf:network:nsa:Razvan"/>
    </owl:NamedIndividual>
    
    <!-- urn:ogf:network:nsa:Razvan -->
    <owl:NamedIndividual rdf:about="urn:ogf:network:nsa:Razvan">
        <rdf:type rdf:resource="http://www.glif.is/working-groups/tech/dtox#NSA"/>
        <managing rdf:resource="urn:ogf:network:nsnetwork:Razvan" />
        <adminContact rdf:datatype="http://www.w3.org/2001/XMLSchema#string">Razvan network</adminContact>
        <csProviderEndpoint rdf:datatype="http://www.w3.org/2001/XMLSchema#string">http://145.100.104.136:9080/NSI/services/ConnectionService</csProviderEndpoint>
    </owl:NamedIndividual>

But we got an error message: `Failure: Razvan <Razvan:A1--Razvan:A1> Server raised fault: 'Cannot connect <STP Razvan:A1> to itself.'`

And then we used the other STP at Razvan's network:

    # ./onsa reserve -t ../opennsa-confs/test-topology.owl -n Michiel -s Michiel:A1 -d Razvan:A3
	Requester URL: http://localhost:7080/NSI/services/ConnectionService
	Site starting on 7080
	Connection ID: d5cfa676-a8fb-11e2-834a-080027449efb
	Global ID: conn-4726
	Reservation created at <NetworkServiceAgent Michiel>
	(TCP Port 7080 Closed)

Task 3: Request on Partner domain
--------------------------------

I got a reservation from Razvan in my logs:

    2013-04-19 16:23:19+0200 [opennsa] 
	2013-04-19 16:23:19+0200 [opennsa.NSIService] Connection urn:uuid:b85f6634-a8fc-11e2-af85-00188bf7d8a5. Reserve request from urn:ogf:network:nsa:Razvan.
	2013-04-19 16:23:19+0200 [opennsa.NSIService] Connection urn:uuid:b85f6634-a8fc-11e2-af85-00188bf7d8a5: Simple path creation: Michiel:A1 -> Michiel:A2 (Michiel)
	2013-04-19 16:23:19+0200 [DUD Network Michiel] Link: 177379308, DudDevicePort_A1 -> DudDevicePort_A2 : RESERVING.
	2013-04-19 16:23:19+0200 [opennsa.Scheduler] State transition scheduled: In 35 seconds to state Scheduled
	2013-04-19 16:23:19+0200 [DUD Network Michiel] Link: 177379308, DudDevicePort_A1 -> DudDevicePort_A2 : RESERVED.
	2013-04-19 16:23:19+0200 [opennsa.Connection] Connection urn:uuid:b85f6634-a8fc-11e2-af85-00188bf7d8a5: Reserve succeeded
	2013-04-19 16:23:19+0200 [opennsa.Scheduler] State transition scheduled: In 35 seconds to state Scheduled
	
And then I did the same from my server to his:

    # ./onsa reserve -t ../opennsa-confs/test-topology.owl -n Michiel -s Michiel:A1 -d Razvan:A2
	Requester URL: http://localhost:7080/NSI/services/ConnectionService
	Site starting on 7080
	Connection ID: 47cd72e8-a8fd-11e2-834a-080027449efb
	Global ID: conn-8249
	Reservation created at <NetworkServiceAgent Michiel>
	(TCP Port 7080 Closed)

Task 4: Connecting to the central domain
---------------------------------------

So with help of Jeroen I finally got the message that the defined paths in the OWL file are static and thus they cannot be connected. So I did:

    # ./onsa reserve -t ../opennsa-confs/test-topology.owl -n Michiel -s Michiel:A2 -d uva:michiel2
	Requester URL: http://localhost:7080/NSI/services/ConnectionService
	Site starting on 7080
	Connection ID: 2fd496b6-a903-11e2-834a-080027449efb
	Global ID: conn-6752
	(TCP Port 7080 Closed)
	
In summary you need to:

 - Define the STP's
  - Including a link between the two networks
 - Define the network
 - Define the NSA

And now it worked! Please see the [test-topology.owl](test-topology.owl) file to see how I defined the links. 

Task 5: Analyse orchestration
----------------------------

*1. Describe what you would need to be able to do reservations with other students.*

You would need the names of the STP's in the middle network and the NSA and STP's of the end network.

***THIS SPACE INTENTIONALLY LEFT BLANK***

*3. Is this a tree or chain model? Explain the difference.*

This is will count as a _chain_ because there is a single line between two (and no more and no less) points.


