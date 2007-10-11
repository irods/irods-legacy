<?php

require_once("../src/Prods.inc.php");


$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$target_dir='/tempZone/home/rods/repl_test';
$rule_body='myTestRule||'.
  'delayExec(<PLUSET>1m</PLUSET>,'.
    'msiReplColl(*desc_coll1,*desc_resc,backupMode,*outbuf)'.
    '##msiReplColl(*desc_coll2,*desc_resc,backupMode,*outbuf),'.
  'nop)|nop';
$input_params=array('*desc_coll1'=>'/tempZone/home/rods/repl_test3',
  '*desc_coll2'=>'/tempZone/home/rods/repl_test4',
  '*desc_resc'=>'demoResc2');
$out_params=array('*desc_coll1','*desc_coll2');  

try {
$start_time=microtime(true);

$conn=new RODSConn($account);
$conn->connect();
$results=$conn->execUserRule($rule_body,$input_params,$out_params);
var_dump($results);
$conn->disconnect();

$rule=new ProdsRule($account,$rule_body,$input_params,$out_params);
$results=$rule->execute(); 
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