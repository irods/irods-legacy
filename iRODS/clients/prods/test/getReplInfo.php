<?php

require_once("../src/Prods.inc.php");


$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$target_file='/tempZone/home/rods/repl_test/ruleInp1';

try {
$start_time=microtime(true);

$myfile=new ProdsFile($account, $target_file);

$replinfo=$myfile->getReplInfo();

var_dump($replinfo);

$end_time=microtime(true);
$exec_time=$end_time-$start_time;
echo "--- test successful!  in ($exec_time sec) --- <br/>\n";

} catch (RODSException $e) {
  
  echo "--- test failed! --- <br/>\n";
  echo ($e);
  echo $e->showStackTrace();
}

?>