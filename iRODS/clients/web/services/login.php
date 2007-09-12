<?php
require_once("../config.inc.php");
require_once(PRODS_INC_PATH);

session_start();

if (!isset($_SESSION['acct_manager']))
{
  $_SESSION['acct_manager']= new RODSAcctManager();
}

$host= (isset($_REQUEST['host']))?$_REQUEST['host']:DEFAULT_RODS_HOST;
$port= (isset($_REQUEST['port']))?$_REQUEST['port']:DEFAULT_RODS_PORT;
$user= (isset($_REQUEST['user']))?$_REQUEST['user']:DEFAULT_RODS_USER;
$pass= (isset($_REQUEST['pass']))?$_REQUEST['pass']:DEFAULT_RODS_PASS;

$acct=new RODSAccount($host, $port, $user, $pass);
try {
  $acct->getUserInfo();
  $_SESSION['acct_manager']->add($acct);
} catch (Exception $e) {
  generateClientError($e->getMessage());
}

if (isset($_REQUEST['init_path']))
  $ruri_home=$acct->getUserHomeDirURI($_REQUEST['init_path']);
else  
  $ruri_home=$acct->getUserHomeDirURI();
$var=array("success"=>true,"ruri_home"=>$ruri_home);

echo json_encode($var);  

function generateClientError($msg)
{
  echo json_encode(array("success"=>false, "errors" => $msg));
  exit(0);
}

?>
