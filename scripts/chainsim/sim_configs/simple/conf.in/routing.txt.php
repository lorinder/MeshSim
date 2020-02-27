<?php

require_once "scanargs.php";
$kv = get_kv($argv);

if ($kv["routing"] == "static") {
	// Read the routing template
	ob_start();
	require "routing_template.txt.php";
	$template = ob_get_contents();
	ob_clean();

	// filter through genroutingtables
	$fp = popen($kv["_genconf_dir"]."/genroutingtables -", "w");
	fwrite($fp, $template);
	pclose($fp);
} else {
	echo "proto ".$kv["routing"]."\n";
}

?>
