<?php
require_once "scanargs.php";
$kv = get_kv($argv);
?>
--ns3::TcpSocket::SegmentSize=1446
--ns3::dot11s::HwmpProtocol::MaxTtl=2
--RngSeed=<?php echo $kv["seed"]."\n"; ?>
--meshSize=<?php echo $kv["meshSize"]."\n"; ?>
--staSize=<?php echo $kv["staSize"]."\n"; ?>
--ns3::dot11s::PeerLink::MaxRetries=<?php
	echo 4*$kv["peerLinkScale"]."\n"; ?>
--ns3::dot11s::PeerLink::MaxBeaconLoss=<?php
	echo 2*$kv["peerLinkScale"]."\n"; ?>
--ns3::dot11s::PeerLink::MaxPacketFailure=<?php
	echo 2*$kv["peerLinkScale"]."\n"; ?>
