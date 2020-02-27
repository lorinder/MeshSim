<?php

require_once "scanargs.php";
$kv = get_kv($argv);

for ($i = 1; $i < $kv["size"]; $i++) {
	echo "X 10.1.2.".$i."\n";
}
echo "10.1.3.101 10.1.2.".$kv["size"]."\n"

?>
