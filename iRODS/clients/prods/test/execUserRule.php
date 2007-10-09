<?php

require_once("../src/Prods.inc.php");


$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$target_dir='/tempZone/home/rods/repl_test';
$rule_body='myTestRule||'.
  'delayExec(<PLUSET>1m</PLUSET>,'.
    'msiReplColl(*desc_coll,*desc_resc,backupMode,*outbuf),'.
  'nop)|nop';
$input_params=array('*desc_coll'=>'/tempZone/home/rods/repl_test',
  '*desc_resc'=>'demoResc2');
$out_params=array('*desc_coll');  

try {
$start_time=microtime(true);

$conn=new RODSConn($account);
$conn->connect();
$results=$conn->execUserRule($rule_body,$input_params,$out_params);
var_dump($results);


$end_time=microtime(true);
$exec_time=$end_time-$start_time;
echo "--- test successful!  in ($exec_time sec) --- <br/>\n";

} catch (RODSException $e) {
  
  echo "--- test failed! --- <br/>\n";
  echo ($e);
  echo $e->showStackTrace();
}

?>