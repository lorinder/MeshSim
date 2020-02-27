<?php
require_once "scanargs.php";
$kv = get_kv($argv);
echo "--RngSeed=".$kv["seed"]."\n";
echo "--meshSize=".$kv["size"]."\n";
if ($kv["hwmp"] == 0) {
	echo "--ns3::dot11s::HwmpProtocol::MaxTtl=2\n";
}
?>
--staSize=1
--ns3::TcpSocket::SegmentSize=1446
