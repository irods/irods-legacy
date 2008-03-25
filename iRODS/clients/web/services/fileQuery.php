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
  $response=array('success'=> false,'errmsg'=>'Required RODS URI not found');
  echo json_encode($response);
  exit(0);
}    

$action="";
if (isset($_REQUEST['action']))
  $action=$_REQUEST['action'];
else
{
  $response=array('success'=> false,'errmsg'=>'Action type not specified');
  echo json_encode($response);
  exit(0);
}

try {
  
  $file=ProdsFile::fromURI($ruri);
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

  
  switch ($action)
  {
    case 'replica':
      $repl_info=$file->getReplInfo();
      $response=array('success'=> true,'totalCount'=>count($repl_info),
        'que_results'=> $repl_info);
      $str= json_encode($response);
      echo "($str)";  
      exit(0);
      break;  
    
    default:
      $response=array('success'=> false,'errmsg'=>"Action type '$action' not supported");
      echo json_encode($response);
      exit(0);
  }
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> "$e",'errcode'=> $e->getCode());
  echo json_encode($response);
}
?>