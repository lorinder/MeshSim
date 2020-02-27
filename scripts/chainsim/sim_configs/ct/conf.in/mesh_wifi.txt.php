standard n5
ratecontrol ns3::MinstrelHtWifiManager

<?php
require_once "scanargs.php";
$kv = get_kv($argv);

if ($kv["lossmodel"] == "los") {
	echo "channel delay ns3::ConstantSpeedPropagationDelayModel\n";
	echo "channel add_loss ns3::ItuR1411LosPropagationLossModel Frequency=5e+9\n";
} else {
	echo "channel delay ns3::ConstantSpeedPropagationDelayModel\n";
	echo "channel add_loss ns3::ItuR1411NlosOverRooftopPropagationLossModel Frequency=5e+9 CitySize=Medium\n";
}

?>
