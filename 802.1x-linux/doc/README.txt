$Id: README.txt,v 1.16 2002/10/17 10:58:05 rw Exp $

What is it?
===========

This is becoming a client side implementation of
IEEE 802.1x (EAP authenication over 802.3/11). I've heard of
open1x, but that doesn't support wired ethernet and I'd
consider to code generally hopeless, especially wrt protocol
independance.


Where?
======

The program is known to work on Linux-2.4 and FreeBSD-4.5. For
building on FreeBSD, GNU make is required.


Dependancies
============

OpenSSL.

State
=====

Should be usable to do MD5 and TLS authentification
over 802.3. Previous versions of the program might have worked over
802.11 by coincedence. This one will not. Adding 802.11 support should
be fairly easy, given access to appropriate hardware (which I don't
currently have). See README.802.11 for details if you're interested.


Using
=====

Commandline syntax is as follows:


	eapd [-dev <device>] [-mtu <mtu>]
	     [-user <user>] [-debug <n>] [-exit] [-start] [-daemon] [-nologoff]
	     [-md5] 
	     [-tls] [-cert <client cert>] [-ca <ca cert>] [-rand <rand file>]
	     [-nopw]
	     [-frags <frag size>] <username>

     
-dev		Network interface to use.
		(default: eth0)

-mtu		MTU of the network in question. If this value is too
		small, the program will probably segfault 'somewhen'.
		(default: 802.3-Ethernet, ie 1500)

-user		User to switch to after all priviledged actions
		have been carried out. This option is ignorend when
		running setuid.		

-debug		Set debug level (0: 'user mode', 1: 'warnings',
		2: 'informational messages', 3: 'debugging').
		(default: 0)

-exit		Exit after a single protocol exchange has been
		completed.

-start		Try to initiate a protocol conversation by trans-
		mitting EAPOL_START frames.

-nologoff	Don't transmit an EAPOL_LOGOFF frame on exit.

-md5		Enable MD5 based challenge response authentication.
		Read password from terminal (required by RFC2284).

-tls		Enable TLS authentification.

-cert		File containing the client certificate and
		private key in PEM format
		(default from environment: EAPD_CERT).

-ca		File containing a trusted root certificate
		in PEM format
		(default: env EAPD_CA).

-rand		File containing random data to seed the
		OpenSSL PRNG
		(default: env EAPD_RAND or /dev/urandom).

-nopw		The private key used by TLS is not
		encrypted.		

-frags		Fragment size to use when framing TLS
		conversation data into EAP
		(default: 200).

<username>	The username to use for EAP_IDENTITY-responses.


Operation
=========

After argument processing is complete, the program will open the
required network connections and initialize the EAP-layer. If running
setuid 0, it switches its uid and gid to its real uid and
gid. Otherwise, if a -user option has been given, it will use the
uid/gid of the given <user> (numeric or name). It then starts to
process all EAP requests it receives and transmit appropriate
responses, if possible, thereby (possibily periodically)
authenticating the machine it runs on to transmit non-EAPOL ethernet
frames through the switch port the given interface is connected
to. Currently, promiscous mode is used on FreeBSD to capture
packets destined to the PAE (port authenticator entity)-group address
defined in 802.1x, while the Linux-variant simply joins the 'correct'
link layer multicast group.

		--- weikusat@students.uni-mainz.de
