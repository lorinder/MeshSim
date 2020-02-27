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
	$ip = "10.1.4.".$i;
	echo "client$i $ip client\n";
}

echo "echosrv   10.1.1.1 echo_server StartTime=1\n";
echo "echoclnt	10.1.4.1 echo_client StartTime=0\n";
?>

# Connections
<?php
for ($i = 1; $i <= $kv["staSize"]; ++$i) {
	echo "connect srv$i client$i\n";
}
echo "connect echoclnt echosrv\n";
