<?php

function get_kv($argv) {
	$kv = array();
	foreach(array_slice($argv, 1) as $stmt) {
		list($key, $value) = explode('=', $stmt, 2);
		$kv[$key] = $value;
	}
	return $kv;
}

function ob_end_sprint($kv) {
	$contents = ob_get_contents();
	ob_end_clean();

	# Construct kv2
	$kv2 = array();
	foreach($kv as $k => $v) {
		$kv2["\$$k"] = $v;
	}

	echo strtr($contents, $kv2);
}

?>
