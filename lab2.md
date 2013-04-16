Lab 2: VoIP Protocols
=====================

Preparation
-----------

 1. I already had tcpdump installed.
 1. Asterisk server was installed by `apt-get`
 1. I will use the *Zoiper*  SIP client for my Mac and *Bria* for my iPhone.

Task 1: Asterisk Configuration
------------------------------

I first generated the MD5 hash for the users, thanks to [voip-info.org](http://www.voip-info.org/wiki/view/Asterisk+sip+md5secret):

    $ echo -n "appelman01:asterisk:an2013" | md5sum
    9bcc1b8aa11e2c44cc90de89f813d025  -
    $ echo -n "appelman02:asterisk:an2013" | md5sum
    23f134c77231e202ddd494448e9c471f  -

I could then add them to my `sip.conf` file:

    [general]
	srvlookup=yes

	disallow=all   ; First disallow all codecs
	allow=ulaw
	allow=alaw     ; Allow codecs in order of
	allow=ilbc     ; preference
	allow=gsm
	allow=h261

	[1001]
	type=peer
	secret=an2012
	host=dynamic   ; This device registers with us
	context=users

	[1002]
	type=peer
	host=dynamic
	md5secret=f520e168eaa4588992f022afb0779ee4   ;password: an2012
	context=users

	[1003]
	type=peer
	host=dynamic
	md5secret=5f25959ccfe17379c2c7fdd8df879482    ;password: 'an2012'
	context=users

	[appelman01]
	type=peer
	host=dynamic
	md5secret=9bcc1b8aa11e2c44cc90de89f813d025
	context=users

	[appelman02]
	type=peer
	host=dynamic
	md5secret=23f134c77231e202ddd494448e9c471f
	context=users




Task 2: Analyze VOIP streaming packets
--------------------------------------

*Perform a call from a SIP client to another. Use Wireshark/tcpdump to capture packets during the call session.*

**1. Which are the messages to register the SIP-phone to Asterisk server? Draw the workflow in the SIP registration phase and show equivalent SIP messages’ headers.**



**2. Which messages are used to establish the call, for both caller and callee sides? Draw the workflow including caller, callee and Asterisk server and show SIP message headers.**



**3. Capture RTP streaming in the call, using Wireshark to decode the captured conversation. What is the maximal and mean jitter values of the RTP stream? List some possible sources of jitter? Explain relation between jitter and delay.**

