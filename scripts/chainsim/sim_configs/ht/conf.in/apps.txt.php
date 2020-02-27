# Configuration file for applications running in the system

echo_srv 10.1.1.1	echo_server
echo_cl	10.1.4.1	echo_client

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
echo "srv $server file_server StartTime=20s SendSize=65536\n";
echo "cl $client client\n";
?>
connect echo_cl echo_srv
connect srv cl
