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



Task 4: Connecting to the central domain
---------------------------------------

Task 5: Analyse orchestration
----------------------------
