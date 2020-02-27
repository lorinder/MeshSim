# Configuration file for applications running in the system

<?php
require_once "scanargs.php";
$kv = get_kv($argv);
if ($kv["peers"] == "chain") {
	$client = "10.1.4.1";
	$server = "10.1.1.1";
} else if ($kv["peers"] == "apsta") {
	$client = "10.1.4.1";
	$server = "10.1.3.".$kv["size"];
} else if ($kv["peers"] == "intramesh") {
	$client = "10.1.3.1";
	$server = "10.1.3.".$kv["size"];
}
echo "srv $server streaming_server "
echo "StartTime=20s PacketSize=1400 ";
echo "DataRate=".$kv["streamRate"]."Mbps ";
echo "OffTime=ns3::ConstantRandomVariable[Constant=0.0]\n";

echo "cl $client client\n";
echo "connect srv cl\n";

?>
