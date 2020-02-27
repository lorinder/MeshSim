This directory contains variants of the chainsim setup.

It reflects some of the past experiments performed.  The parts are also
useful to create new simulation configurations.

simple:
	Fairly simple TCP setup.  Single client requesting at the end of
	the chain.  No lower level parameters tuned (other than Hwmp
	TTL).  In particular, expect all the HWMP bugs in this one.

	Allows for relatively free routing specification.


basic:
	A simple model attempting to simulate the easiest possible case.

	Uses UDP; again single client.
	Testable segments: Entire chain [chain] (backhaul server <-> STA),
	Ap<->STA [apsta] (not going through the mesh), and within the
	mesh [intramesh].

	Constant Rate station Manager.

	Supports static routing or other methods.

	Attempts 802.11n even with multiple antennas & streams.
	
	Loss model is range propagation loss, i.e., as perfect as it
	gets.

ct:
	Connectivity/rate tests with different propagation loss models
	(e.g. outdoors line of sight or not) as well as varying transmit
	powers.

	Single TCP application.

ht:
	High Throughput (i.e., 802.11n) tests.

	Uses TCP; has many of the 802.11n performance parameters
	tunable.  Just like conf_basic, can run chain, apsta and
	intramesh.

	Was used to test which parameter settings of 802.11n (e.g. # of
	antennas, greenfield, etc.) affect performance in what way.

	Is more complicated as conf_basic in the sense that it uses TCP,
	more complicated Station Manager, etc.

ht_udp:
	Somewhat simplified variant of conf_ht; uses UDP instead of TCP.

multiple:
	TCP based using multiple clients, moved back to 802.11g.

proxy:
	A simple test for comparing udp proxy setups to tcp.  Here the
	proxy setups have been tuned to achieve close to the highest
	rates but with the goal of having < 10% packet loss.  The idea
	is that those parameters should then be usable together with
	decoders.

rqtest:
	A test of RqEncoder and RqDecoder.  This is what was used for
	the data of our 2019/12/17 presentation.

rq_multiclient:
	A test of RqEncoder and RqDecoder with multiple clients.
