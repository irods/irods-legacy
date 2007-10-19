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
  $response=array('success'=> false,'errmsg'=>'Expected RODS URI not found');
  echo json_encode($response);
  exit(0);
} 

$resource="";
if (isset($_REQUEST['resource']))
  $resource=$_REQUEST['resource'];
else
{
  $response=array('success'=> false,'errmsg'=>'Expected resource not found');
  echo json_encode($response);
  exit(0);
} 

$action="";
if (isset($_REQUEST['action']))
  $action=$_REQUEST['action'];
else
{
  $response=array('success'=> false,'errmsg'=>'Expected action not found');
  echo json_encode($response);
  exit(0);
} 

try {
  $file=ProdsFile::fromURI($ruri, false);
  if (empty($file->account->pass))
  {
    $acct=$_SESSION['acct_manager']->findAcct($file->account);
    if (empty($acct))
    {
      $response=array('success'=> false,'errmsg'=>'Authentication Required');
      echo json_encode($response);
      exit(0);
    }
    $file->account=$acct;
  }
  
  switch($action)
  {
    case 'add':
      $file->repl($resource,array('backupMode'=>true));
      $response=array('success'=> true,'log'=>"file replicated to resource $resource");
      break;
    
    case 'remove':
      $file->unlink($resource,true);
      $response=array('success'=> true,'log'=>"replica on resource $resource is removed");
      break;  
    
    default:
      $response=array('success'=> false,'errmsg'=>"Action '$action' is not supported");
      break;
  }  
  echo json_encode($response);
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> $e->getMessage(), 'errcode'=> $e->getCode());
  echo json_encode($response);
}

  
?>