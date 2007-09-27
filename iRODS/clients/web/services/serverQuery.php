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
  $response=array('success'=> false,'error'=>'Required RODS URI not found');
  echo json_encode($response);
  exit(0);
}    

$account=RODSAccount::fromURI($ruri);
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

$action="";
if (isset($_REQUEST['action']))
  $action=$_REQUEST['action'];
else
{
  $response=array('success'=> false,'error'=>'Action type not specified');
  echo json_encode($response);
  exit(0);
}


try {

  switch ($action)
  {
    case 'metadataname_for_files':
      $que=new ProdsQuery($account);
      $names=$que->getMetadataNamesForAllFiles();
      $fmted_array=array();
      foreach($names as $name)
        $fmted_array[]=array('name'=>$name);
       
      $response=array('success'=> true,'totalCount'=>count($names),
        'que_results'=> $fmted_array);
      $str= json_encode($response);
      echo "($str)";  
      exit(0);
      break;
    
    case 'resources':
      $que=new ProdsQuery($account);
      $resources=$que->getResources();
      $response=array('success'=> true,'totalCount'=>count($resources),
        'que_results'=> $resources);
      $str= json_encode($response);
      echo "($str)";  
      exit(0);
      break;  
    
    default:
      $response=array('success'=> false,'error'=>"Action type '$action' not supported");
      echo json_encode($response);
      exit(0);
  }
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> "$e",'errcode'=> $e->getCode());
  echo json_encode($response);
}
?>