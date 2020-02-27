<?php
require_once "scanargs.php";
$kv = get_kv($argv);
?>
proto none

# backhaul
routes_on 10.1.1.1
0.0.0.0/0	10.1.1.1	10.1.1.2

# STA
<?php
for ($i = 1; $i <= $kv["staSize"]; ++$i) {
	echo "routes_on 10.1.3.".(100 + $i)."\n";
	echo "0.0.0.0/0\t10.1.3.".(100 + $i)."\t10.1.3.".$kv["meshSize"]."\n"; 
	echo "10.1.4.$i\t10.1.4.".(100 + $i)."\n";
}
?>

# wSTA
<?php
for ($i = 1; $i <= $kv["staSize"]; ++$i) {
	echo "routes_on 10.1.4.$i\n";
	echo "0.0.0.0/0\t10.1.4.$i\t10.1.4.".(100 + $i)."\n";
	echo "\n";
}
?>

# mesh nodes
<?php
// The first n - 1 mesh nodes
for ($i = 1; $i < $kv["meshSize"]; ++$i) {
	echo "routes_on 10.1.2.$i\n";

	// Default route
	// (pointing away from the backhaul)
	echo "0.0.0.0/0\t10.1.2.$i\t10.1.2.".($i + 1)."\n";

	// Exceptions:  Everything that is closer to the backhaul
	if ($i > 1) {
		$nexthop = "10.1.2.".($i - 1);
		echo "10.1.1.1\t10.1.2.$i\t$nexthop\n";
		for ($j = 1; $j < $i; ++$j) {
			echo "10.1.2.$j\t10.1.2.$i\t$nexthop\n";
			echo "10.1.3.$j\t10.1.2.$i\t$nexthop\n";
		}
	} else {
		echo "10.1.1.1\t10.1.1.2\t10.1.1.1\n";
	}
	echo "\n";
}
// The last hop
// Here we point the default root to the backhaul and then
// implement specific rules for each STA.
$i = $kv["meshSize"];
echo "routes_on 10.1.2.$i\n";
echo "0.0.0.0/0\t10.1.2.$i\t10.1.2.".($i - 1)."\n";
for ($j = 1; $j <= $kv["staSize"]; ++$j) {
	echo "10.1.3.".($j + 100)."\t10.1.3.$i\t10.1.3.".($j + 100)."\n";
	echo "10.1.4.".($j + 100)."\t10.1.3.$i\t10.1.3.".($j + 100)."\n";
	echo "10.1.4.".$j."\t10.1.3.$i\t10.1.3.".($j + 100)."\n";
}

