<?php

require_once("../src/Prods.inc.php");


$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$target_dir='/tempZone/home/rods/pictures';

try {
$start_time=microtime(true);

$dir=new ProdsDir($account, $target_dir);

$metas=$dir->rmMetaByID(11053);



$metas=$dir->getMeta();


var_dump($metas);

$end_time=microtime(true);
$exec_time=$end_time-$start_time;
echo "--- test successful!  in ($exec_time sec) --- <br/>\n";

} catch (RODSException $e) {
  
  echo "--- test failed! --- <br/>\n";
  echo ($e);
  echo $e->showStackTrace();
}

?>