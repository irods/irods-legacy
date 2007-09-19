<?php
require_once("../config.inc.php");
session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();
  
$ruri="";
if (isset($_REQUEST['ruri']))
  $ruri=$_REQUEST['ruri'];
else
{
  $response=array('success'=> false,'error'=>'Required RODS URI Required');
  echo json_encode($response);
  exit(0);
}    

$collection=ProdsDir::fromURI($ruri, false);
if (empty($collection->account->pass))
{
  $acct=$_SESSION['acct_manager']->findAcct($collection->account);
  if (empty($acct))
  {
    $response=array('success'=> false,'error'=>'Authentication Required');
    echo json_encode($response);
    exit(0);
  }
  $collection->account=$acct;
}  

try {

/*
$conn= new RODSConn($collection->account);
$conn->connect();
$resources=$conn->getResources();
$response=array('success'=> true,'totalCount'=>count($resources),
  'que_results'=> $resources);
echo json_encode($response);
$conn->disconnect();
*/

$que=new ProdsQuery($collection->account);
$resources=$que->getResources();
$response=array('success'=> true,'totalCount'=>count($resources),
  'que_results'=> $resources);
echo json_encode($response);

} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> $e->getMessage(),'errcode'=> $e->getCode());
  echo json_encode($response);
}
?>