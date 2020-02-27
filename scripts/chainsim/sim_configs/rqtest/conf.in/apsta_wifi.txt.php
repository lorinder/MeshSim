<?php
require_once "scanargs.php";
$kv = get_kv($argv);
?>
standard g
ratecontrol ns3::MinstrelWifiManager

channel delay ns3::ConstantSpeedPropagationDelayModel
channel add_loss ns3::RangePropagationLossModel MaxRange=<?php
  echo $kv["staDist"]+0.1; ?>

phy_attribs ChannelNumber=1
