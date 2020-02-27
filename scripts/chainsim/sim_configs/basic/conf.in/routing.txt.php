<?php require_once "scanargs.php";
$kv = get_kv($argv);

if ($kv["routing"] == "static") {
?>

proto none

# backhaul
routes_on 10.1.1.1
0.0.0.0/0	10.1.1.1	10.1.1.2

# STA
routes_on 10.1.3.101
0.0.0.0/0	10.1.3.101	10.1.3.<?php echo $kv["size"]."\n"; ?>
10.1.4.1	10.1.4.101

# wSTA
routes_on 10.1.4.1
0.0.0.0/0	10.1.4.1	10.1.4.101

# mesh nodes
<?php
// The first n - 1 mesh nodes
for ($i = 1; $i <= $kv["size"]; ++$i) {
	echo "routes_on 10.1.2.".$i."\n";

	// Default route
	if ($i > 1) {
		echo "0.0.0.0/0\t10.1.2.".$i."\t10.1.2.".($i - 1)."\n";
	} else {
		echo "0.0.0.0/0\t10.1.1.2\t10.1.1.1\n";
	}

	// Exceptions:  Everything that is further away from the backhaul
	$nexthop = "10.1.2.".($i - 1);
	for ($j = 1; $j < $i; ++$j) {
		echo "10.1.2.$j\t10.1.2.$i\t$nexthop\n";
		echo "10.1.3.$j\t10.1.2.$i\t$nexthop\n";
	}
	if ($i < $kv["size"]) {
		$nexthop = "10.1.2.".($i + 1);
		for ($j = $i + 1; $j <= $kv["size"]; ++$j) {
			echo "10.1.2.$j\t10.1.2.$i\t$nexthop\n";
			echo "10.1.3.$j\t10.1.2.$i\t$nexthop\n";
		}
		echo "10.1.3.101\t10.1.2.$i\t$nexthop\n";
		echo "10.1.4.101\t10.1.2.$i\t$nexthop\n";
		echo "10.1.4.1\t10.1.2.$i\t$nexthop\n";
	} else {
		$iface = "10.1.3.".$i;
		$nexthop = "10.1.3.101";
		echo "10.1.3.101\t$iface\n";
		echo "10.1.4.101\t$iface\t$nexthop\n";
		echo "10.1.4.1\t$iface\t$nexthop\n";
	}
	echo "\n";
}
?>

<?php 
} else {
	echo "proto ".$kv["routing"]."\n";
}
?>
