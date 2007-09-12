<?php

require_once("../src/Prods.inc.php");


$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$home_dir_str='/tempZone/home/rods';

try {

echo "--- trying to connect --- ".microtime()." <br/>\n";
$conn = RODSConnManager::getConn($account);

echo "--- trying temp password #1 --- ".microtime()." <br/>\n";
$key=$conn->connect(); 
$key=$conn->getKeyForTempPassword(); 
echo "key = $key <br/> \n";
$temppass=$conn->getTempPassword($key);
echo "temppass = $temppass <br/> \n";
$account2=clone $account;
$account2->pass=$temppass;
echo "--- trying to connect with temp password--- ".microtime()." <br/>\n";
$conn2=new RODSConn($account2);
$conn2->connect();
echo "--- temp password #1 passed --- ".microtime()." <br/>\n";

echo "--- trying temp password #2 --- ".microtime()." <br/>\n";
$temppass=$account->getTempPassword();
echo "temppass#2 = $temppass <br/> \n";
$account2=clone $account;
$account2->pass=$temppass;
echo "--- trying to connect with temp password #2 --- ".microtime()." <br/>\n";
$conn2=new RODSConn($account2);
$conn2->connect();
echo "--- temp password #2 passed --- ".microtime()." <br/>\n";

echo "--- test successful! --- ".microtime()." <br/>\n";

} catch (RODSException $e) {
  
  echo "--- test failed! --- ".microtime()." <br/>\n";
  echo ($e);
  echo $e->showStackTrace();
}

?>