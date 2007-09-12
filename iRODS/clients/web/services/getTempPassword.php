<?php
require_once("../config.inc.php");

// set session id if its specified
$sid='';
if (isset($_REQUEST['SID']))
  $sid=$_REQUEST['SID'];
else
if (isset($_REQUEST['sid']))
  $sid=$_REQUEST['sid'];  
if (!empty($sid))
  session_id($sid);

session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();

$ruri="";
if (isset($_REQUEST['ruri']))
  $ruri=$_REQUEST['ruri'];
else
{
  $response=array('success'=> false,'error'=>'required RODS URI not found');
  echo json_encode($response);
  exit(0);
}    

$account=RODSAccount::fromURI($ruri, false);
if (empty($account->pass))
{
  $acct=$_SESSION['acct_manager']->findAcct($account);
  if (empty($acct))
  {
    $response=array('success'=> false,'error'=>'Authentication Required');
    echo json_encode($response);
    exit(0);
  }
  $account=$acct;
}  
try {
$temppass=$account->getTempPassword();
$response=array('success'=> true,'temppass'=>$temppass);
echo json_encode($response);

} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> $e->getMessage(),'errcode'=> $e->getCode());
  echo json_encode($response);
}
?>