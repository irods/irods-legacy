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
  $response=array('success'=> false,'log'=>'Expected RODS URI not found');
  echo json_encode($response);
  exit(0);
} 

$name='';
if (isset($_REQUEST['name']))
  $name=$_REQUEST['name'];
if (empty($name))
{
  $response=array('success'=> false,'log'=>'New collection name not specified');
  echo json_encode($response);
  exit(0);
}  

try {
  
  $parent=ProdsDir::fromURI($ruri, false);
  if (empty($parent->account->pass))
  {
    $acct=$_SESSION['acct_manager']->findAcct($parent->account);
    if (empty($acct))
    {
      $response=array('success'=> false,'log'=>'Authentication Required');
      echo json_encode($response);
      exit(0);
    }
    $parent->account=$acct;
  }
  if (empty($parent->account->zone))
  {
    $parent->account->getUserInfo();
  }  
  
  $parent->mkdir($name);
  
  $response=array('success'=> true,'log'=>'new collection created!');
  echo json_encode($response);
  
} catch (Exception $e) {
  $msg='';
  $err_abbr=RODSException::rodsErrCodeToAbbr($e->getCode());
  if (null!=$err_abbr)
    $msg='Failed to create collection because server returned error: '.$err_abbr;
  else
    $msg='Failed to create collection: '.$e->getMessage();
  $response=array('success'=> false,'errmsg'=> $msg, 'errcode' => $e->getCode());
  echo json_encode($response);
}

  
?>