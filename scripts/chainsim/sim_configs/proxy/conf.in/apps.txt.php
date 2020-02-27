# Configuration file for applications running in the system

defaults StartTime=10
<?php
require_once "scanargs.php";
$kv = get_kv($argv);

for ($i = 1; $i <= $kv["staSize"]; ++$i) {
	if ($kv["proto"] == "tcp") {
		echo "srv$i 10.1.1.1 file_server SendSize=65536\n";
	} else if ($kv["proto"] == "udp") {
		echo "srv$i 10.1.1.1 streaming_server " 
		. "OffTime=ns3::ConstantRandomVariable[Constant=0.0] " 
		. "OnTime=ns3::ConstantRandomVariable[Constant=1000.0] " 
		. "DataRate=" . $kv["udpCumRate"]*1.e6/$kv["staSize"] . " " 
		. "PacketSize=1280 \n";
	}
}

for ($i = 1; $i <= $kv["staSize"]; ++$i) {
	$ip = "10.1.4.$i";
	echo "client$i $ip client\n";
}

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

	for ($i = 1; $i <= $kv["meshSize"]; ++$i) {
		$ip = "10.1.2.$i";
		echo "proxy$i $ip timed_proxy "
		  . "DataRate=".$kv["proxyRate"]."Mbps "
		  . "SliceWidthMS=20 "
		  . "nSlices=$sl "
		  . "ActiveMap=".(1 << (($i - 1) % $sl))
		  . "\n";
	}
}

echo "echosrv   10.1.1.1 echo_server StartTime=1\n";
echo "echoclnt	10.1.4.1 echo_client StartTime=0\n";
?>

# Connections
<?php
if ($kv["proxyMode"] == 0) {
	for ($i = 1; $i <= $kv["staSize"]; ++$i) {
		echo "connect srv$i client$i\n";
	}
} else {
	assert($kv["staSize"] == 1);
	echo "connect srv1 proxy1\n";
	for ($i = 2; $i <= $kv["meshSize"]; ++$i) {
		echo "connect proxy".($i - 1)." proxy".$i."\n";
	}
	echo "connect proxy".$kv["meshSize"]." client1\n";
}
echo "connect echoclnt echosrv\n";
