# Configuration file for applications running in the system

defaults StartTime=10
<?php
require_once "scanargs.php";
$kv = get_kv($argv);

// Need to fix many things for support of larger staSize
assert($kv["staSize"] == 1);

// Origin sender
echo "srv 10.1.1.1 file_server SendSize=65536\n";

// Mesh Rq Encoder
if ($kv["proto"] == "rq") {
        echo "rqenc 10.1.2.1 rq_encoder"
        . " Kval=" . $kv["rqK"]
        . " Nval=" . $kv["rqN"]
        . " Tval=" . $kv["rqT"]
        . " DataRate=" . $kv["udpCumRate"]* 1e6 *
	   $kv["rqN"]/$kv["rqK"] . "\n";
}

// Proxies 

// proxyMode meanings:
//	0	don't use proxies at all.
//	1	use proxies with each alternating proxy in send mode at
//		any point in time
//	2	use proxies where every third proxy is in send mode at
//		any point in time
if ($kv["proxyMode"] > 0) {
	$sl = 2;
	if ($kv["proxyMode"] >= 2)
		$sl = 3;

	for ($i = 1; $i < $kv["meshSize"]; ++$i) {
		$ip = "10.1.2.$i";
		echo "proxy$i $ip timed_proxy"
		  . " DataRate=".$kv["proxyRate"]."Mbps"
		  . " SliceWidthMS=20"
		  . " nSlices=$sl"
		  . " ActiveMap=".(1 << (($i - 1) % $sl))
		  . "\n";
	}
}

// Mesh Rq Decoders
if ($kv["proto"]  == "rq") {
        echo "rqdec 10.1.2." . $kv["meshSize"]
	  . " rq_decoder"
          . " Kval=" . $kv["rqK"]
          . " Nval=" . $kv["rqN"]
          . " Tval=" . $kv["rqT"] . "\n";
}

// Echo for latency checks
echo "echosrv   10.1.1.1 echo_server StartTime=1 StopTime=5\n";
echo "echoclnt	10.1.4.1 echo_client StartTime=0 StopTime=5\n";

// Client
echo "client 10.1.4.1 client\n";

?>

# Connections
<?php

// Connect server to encoder
$upstream="srv";
if ($kv["proto"] == "rq") {
	echo "connect $upstream rqenc\n";
	$upstream = "rqenc";
}

// Add the proxies
if ($kv["proxyMode"] > 0) {
	for ($i = 1; $i < $kv["meshSize"]; ++$i) {
		$next = "proxy".$i;
		echo "connect $upstream $next\n";
		$upstream = $next;
	}
}

// Connect to decoder
if ($kv["proto"] == "rq") {
	echo "connect $upstream rqdec\n";
	$upstream = "rqdec";
}

// Connect to client
echo "connect $upstream client\n";

echo "connect echoclnt echosrv\n";
