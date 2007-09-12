<?php

require_once("../src/Prods.inc.php");


$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$target_dir='/tempZone/home/rods';

try {
$start_time=microtime(true);

$dir=new ProdsDir($account, $target_dir);

$terms= array('descendentOnly'=>true, 'recursive'=>true);

$total_count=0;
$results=$dir->findFiles($terms,$total_count);

foreach ($results as $file)
{
  echo $file->path_str."\n";
}

echo "Total: $total_count files";

$end_time=microtime(true);
$exec_time=$end_time-$start_time;
echo "--- test successful!  in ($exec_time sec) --- <br/>\n";

} catch (RODSException $e) {
  
  echo "--- test failed! --- <br/>\n";
  echo ($e);
  echo $e->showStackTrace();
}

?>