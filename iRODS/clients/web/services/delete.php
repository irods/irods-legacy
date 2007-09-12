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

$files=array();
$dirs=array();
if (isset($_REQUEST['files']))
  $files=$_REQUEST['files'];
if (isset($_REQUEST['dirs']))
  $dirs=$_REQUEST['dirs'];
if ( (empty($files))&&(empty($dirs)) )
{
  $response=array('success'=> false,'errmsg'=>'No files or collections specified');
  echo json_encode($response);
  exit(0);
}  

$force_delete=false;
if (isset($_REQUEST['force']))
  $force_delete=true;
  
try {
  
  $parent=ProdsDir::fromURI($ruri, false);
  if (empty($parent->account->pass))
  {
    $acct=$_SESSION['acct_manager']->findAcct($parent->account);
    if (empty($acct))
    {
      $response=array('success'=> false,'errmsg'=>'Authentication Required');
      echo json_encode($response);
      exit(0);
    }
    $parent->account=$acct;
  }
  if (empty($parent->account->zone))
  {
    $parent->account->getUserInfo();
  }  
  
  foreach ($files as $filename)
  {
    $myfile=new ProdsFile($parent->account,$parent->path_str.'/'.$filename);
    $myfile->unlink(NULL, $force_delete);
  }
  
  foreach ($dirs as $dirname)
  {
    $mydir=new ProdsDir($parent->account,$parent->path_str.'/'.$dirname);
    $mydir->rmdir(true, $force_delete);
  }
  
  $response=array('success'=> true,'log'=>count($files).' files and '.count($dirs).' collections deleted!');
  echo json_encode($response);
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> $e->getMessage(), 'errcode'=> $e->getCode());
  echo json_encode($response);
}

  
?>