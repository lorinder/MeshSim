# Configuration file for location of STA nodes
#
# This describes the position allocation of STAs.  See mesh_mobility.txt
# on details of the file format.

ns3::ListPositionAllocator
<?php

require_once "scanargs.php";
$kv = get_kv($argv);

echo "".(($kv["size"] - 1)*$kv["meshDist"] + $kv["staDist"])." 0\n";

?>
