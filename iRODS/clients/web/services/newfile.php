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
  $response=array('success'=> false,'log'=>'New file name not specified');
  echo json_encode($response);
  exit(0);
}  

$resc='';
if (isset($_REQUEST['resc']))
  $resc=$_REQUEST['resc'];
if (empty($resc))
{
  $response=array('success'=> false,'log'=>'Resource name not specified');
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
  
  $file=new ProdsFile($parent->account,$parent->path_str.'/'.$name);
  if ($file->exists())
  {
    $response=array('success'=> false,'log'=>'Path '.$parent->path_str.'/'.$name.' already exists!');
  }
  else
  {
    $file->open('w',$resc);
    $file->close();
    $response=array('success'=> true,'log'=>"new file '$name' created!");
  }
  echo json_encode($response);
  
} catch (Exception $e) {
  $response=array('success'=> false,'log'=> $e->getMessage());
  echo json_encode($response);
}

  
?>